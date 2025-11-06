#include "../include/BaseApp.h"
#include <algorithm>
#include <cstring>  
#include <string> 

static const char* kHlslSource = R"(
cbuffer CBNeverChanges      : register(b0) { float4x4 gView; }
cbuffer CBChangeOnResize    : register(b1) { float4x4 gProj; }
cbuffer CBChangesEveryFrame : register(b2) { float4x4 gWorld; float4 vMeshColor; }

Texture2D gTxDiffuse : register(t0);
SamplerState gSamLinear : register(s0);

struct VS_IN  { float3 Pos:POSITION; float2 Tex:TEXCOORD0; };
struct VS_OUT { float4 Pos:SV_POSITION; float2 Tex:TEXCOORD0; };

VS_OUT VS(VS_IN i)
{
    VS_OUT o;
    float4 w = mul(float4(i.Pos,1), gWorld);
    float4 v = mul(w, gView);
    o.Pos    = mul(v, gProj);
    o.Tex    = i.Tex;
    return o;
}

// === FIX: Pixel Shader simplificado ===
float4 PS(VS_OUT i):SV_Target
{
    // Por ahora, solo devuelve el color base.
    return vMeshColor;
}
)";
BaseApp::BaseApp(HINSTANCE, int) {}

int BaseApp::run(HINSTANCE hInst, int nCmdShow)
{
    if (FAILED(m_window.init(hInst, nCmdShow, WndProc, this))) return 0;
    if (FAILED(init())) return 0;

    MSG msg = {};
    LARGE_INTEGER freq, prev;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&prev);

    while (WM_QUIT != msg.message)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            LARGE_INTEGER curr; QueryPerformanceCounter(&curr);
            float dt = float(curr.QuadPart - prev.QuadPart) / float(freq.QuadPart);
            prev = curr;

            update(dt);
            render();
        }
    }
    return int(msg.wParam);
}

// AABB helper
static void ComputeAABB(const std::vector<SimpleVertex>& vtx, XMFLOAT3& outMin, XMFLOAT3& outMax)
{
    if (vtx.empty()) { outMin = { 0,0,0 }; outMax = { 0,0,0 }; return; }
    XMFLOAT3 mn = vtx[0].Pos;
    XMFLOAT3 mx = vtx[0].Pos;
    for (const auto& v : vtx)
    {
        mn.x = std::min(mn.x, v.Pos.x); mn.y = std::min(mn.y, v.Pos.y); mn.z = std::min(mn.z, v.Pos.z);
        mx.x = std::max(mx.x, v.Pos.x); mx.y = std::max(mx.y, v.Pos.y); mx.z = std::max(mx.z, v.Pos.z);
    }
    outMin = mn; outMax = mx;
}

// Ruta absoluta del asset
static std::string MakeAssetPath(const char* rel)
{
    wchar_t exePathW[MAX_PATH]{};
    GetModuleFileNameW(nullptr, exePathW, MAX_PATH);
    std::wstring exePath(exePathW);
    size_t pos = exePath.find_last_of(L"\\/");
    std::wstring base = (pos == std::wstring::npos) ? L"." : exePath.substr(0, pos);

    std::wstring fullW = base + L"\\" + std::wstring(rel, rel + std::strlen(rel));
    std::string  fullA(fullW.begin(), fullW.end());
    return fullA;
}

