#include "../include/BaseApp.h"
#include "../include/ModelLoader.h" 
#include <algorithm>
#include <cstring>
#include <string> 
#include <d3dx11.h> 

// == HLSL ==
static const char* kHlslSource = R"(
cbuffer CBNeverChanges      : register(b0) { float4x4 gView; }
cbuffer CBChangeOnResize    : register(b1) { float4x4 gProj; }
cbuffer CBChangesEveryFrame : register(b2) { float4x4 gWorld; float4 vMeshColor; }

Texture2D    gTxDiffuse : register(t0);
SamplerState gSamLinear : register(s0);

struct VS_IN  { 
    float3 Pos   : POSITION; 
    float2 Tex   : TEXCOORD0; 
    float3 Normal: NORMAL; 
};
struct VS_OUT { 
    float4 Pos:SV_POSITION; 
    float2 Tex:TEXCOORD0; 
};

VS_OUT VS(VS_IN i)
{
    VS_OUT o;
    float4 w = mul(float4(i.Pos,1), gWorld);
    float4 v = mul(w, gView);
    o.Pos    = mul(v, gProj);
    o.Tex    = i.Tex;
    return o;
}

float4 PS(VS_OUT i):SV_Target
{
    // Prueba rápida (dejar comentada normalmente)
    // return float4(i.Tex.x, i.Tex.y, 0.0, 1.0);

    return gTxDiffuse.Sample(gSamLinear, i.Tex) * vMeshColor;
}
)";
// ==================================================

// ---- Helpers ----
static void ComputeAABB(const std::vector<SimpleVertex>& vtx, XMFLOAT3& outMin, XMFLOAT3& outMax)
{
    if (vtx.empty()) { outMin = { 0,0,0 }; outMax = { 0,0,0 }; return; }
    XMFLOAT3 mn = vtx[0].Pos, mx = vtx[0].Pos;
    for (const auto& v : vtx) {
        mn.x = std::min(mn.x, v.Pos.x); mn.y = std::min(mn.y, v.Pos.y); mn.z = std::min(mn.z, v.Pos.z);
        mx.x = std::max(mx.x, v.Pos.x); mx.y = std::max(mx.y, v.Pos.y); mx.z = std::max(mx.z, v.Pos.z);
    }
    outMin = mn; outMax = mx;
}

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

// ---- BaseApp ----
BaseApp::BaseApp(HINSTANCE, int) {}

