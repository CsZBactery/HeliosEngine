#include "../include/Prerequisites.h"
#include "../include/BaseApp.h"
#include "../include/ModelLoader.h"   // para cargar el OBJ
#include <algorithm>                  // std::min/std::max
#include <cfloat>                     // FLT_MAX

// ================= HLSL embebido (checker procedural) =================
static const char* kHlslSource = R"(
cbuffer CBNeverChanges      : register(b0) { float4x4 gView; }
cbuffer CBChangeOnResize    : register(b1) { float4x4 gProj; }
cbuffer CBChangesEveryFrame : register(b2) { float4x4 gWorld; float4 vMeshColor; }

Texture2D gTxDiffuse : register(t0);
SamplerState gSamLinear : register(s0);

struct VS_IN { float3 Pos:POSITION; float2 Tex:TEXCOORD0; };
struct VS_OUT{ float4 Pos:SV_POSITION; float2 Tex:TEXCOORD0; };

VS_OUT VS(VS_IN i){
    VS_OUT o;
    float4 w = mul(float4(i.Pos,1), gWorld);
    float4 v = mul(w, gView);
    o.Pos    = mul(v, gProj);
    o.Tex    = i.Tex;
    return o;
}

float4 PS(VS_OUT i):SV_Target{
    // Checker procedural por si no hay textura ligada
    float2 uv = i.Tex * 8.0;
    float  c  = (fmod(floor(uv.x)+floor(uv.y),2.0) < 1.0) ? 0.2 : 1.0;
    float4 checker = float4(c,c,c,1);

    float4 texel = gTxDiffuse.Sample(gSamLinear, i.Tex);
    float useChecker = step(texel.r+texel.g+texel.b+texel.a, 0.0001);

    return lerp(texel, checker, useChecker) * vMeshColor;
}
)";
// ======================================================================

BaseApp::BaseApp(HINSTANCE, int) {}

// Punto de entrada (envoltura de clase)
int BaseApp::run(HINSTANCE hInst, int nCmdShow) {
    if (FAILED(m_window.init(hInst, nCmdShow, WndProc))) return 0;
    if (FAILED(init())) return 0;

    MSG msg = {};
    LARGE_INTEGER freq, prev;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&prev);

    while (WM_QUIT != msg.message) {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else {
            LARGE_INTEGER curr;
            QueryPerformanceCounter(&curr);
            float deltaTime = static_cast<float>(curr.QuadPart - prev.QuadPart) / freq.QuadPart;
            prev = curr;
            update(deltaTime);
            render();
        }
    }
    return (int)msg.wParam;
}