HRESULT BaseApp::init()
{
    HRESULT hr = S_OK;

    // 1) SwapChain (crea Device/Context/BackBuffer)
    hr = m_swapChain.init(m_device, m_deviceContext, m_backBuffer, m_window);
    if (FAILED(hr)) { ERROR(L"BaseApp", L"init", L"Failed SwapChain"); return hr; }

    // 2) RTV del back-buffer
    hr = m_renderTargetView.init(m_device, m_backBuffer, DXGI_FORMAT_R8G8B8A8_UNORM);
    if (FAILED(hr)) { ERROR(L"BaseApp", L"init", L"Failed RTV"); return hr; }

    // === MSAA match con el back-buffer ===
    D3D11_TEXTURE2D_DESC bbDesc{};
    m_backBuffer.m_texture->GetDesc(&bbDesc);

    // 3) Depth/Stencil
    hr = m_depthStencil.init(
        m_device,
        m_window.m_width, m_window.m_height,
        DXGI_FORMAT_D24_UNORM_S8_UINT,
        D3D11_BIND_DEPTH_STENCIL,
        bbDesc.SampleDesc.Count, bbDesc.SampleDesc.Quality);
    if (FAILED(hr)) { ERROR(L"BaseApp", L"init", L"Failed Depth Texture"); return hr; }

    // 4) DSV
    hr = m_depthStencilView.init(m_device, m_depthStencil, DXGI_FORMAT_D24_UNORM_S8_UINT);
    if (FAILED(hr)) { ERROR(L"BaseApp", L"init", L"Failed DSV"); return hr; }

    // 5) Viewport
    hr = m_viewport.init(m_window);
    if (FAILED(hr)) { ERROR(L"BaseApp", L"init", L"Failed Viewport"); return hr; }

    // 6) InputLayout (POSITION, TEXCOORD)
    std::vector<D3D11_INPUT_ELEMENT_DESC> Layout;
    {
        D3D11_INPUT_ELEMENT_DESC p{};
        p.SemanticName = "POSITION";
        p.SemanticIndex = 0;
        p.Format = DXGI_FORMAT_R32G32B32_FLOAT;
        p.InputSlot = 0;
        p.AlignedByteOffset = 0;
        p.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        p.InstanceDataStepRate = 0;
        Layout.push_back(p);

        D3D11_INPUT_ELEMENT_DESC t{};
        t.SemanticName = "TEXCOORD";
        t.SemanticIndex = 0;
        t.Format = DXGI_FORMAT_R32G32_FLOAT;
        t.InputSlot = 0;
        t.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
        t.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        t.InstanceDataStepRate = 0;
        Layout.push_back(t);
    }

    
    D3D11_RASTERIZER_DESC rsDesc = {};
    rsDesc.FillMode = D3D11_FILL_SOLID;
    rsDesc.CullMode = D3D11_CULL_NONE;
    rsDesc.FrontCounterClockwise = false;
    rsDesc.DepthClipEnable = true;

    ID3D11RasterizerState* pRasterState;
    hr = m_device.m_device->CreateRasterizerState(&rsDesc, &pRasterState);
    if (FAILED(hr)) { ERROR(L"BaseApp", L"init", L"Failed RasterizerState"); return hr; }

    m_deviceContext.m_deviceContext->RSSetState(pRasterState);
    pRasterState->Release();

    // 7) Shaders desde HLSL embebido
    hr = m_shaderProgram.initFromSource(m_device, kHlslSource, Layout);
    if (FAILED(hr)) { ERROR(L"BaseApp", L"init", L"Failed ShaderProgram"); return hr; }

    // 8) Cargar modelo OBJ
    bool loadedOBJ = false;
    {
        ModelLoader loader;
        const std::string objPath = MakeAssetPath("Assets\\Moto\\serieS.obj");
        OutputDebugStringA(("OBJ path: " + objPath + "\n").c_str());

        loadedOBJ = loader.LoadOBJ(objPath, m_mesh, /*flipV=*/false);
        if (!loadedOBJ) {
            ERROR(L"BaseApp", L"init", L"OBJ Load FAILED -> using fallback quad");
            m_mesh.m_vertex = {
                { XMFLOAT3(-1,0,-1), XMFLOAT2(0,0) }, { XMFLOAT3(1,0,-1), XMFLOAT2(1,0) },
                { XMFLOAT3(1,0, 1), XMFLOAT2(1,1) }, { XMFLOAT3(-1,0, 1), XMFLOAT2(0,1) },
            };
            m_mesh.m_index = { 0,1,2, 0,2,3 };
        }
        m_mesh.m_numVertex = (int)m_mesh.m_vertex.size();
        m_mesh.m_numIndex = (int)m_mesh.m_index.size();
        OutputDebugStringA(("Mesh loaded. Vertices: " + std::to_string(m_mesh.m_numVertex) + ", Indices: " + std::to_string(m_mesh.m_numIndex) + "\n").c_str());
    }

    m_cameraDistance = 3.0f;

    float aspect = (float)m_window.m_width / (float)m_window.m_height;
    float fovY = XMConvertToRadians(45.0f);
    // ¡Aumenta el plano lejano para ver tu moto gigante!
    m_Projection = XMMatrixPerspectiveFovLH(fovY, aspect, 0.1f, 1000.0f);

   
    hr = m_cbNeverChanges.init(m_device, sizeof(CBNeverChanges));
    if (FAILED(hr)) { ERROR(L"BaseApp", L"init", L"Failed cbNeverChanges"); return hr; }

    hr = m_cbChangeOnResize.init(m_device, sizeof(CBChangeOnResize));
    if (FAILED(hr)) { ERROR(L"BaseApp", L"init", L"Failed cbChangeOnResize"); return hr; }

    hr = m_cbChangesEveryFrame.init(m_device, sizeof(CBChangesEveryFrame));
    if (FAILED(hr)) { ERROR(L"BaseApp", L"init", L"Failed cbChangesEveryFrame"); return hr; }

    // Sube a los constant buffers (view/proj/world/color)
    cbNeverChanges.mView = XMMatrixTranspose(m_View);
    cbChangesOnResize.mProjection = XMMatrixTranspose(m_Projection);
    cb.mWorld = XMMatrixTranspose(m_World);
    m_vMeshColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f); // Blanco
    cb.vMeshColor = m_vMeshColor;

    m_cbNeverChanges.update(m_deviceContext, nullptr, 0, nullptr, &cbNeverChanges, 0, 0);
    m_cbChangeOnResize.update(m_deviceContext, nullptr, 0, nullptr, &cbChangesOnResize, 0, 0);
    m_cbChangesEveryFrame.update(m_deviceContext, nullptr, 0, nullptr, &cb, 0, 0);

    
    // 10) CREAR BUFFERS DE GEOMETRÍA
    hr = m_vertexBuffer.init(m_device, m_mesh, D3D11_BIND_VERTEX_BUFFER);
    if (FAILED(hr)) { ERROR(L"BaseApp", L"init", L"Failed Vertex Buffer"); return hr; }

    hr = m_indexBuffer.init(m_device, m_mesh, D3D11_BIND_INDEX_BUFFER);
    if (FAILED(hr)) { ERROR(L"BaseApp", L"init", L"Failed Index Buffer"); return hr; }

    // 11) CREAR SAMPLER STATE
    hr = m_samplerState.init(m_device);
    if (FAILED(hr)) { ERROR(L"BaseApp", L"init", L"Failed SamplerState"); return hr; }

    return S_OK;
}