int BaseApp::run(HINSTANCE hInst, int nCmdShow)
{
    // Asegúrate que Window::init tenga overload para recibir this en lpCreateParams
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
            TranslateMessage(&msg); DispatchMessage(&msg);
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

HRESULT BaseApp::init()
{
    HRESULT hr = S_OK;

    // 1) SwapChain/Device/Context + 2) RTV
    hr = m_swapChain.init(m_device, m_deviceContext, m_backBuffer, m_window);
    if (FAILED(hr)) { ERROR(L"BaseApp", L"init", L"Failed SwapChain"); return hr; }
    hr = m_renderTargetView.init(m_device, m_backBuffer, DXGI_FORMAT_R8G8B8A8_UNORM);
    if (FAILED(hr)) { ERROR(L"BaseApp", L"init", L"Failed RTV"); return hr; }

    // 3) Depth texture (match MSAA con backbuffer) + 4) DSV
    D3D11_TEXTURE2D_DESC bbDesc{}; m_backBuffer.m_texture->GetDesc(&bbDesc);
    hr = m_depthStencil.init(m_device, m_window.m_width, m_window.m_height,
        DXGI_FORMAT_D24_UNORM_S8_UINT, D3D11_BIND_DEPTH_STENCIL,
        bbDesc.SampleDesc.Count, bbDesc.SampleDesc.Quality);
    if (FAILED(hr)) { ERROR(L"BaseApp", L"init", L"Failed Depth Texture"); return hr; }
    hr = m_depthStencilView.init(m_device, m_depthStencil, DXGI_FORMAT_D24_UNORM_S8_UINT);
    if (FAILED(hr)) { ERROR(L"BaseApp", L"init", L"Failed DSV"); return hr; }

    // 5) Viewport
    hr = m_viewport.init(m_window);
    if (FAILED(hr)) { ERROR(L"BaseApp", L"init", L"Failed Viewport"); return hr; }

    // 6) InputLayout (Pos, Tex, Normal)
    std::vector<D3D11_INPUT_ELEMENT_DESC> Layout;
    {
        D3D11_INPUT_ELEMENT_DESC p{};
        p.SemanticName = "POSITION"; p.Format = DXGI_FORMAT_R32G32B32_FLOAT;
        p.InputSlot = 0; p.AlignedByteOffset = 0; p.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        Layout.push_back(p);

        D3D11_INPUT_ELEMENT_DESC t{};
        t.SemanticName = "TEXCOORD"; t.Format = DXGI_FORMAT_R32G32_FLOAT;
        t.InputSlot = 0; t.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT; t.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        Layout.push_back(t);

        D3D11_INPUT_ELEMENT_DESC n{};
        n.SemanticName = "NORMAL"; n.Format = DXGI_FORMAT_R32G32B32_FLOAT;
        n.InputSlot = 0; n.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT; n.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        Layout.push_back(n);
    }

    // 6.5) Rasterizer (sin culling para evitar caras “faltantes”)
    {
        D3D11_RASTERIZER_DESC rsDesc = {};
        rsDesc.FillMode = D3D11_FILL_SOLID;
        rsDesc.CullMode = D3D11_CULL_NONE;
        rsDesc.FrontCounterClockwise = FALSE;
        rsDesc.DepthClipEnable = TRUE;
        ID3D11RasterizerState* pRS = nullptr;
        hr = m_device.m_device->CreateRasterizerState(&rsDesc, &pRS);
        if (FAILED(hr)) { ERROR(L"BaseApp", L"init", L"Failed RasterizerState"); return hr; }
        m_deviceContext.m_deviceContext->RSSetState(pRS);
        pRS->Release();
    }

    // 7) ShaderProgram desde HLSL embebido
    hr = m_shaderProgram.initFromSource(m_device, kHlslSource, Layout);
    if (FAILED(hr)) { ERROR(L"BaseApp", L"init", L"Failed ShaderProgram"); return hr; }

    // 8) Cargar modelo OBJ 
    {
        OBJParser loader;
        const std::string objPath = MakeAssetPath("Assets\\Moto\\repsol3.obj");
        OutputDebugStringA(("OBJ path: " + objPath + "\n").c_str());

        if (!loader.LoadOBJ(objPath, m_mesh, /*flipV=*/true)) {
            ERROR(L"BaseApp", L"init", L"OBJ Load FAILED -> using fallback quad");
            m_mesh.m_vertex = {
                { XMFLOAT3(-1,0,-1), XMFLOAT2(0,0), XMFLOAT3(0,1,0) },
                { XMFLOAT3(1,0,-1), XMFLOAT2(1,0), XMFLOAT3(0,1,0) },
                { XMFLOAT3(1,0, 1), XMFLOAT2(1,1), XMFLOAT3(0,1,0) },
                { XMFLOAT3(-1,0, 1), XMFLOAT2(0,1), XMFLOAT3(0,1,0) },
            };
            m_mesh.m_index = { 0,1,2, 0,2,3 };
        }

        m_mesh.m_numVertex = (int)m_mesh.m_vertex.size();
        m_mesh.m_numIndex = (int)m_mesh.m_index.size();

        OutputDebugStringA(("Mesh loaded. V=" + std::to_string(m_mesh.m_numVertex) +
            " I=" + std::to_string(m_mesh.m_numIndex) + "\n").c_str());
    }

    // 8.5) Cargar textura (wrapper)
    {
        const std::string texBase = MakeAssetPath("Assets\\Textures\\LV");

        HRESULT hr_tex = m_textureCube.init(m_device, texBase, ExtensionType::PNG);
        if (FAILED(hr_tex)) {
            OutputDebugStringA("FAILED loading Tex_0041_0.png\n");
        }
        else {
            OutputDebugStringA("OK loading Tex_0041_0.png\n");
        }
    }


    // 9) Auto-encuadre por AABB (centra y calcula distancia)
    {
        XMFLOAT3 aabbMin, aabbMax; ComputeAABB(m_mesh.m_vertex, aabbMin, aabbMax);
        XMVECTOR vMin = XMLoadFloat3(&aabbMin);
        XMVECTOR vMax = XMLoadFloat3(&aabbMax);
        XMVECTOR vCenter = 0.5f * (vMin + vMax);
        XMVECTOR vExt = 0.5f * (vMax - vMin);
        float radius = XMVectorGetX(XMVector3Length(vExt)); // esfera contenedora aprox

        // World: trasladar el modelo para que su centro quede en el origen
        XMFLOAT3 fCenter; XMStoreFloat3(&fCenter, vCenter);
        m_World = XMMatrixTranslation(-fCenter.x, -fCenter.y, -fCenter.z);

        // Proyección
        float aspect = (float)m_window.m_width / (float)m_window.m_height;
        float fovY = XMConvertToRadians(45.0f);
        m_Projection = XMMatrixPerspectiveFovLH(fovY, aspect, 0.01f, 10000.0f);

        // Distancia de cámara cómoda
        float fovX = 2.0f * atanf(tanf(fovY * 0.5f) * aspect);
        float dist = std::max(radius / sinf(fovX * 0.5f), radius / sinf(fovY * 0.5f)) * 1.35f;
        m_cameraDistance = std::max(dist, 1.0f); // por si radius ~ 0

        // Vista inicial (ligeramente oblicua)
        XMVECTOR Eye = XMVectorSet(0.7f, 0.45f, -1.0f, 0.0f);
        Eye = XMVector3Normalize(Eye) * m_cameraDistance;
        XMVECTOR At = XMVectorZero();
        XMVECTOR Up = XMVectorSet(0, 1, 0, 0);
        m_View = XMMatrixLookAtLH(Eye, At, Up);
    }

    // 10) Constant Buffers
    if (FAILED(m_cbNeverChanges.init(m_device, sizeof(CBNeverChanges))))         return E_FAIL;
    if (FAILED(m_cbChangeOnResize.init(m_device, sizeof(CBChangeOnResize))))     return E_FAIL;
    if (FAILED(m_cbChangesEveryFrame.init(m_device, sizeof(CBChangesEveryFrame)))) return E_FAIL;

    cbNeverChanges.mView = XMMatrixTranspose(m_View);
    cbChangesOnResize.mProjection = XMMatrixTranspose(m_Projection);
    m_vMeshColor = XMFLOAT4(1, 1, 1, 1);
    cb.mWorld = XMMatrixTranspose(m_World);
    cb.vMeshColor = m_vMeshColor;

    m_cbNeverChanges.update(m_deviceContext, nullptr, 0, nullptr, &cbNeverChanges, 0, 0);
    m_cbChangeOnResize.update(m_deviceContext, nullptr, 0, nullptr, &cbChangesOnResize, 0, 0);
    m_cbChangesEveryFrame.update(m_deviceContext, nullptr, 0, nullptr, &cb, 0, 0);

    // 11) VB/IB + Topology
    hr = m_vertexBuffer.init(m_device, m_mesh, D3D11_BIND_VERTEX_BUFFER);
    if (FAILED(hr)) { ERROR(L"BaseApp", L"init", L"Failed VertexBuffer"); return hr; }
    hr = m_indexBuffer.init(m_device, m_mesh, D3D11_BIND_INDEX_BUFFER);
    if (FAILED(hr)) { ERROR(L"BaseApp", L"init", L"Failed IndexBuffer"); return hr; }
    m_deviceContext.IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // 12) Sampler
    if (FAILED(m_samplerState.init(m_device))) { ERROR(L"BaseApp", L"init", L"Failed SamplerState"); return E_FAIL; }

    // Rotación inicial
    //m_modelRotation = 0.0f;//

    return S_OK;
}

void BaseApp::update(float deltaTime)
{
    // --- Velocidades (grados/seg) -> rad/seg
    const float spinW = XMConvertToRadians(m_spinSpeedDeg);   // rotación del modelo
    const float orbitW = XMConvertToRadians(m_orbitSpeedDeg);  // órbita de cámara

    // --- Avanzar ángulos
    m_spinAngle += spinW * deltaTime;
    m_orbitAngle += orbitW * deltaTime;

    // --- Cámara en órbita suave alrededor del modelo
    const float r = m_cameraDistance;

    // Sube un poco la cámara
    const float eyeY = r * 0.5f;      // prueba 0.4f, 0.5f, 0.6f
    const float ex = sinf(m_orbitAngle) * r;
    const float ez = -cosf(m_orbitAngle) * r;

    // Punto al que miras (subido)
    const float targetY = 1.5f;          // si sigue bajo, prueba 2.0f, 2.5f

    XMVECTOR Eye = XMVectorSet(ex, eyeY, ez, 1.0f);
    XMVECTOR At = XMVectorSet(0.0f, targetY, 0.0f, 1.0f);
    XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

    m_View = XMMatrixLookAtLH(Eye, At, Up);

    // --- World: inclina -90° en X para colocar la malla, + giro sobre Y
    XMMATRIX rotX = XMMatrixRotationX(XMConvertToRadians(0.0f));
    XMMATRIX rotY = XMMatrixRotationY(m_spinAngle);
    m_World = rotX * rotY;

    // --- Subir constantes
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
    const float zoomSpeed = 1.2f;
    if (zDelta > 0) m_cameraDistance /= zoomSpeed;
    else            m_cameraDistance *= zoomSpeed;

    m_cameraDistance = std::max(0.1f, std::min(m_cameraDistance, 10000.0f));

    std::string dbg = "Camera Distance: " + std::to_string(m_cameraDistance) + "\n";
    OutputDebugStringA(dbg.c_str());
}

void BaseApp::render()
{
    const float Clear[4] = { 0.05f, 0.05f, 0.05f, 1.0f };
    m_renderTargetView.render(m_deviceContext, m_depthStencilView, 1, Clear);

    m_viewport.render(m_deviceContext);
    m_depthStencilView.render(m_deviceContext);
    m_shaderProgram.render(m_deviceContext);

    // VB/IB
    m_vertexBuffer.render(m_deviceContext, 0, 1);
    m_indexBuffer.render(m_deviceContext, 0, 1, false, DXGI_FORMAT_R32_UINT);

    // CBs + textura + sampler
    m_cbNeverChanges.render(m_deviceContext, 0, 1);
    m_cbChangeOnResize.render(m_deviceContext, 1, 1);
    m_cbChangesEveryFrame.render(m_deviceContext, 2, 1);
    m_cbChangesEveryFrame.render(m_deviceContext, 2, 1, true);
    m_textureCube.render(m_deviceContext, 0, 1);
    m_samplerState.render(m_deviceContext, 0, 1);

    // Topología
    m_deviceContext.IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Draw
    m_deviceContext.DrawIndexed(m_mesh.m_numIndex, 0, 0);

    m_swapChain.present();
}

void BaseApp::destroy()
{
    if (m_deviceContext.m_deviceContext) m_deviceContext.m_deviceContext->ClearState();

    m_samplerState.destroy();
    m_textureCube.destroy();
    m_cbNeverChanges.destroy();
    m_cbChangeOnResize.destroy();
    m_cbChangesEveryFrame.destroy();
    m_vertexBuffer.destroy();
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
    BaseApp* pApp = reinterpret_cast<BaseApp*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

    switch (message)
    {
    case WM_CREATE:
    {
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pCreate->lpCreateParams);
    }
    return 0;

    case WM_MOUSEWHEEL:
        if (pApp) { pApp->onMouseWheel(GET_WHEEL_DELTA_WPARAM(wParam)); }
        return 0;

    case WM_KEYDOWN:
        if (pApp) {
            switch (wParam) {
            case VK_OEM_PLUS:
            case VK_ADD:      pApp->onMouseWheel(+120); break;
            case VK_OEM_MINUS:
            case VK_SUBTRACT: pApp->onMouseWheel(-120); break;
            }
        }
        return 0;

    case WM_PAINT:
    {
        PAINTSTRUCT ps; BeginPaint(hWnd, &ps); EndPaint(hWnd, &ps);
    }
    return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}