HRESULT BaseApp::init() {
    HRESULT hr = S_OK;

    // 1) SwapChain (crea Device/Context/BackBuffer)
    hr = m_swapChain.init(m_device, m_deviceContext, m_backBuffer, m_window);
    if (FAILED(hr)) { ERROR("BaseApp", "init", "Failed to initialize SwapChain."); return hr; }

    // 2) RTV del back-buffer
    hr = m_renderTargetView.init(m_device, m_backBuffer, DXGI_FORMAT_R8G8B8A8_UNORM);
    if (FAILED(hr)) { ERROR("BaseApp", "init", "Failed to initialize RenderTargetView."); return hr; }

    // MSAA match con el back-buffer
    D3D11_TEXTURE2D_DESC bbDesc{};
    m_backBuffer.m_texture->GetDesc(&bbDesc);

    // 3) Depth/Stencil (match MSAA)
    hr = m_depthStencil.init(
        m_device,
        m_window.m_width,
        m_window.m_height,
        DXGI_FORMAT_D24_UNORM_S8_UINT,
        D3D11_BIND_DEPTH_STENCIL,
        bbDesc.SampleDesc.Count,
        bbDesc.SampleDesc.Quality
    );
    if (FAILED(hr)) { ERROR("BaseApp", "init", "Failed to initialize DepthStencil Texture."); return hr; }

    // 4) DSV
    hr = m_depthStencilView.init(m_device, m_depthStencil, DXGI_FORMAT_D24_UNORM_S8_UINT);
    if (FAILED(hr)) { ERROR("BaseApp", "init", "Failed to initialize DepthStencilView."); return hr; }

    // 5) Viewport
    hr = m_viewport.init(m_window);
    if (FAILED(hr)) { ERROR("BaseApp", "init", "Failed to initialize Viewport."); return hr; }

    // 6) InputLayout
    std::vector<D3D11_INPUT_ELEMENT_DESC> Layout;
    {
        D3D11_INPUT_ELEMENT_DESC p{};
        p.SemanticName = "POSITION";
        p.SemanticIndex = 0;
        p.Format = DXGI_FORMAT_R32G32B32_FLOAT;
        p.InputSlot = 0;
        p.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
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

    // 7) Shaders desde HLSL embebido
    hr = m_shaderProgram.initFromSource(m_device, kHlslSource, Layout);
    if (FAILED(hr)) {
        ERROR("BaseApp", "init", ("Failed to initialize ShaderProgram. HRESULT: " + std::to_string(hr)).c_str());
        return hr;
    }

    // 8) Cargar modelo OBJ a m_mesh
    {
        ModelLoader loader;
        // Ajusta esta ruta si tu archivo está en otro nombre o carpeta:
        const std::string objPath = "Assets/Moto/repsol3.obj";
        m_mesh.m_vertex.clear();
        m_mesh.m_index.clear();
        m_mesh.m_numVertex = 0;
        m_mesh.m_numIndex = 0;

        if (!loader.LoadOBJ(objPath, m_mesh, /*flipV=*/true)) {
            ERROR("BaseApp", "init", "Failed to load OBJ. Verify path/triangulation.");
            return E_FAIL;
        }
        MESSAGE(L"ModelLoader", "LoadOBJ", L"OBJ loaded successfully");
    }

    // 9) Vertex/Index Buffers
    hr = m_vertexBuffer.init(m_device, m_mesh, D3D11_BIND_VERTEX_BUFFER);
    if (FAILED(hr)) { ERROR("BaseApp", "init", "Failed to initialize VertexBuffer."); return hr; }

    hr = m_indexBuffer.init(m_device, m_mesh, D3D11_BIND_INDEX_BUFFER);
    if (FAILED(hr)) { ERROR("BaseApp", "init", "Failed to initialize IndexBuffer."); return hr; }

    // Topología
    m_deviceContext.IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // 10) Constant Buffers
    hr = m_cbNeverChanges.init(m_device, sizeof(CBNeverChanges));
    if (FAILED(hr)) { ERROR("BaseApp", "init", "Failed to initialize CBNeverChanges."); return hr; }

    hr = m_cbChangeOnResize.init(m_device, sizeof(CBChangeOnResize));
    if (FAILED(hr)) { ERROR("BaseApp", "init", "Failed to initialize CBChangeOnResize."); return hr; }

    hr = m_cbChangesEveryFrame.init(m_device, sizeof(CBChangesEveryFrame));
    if (FAILED(hr)) { ERROR("BaseApp", "init", "Failed to initialize CBChangesEveryFrame."); return hr; }

    // 11) Sampler
    hr = m_samplerState.init(m_device);
    if (FAILED(hr)) { ERROR("BaseApp", "init", "Failed to initialize SamplerState."); return hr; }

    // 12) World inicial
    m_World = XMMatrixIdentity();

    // ======== CÁMARA AUTOMÁTICA BASADA EN AABB DEL MESH ========
    XMFLOAT3 minP(FLT_MAX, FLT_MAX, FLT_MAX);
    XMFLOAT3 maxP(-FLT_MAX, -FLT_MAX, -FLT_MAX);

    for (const auto& v : m_mesh.m_vertex) {
        minP.x = std::min(minP.x, v.Pos.x);
        minP.y = std::min(minP.y, v.Pos.y);
        minP.z = std::min(minP.z, v.Pos.z);
        maxP.x = std::max(maxP.x, v.Pos.x);
        maxP.y = std::max(maxP.y, v.Pos.y);
        maxP.z = std::max(maxP.z, v.Pos.z);
    }
    if (!(minP.x < maxP.x && minP.y < maxP.y && minP.z < maxP.z)) {
        minP = XMFLOAT3(-1.f, -1.f, -1.f);
        maxP = XMFLOAT3(1.f, 1.f, 1.f);
    }

    XMVECTOR vMin = XMLoadFloat3(&minP);
    XMVECTOR vMax = XMLoadFloat3(&maxP);
    XMVECTOR center = XMVectorScale(XMVectorAdd(vMin, vMax), 0.5f);
    XMVECTOR ext = XMVectorScale(XMVectorSubtract(vMax, vMin), 0.5f);

    float radius = XMVectorGetX(XMVector3Length(ext));
    radius = std::max(radius, 0.001f);

    const float fov = XM_PIDIV4;
    const float aspect = (float)m_window.m_width / (float)m_window.m_height;
    const float nearZ = 0.01f;
    float dist = (radius / tanf(fov * 0.5f)) * 1.15f;

    XMVECTOR Eye = XMVectorAdd(center, XMVectorSet(0.f, 0.f, -dist, 0.f));
    XMVECTOR At = center;
    XMVECTOR Up = XMVectorSet(0.f, 1.f, 0.f, 0.f);

    m_View = XMMatrixLookAtLH(Eye, At, Up);
    m_Projection = XMMatrixPerspectiveFovLH(fov, aspect, nearZ, std::max(nearZ * 10.f, dist * 4.f));

    cbNeverChanges.mView = XMMatrixTranspose(m_View);
    cbChangesOnResize.mProjection = XMMatrixTranspose(m_Projection);

    // ======== Rasterizer: sólido y sin culling (evita caras invertidas) ========
    {
        D3D11_RASTERIZER_DESC rd{};
        rd.FillMode = D3D11_FILL_SOLID;      // Para depurar cambia a D3D11_FILL_WIREFRAME
        rd.CullMode = D3D11_CULL_NONE;       // Sin culling para OBJ exportados con winding distinto
        rd.DepthClipEnable = TRUE;

        ID3D11RasterizerState* rs = nullptr;
        if (SUCCEEDED(m_device.m_device->CreateRasterizerState(&rd, &rs)) && rs) {
            m_deviceContext.RSSetState(rs);
            SAFE_RELEASE(rs);
        }
    }

    return S_OK;
}

void BaseApp::update(float deltaTime) {
    // Animación básica de color
    static float t = 0.0f;
    if (m_swapChain.m_driverType == D3D_DRIVER_TYPE_REFERENCE) {
        t += (float)XM_PI * 0.0125f;
    }
    else {
        static DWORD t0 = 0;
        DWORD tc = GetTickCount();
        if (t0 == 0) t0 = tc;
        t = (tc - t0) * 0.001f;
    }

    // Actualiza CB view/projection (por si la ventana cambió)
    cbNeverChanges.mView = XMMatrixTranspose(m_View);
    m_cbNeverChanges.update(m_deviceContext, nullptr, 0, nullptr, &cbNeverChanges, 0, 0);

    m_Projection = XMMatrixPerspectiveFovLH(
        XM_PIDIV4,
        (float)m_window.m_width / (float)m_window.m_height,
        0.01f,
        1000.0f
    );
    cbChangesOnResize.mProjection = XMMatrixTranspose(m_Projection);
    m_cbChangeOnResize.update(m_deviceContext, nullptr, 0, nullptr, &cbChangesOnResize, 0, 0);

    // Color animado
    m_vMeshColor.x = (sinf(t * 1.0f) + 1.0f) * 0.5f;
    m_vMeshColor.y = (cosf(t * 3.0f) + 1.0f) * 0.5f;
    m_vMeshColor.z = (sinf(t * 5.0f) + 1.0f) * 0.5f;
    m_vMeshColor.w = 1.0f;

    // World (si quieres rotación, descomenta)
    // m_World = XMMatrixRotationY(t * 0.5f);

    cb.mWorld = XMMatrixTranspose(m_World);
    cb.vMeshColor = m_vMeshColor;
    m_cbChangesEveryFrame.update(m_deviceContext, nullptr, 0, nullptr, &cb, 0, 0);
}

void BaseApp::render() {
    // Clear
    const float ClearColor[4] = { 0.12f, 0.12f, 0.12f, 1.0f };
    m_renderTargetView.render(m_deviceContext, m_depthStencilView, 1, ClearColor);

    // Viewport + clear depth
    m_viewport.render(m_deviceContext);
    m_depthStencilView.render(m_deviceContext);

    // Shaders + input layout
    m_shaderProgram.render(m_deviceContext);

    // Buffers
    m_vertexBuffer.render(m_deviceContext, 0, 1);
    m_indexBuffer.render(m_deviceContext, 0, 1, false, DXGI_FORMAT_R32_UINT);

    // Const buffers
    m_cbNeverChanges.render(m_deviceContext, 0, 1);
    m_cbChangeOnResize.render(m_deviceContext, 1, 1);
    m_cbChangesEveryFrame.render(m_deviceContext, 2, 1);
    m_cbChangesEveryFrame.render(m_deviceContext, 2, 1, true); // PS

    // Sampler
    m_samplerState.render(m_deviceContext, 0, 1);

    // Draw
    m_deviceContext.DrawIndexed(m_mesh.m_numIndex, 0, 0);

    // Present
    m_swapChain.present();
}

void BaseApp::destroy() {
    if (m_deviceContext.m_deviceContext) m_deviceContext.m_deviceContext->ClearState();

    m_samplerState.destroy();
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

LRESULT BaseApp::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message)
    {
    case WM_CREATE:
        if (auto* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam)) {
            SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pCreate->lpCreateParams);
        }
        return 0;
    case WM_PAINT: {
        PAINTSTRUCT ps;
        BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
    } return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}