void BaseApp::update(float)
{
    // === 1. CALCULAR LA CÁMARA CON EL ZOOM ===
    // (Iniciará en 3.0f, como lo pusiste en init())

    float yOffset = m_cameraDistance * 0.33f; // Sube 1/3 de lo que se aleja

    XMVECTOR Eye = XMVectorSet(0.0f, yOffset, -m_cameraDistance, 1.0f);
    XMVECTOR At = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f); // Siempre miramos al origen
    XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

    // Recalcular la matriz de Vista (View)
    m_View = XMMatrixLookAtLH(Eye, At, Up);

    // === 2. DEFINIR LA ROTACIÓN DEL XBOX ===

    // Rota -90 grados en X para "levantarlo" (Z-up de Blender a Y-up de DirectX)
    XMMATRIX rotX = XMMatrixRotationX(XMConvertToRadians(-90.0f));

    // Rota 20 grados en Y para que se vea de lado (¡Puedes ajustar este '20.0f'!)
    XMMATRIX rotY = XMMatrixRotationY(XMConvertToRadians(20.0f));

    // Combinamos las rotaciones (primero X, luego Y)
    m_World = rotX * rotY;


    // === 3. ACTUALIZAR LOS BUFFERS ===
    // Re-sube view/proj (por si hubo resize) y world/color
    cbNeverChanges.mView = XMMatrixTranspose(m_View);
    cbChangesOnResize.mProjection = XMMatrixTranspose(m_Projection);
    cb.mWorld = XMMatrixTranspose(m_World);
    cb.vMeshColor = m_vMeshColor;

    m_cbNeverChanges.update(m_deviceContext, nullptr, 0, nullptr, &cbNeverChanges, 0, 0);
    m_cbChangeOnResize.update(m_deviceContext, nullptr, 0, nullptr, &cbChangesOnResize, 0, 0);
    m_cbChangesEveryFrame.update(m_deviceContext, nullptr, 0, nullptr, &cb, 0, 0);
}

void BaseApp::onMouseWheel(int zDelta)
{
    // ¡Volvemos al zoom multiplicativo (más suave)!
    // Es mejor para modelos pequeños que el zoom "aditivo" de 100 unidades.
    float zoomSpeed = 1.2f;

    if (zDelta > 0) // Tecla '+' o Scroll Arriba (Acercar)
    {
        m_cameraDistance /= zoomSpeed;
    }
    else // Tecla '-' o Scroll Abajo (Alejar)
    {
        m_cameraDistance *= zoomSpeed;
    }

    // === ¡NUEVOS LÍMITES PARA UN MODELO PEQUEÑO! ===
    if (m_cameraDistance < 0.1f) // Límite mínimo: 0.1 unidades (¡muy cerca!)
    {
        m_cameraDistance = 0.1f;
    }
    if (m_cameraDistance > 1000.0f) // Límite máximo: 1000 unidades
    {
        m_cameraDistance = 1000.0f;
    }

    // Imprime la distancia actual para debugging
    std::string debugMsg = "Camera Distance: " + std::to_string(m_cameraDistance) + "\n";
    OutputDebugStringA(debugMsg.c_str());
}

void BaseApp::render()
{
    const float Clear[4] = { 0.05f, 0.05f, 0.05f, 1.0f };
    m_renderTargetView.render(m_deviceContext, m_depthStencilView, 1, Clear);

    m_viewport.render(m_deviceContext);
    m_depthStencilView.render(m_deviceContext);

    m_shaderProgram.render(m_deviceContext);

    m_vertexBuffer.render(m_deviceContext, 0, 1);
    m_indexBuffer.render(m_deviceContext, 0, 1, false, DXGI_FORMAT_R32_UINT);

    m_cbNeverChanges.render(m_deviceContext, 0, 1);
    m_cbChangeOnResize.render(m_deviceContext, 1, 1);
    m_cbChangesEveryFrame.render(m_deviceContext, 2, 1);
    m_cbChangesEveryFrame.render(m_deviceContext, 2, 1, true); // PS
    m_samplerState.render(m_deviceContext, 0, 1);

    // === ¡FIX: DEFINIR TOPOLOGÍA! ===
    m_deviceContext.m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    m_deviceContext.DrawIndexed(m_mesh.m_numIndex, 0, 0);
    m_swapChain.present();
}

void BaseApp::destroy()
{
    if (m_deviceContext.m_deviceContext) m_deviceContext.m_deviceContext->ClearState();

    m_samplerState.destroy();
    m_cbNeverChanges.destroy();
    m_cbChangeOnResize.destroy();
    m_cbChangesEveryFrame.destroy();
    m_vertexBuffer.destroy(); // <-- FIX: 'V' eliminada
    m_indexBuffer.destroy();
    m_shaderProgram.destroy();
    m_depthStencil.destroy();
    m_depthStencilView.destroy();
    m_renderTargetView.destroy();
    m_swapChain.destroy();
    m_backBuffer.destroy();
    m_deviceContext.destroy();
    m_device.destroy();
}

LRESULT CALLBACK BaseApp::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    // === ¡Esta línea sigue siendo la más importante! ===
    // (Debe estar ANTES del switch)
    BaseApp* pApp = reinterpret_cast<BaseApp*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

     switch (message)
    {
    case WM_CREATE:
    {
        // "Guarda" el puntero 'this' en la ventana
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pCreate->lpCreateParams);
    }
    return 0;

    case WM_PAINT:
    {
        PAINTSTRUCT ps; BeginPaint(hWnd, &ps); EndPaint(hWnd, &ps);
    }
    return 0;

    // Dejamos el código del scroll, no hace daño
    case WM_MOUSEWHEEL:
    {
        if (pApp)
        {
            int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
            pApp->onMouseWheel(zDelta);
        }
    }
    return 0;

    // === ¡¡¡AQUÍ ESTÁ EL CÓDIGO NUEVO!!! ===
    case WM_KEYDOWN: // Se presionó una tecla
    {
        if (pApp) // Si tenemos nuestra app
        {
            switch (wParam) // Vemos qué tecla fue
            {
                // Si fue la tecla '+' (teclado normal) O la del Numpad (+)
            case VK_OEM_PLUS:
            case VK_ADD:
                pApp->onMouseWheel(1); // Llama a onMouseWheel con un valor positivo (Acercar)
                break;

                // Si fue la tecla '-' (teclado normal) O la del Numpad (-)
            case VK_OEM_MINUS:
            case VK_SUBTRACT:
                pApp->onMouseWheel(-1); // Llama a onMouseWheel con un valor negativo (Alejar)
                break;
            }
        }
    }
    return 0; // ¡Importante!

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    // Esta es la línea final
    return DefWindowProc(hWnd, message, wParam, lParam);
}