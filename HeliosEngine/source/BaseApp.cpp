#include "../include/BaseApp.h"

// =====================================================
// HLSL embebido – textura procedural (checker), sin SRV
// =====================================================
// HLSL embebido: cbuffers compatibles con tus C++ CBs
static const char* kHLSL = R"(
cbuffer CBNeverChanges     : register(b0) { float4x4 mView; };
cbuffer CBChangeOnResize   : register(b1) { float4x4 mProjection; };
cbuffer CBChangesEveryFrame: register(b2) { float4x4 mWorld; float4 vMeshColor; };

struct VS_IN  { float3 Pos: POSITION; float2 Tex: TEXCOORD0; };
struct VS_OUT { float4 Pos: SV_Position; float2 Tex: TEXCOORD0; };

VS_OUT VS(VS_IN i) {
    VS_OUT o;
    float4 p = float4(i.Pos, 1.0f);
    o.Pos = mul(p, mWorld);
    o.Pos = mul(o.Pos, mView);
    o.Pos = mul(o.Pos, mProjection);
    o.Tex = i.Tex;
    return o;
}

// Procedural checker (sin textura t0). Así NO dependemos de assets.
float4 PS(VS_OUT i) : SV_Target {
    float2 uv = i.Tex * 10.0;
    float2 f  = frac(uv);
    float c   = ( (f.x > 0.5) ^ (f.y > 0.5) ) ? 1.0 : 0.0;
    float4 a = float4(0.15, 0.15, 0.18, 1.0);
    float4 b = float4(0.85, 0.85, 0.90, 1.0);
    float4 col = lerp(a, b, c);
    return col * vMeshColor;
}
)";


// -----------------------------------------------------

BaseApp::BaseApp(HINSTANCE hInst, int nCmdShow) {}

// El wWinMain creado, pero con un método de clase
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

    // SwapChain + Device + Context + BackBuffer
    hr = m_swapChain.init(m_device, m_deviceContext, m_backBuffer, m_window);
    if (FAILED(hr)) return hr;

    // RTV desde backbuffer
    hr = m_renderTargetView.init(m_device, m_backBuffer, DXGI_FORMAT_R8G8B8A8_UNORM);
    if (FAILED(hr)) return hr;

    // DepthStencil
    hr = m_depthStencil.init(m_device,
        m_window.m_width, m_window.m_height,
        DXGI_FORMAT_D24_UNORM_S8_UINT,
        D3D11_BIND_DEPTH_STENCIL, 1, 0);
    if (FAILED(hr)) return hr;

    hr = m_depthStencilView.init(m_device, m_depthStencil, DXGI_FORMAT_D24_UNORM_S8_UINT);
    if (FAILED(hr)) return hr;

    // Viewport
    hr = m_viewport.init(m_window);
    if (FAILED(hr)) return hr;

    // Input layout (POSITION + TEXCOORD)
    std::vector<D3D11_INPUT_ELEMENT_DESC> Layout = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    // *** Shader desde MEMORIA (sin assets) ***
    hr = m_shaderProgram.initFromSource(m_device, kHLSL, Layout);
    if (FAILED(hr)) {
        ERROR("BaseApp", "init", "Failed to initialize ShaderProgram from source.");
        return hr;
    }


    // --------- Geometría: cubo ----------
    SimpleVertex vertices[] =
    {
        { XMFLOAT3(-1,  1, -1), XMFLOAT2(0,0) }, { XMFLOAT3(1,  1, -1), XMFLOAT2(1,0) },
        { XMFLOAT3(1,  1,  1), XMFLOAT2(1,1) }, { XMFLOAT3(-1,  1,  1), XMFLOAT2(0,1) },

        { XMFLOAT3(-1, -1, -1), XMFLOAT2(0,0) }, { XMFLOAT3(1, -1, -1), XMFLOAT2(1,0) },
        { XMFLOAT3(1, -1,  1), XMFLOAT2(1,1) }, { XMFLOAT3(-1, -1,  1), XMFLOAT2(0,1) },

        { XMFLOAT3(-1, -1,  1), XMFLOAT2(0,0) }, { XMFLOAT3(-1, -1, -1), XMFLOAT2(1,0) },
        { XMFLOAT3(-1,  1, -1), XMFLOAT2(1,1) }, { XMFLOAT3(-1,  1,  1), XMFLOAT2(0,1) },

        { XMFLOAT3(1, -1,  1), XMFLOAT2(0,0) }, { XMFLOAT3(1, -1, -1), XMFLOAT2(1,0) },
        { XMFLOAT3(1,  1, -1), XMFLOAT2(1,1) }, { XMFLOAT3(1,  1,  1), XMFLOAT2(0,1) },

        { XMFLOAT3(-1, -1, -1), XMFLOAT2(0,0) }, { XMFLOAT3(1, -1, -1), XMFLOAT2(1,0) },
        { XMFLOAT3(1,  1, -1), XMFLOAT2(1,1) }, { XMFLOAT3(-1,  1, -1), XMFLOAT2(0,1) },

        { XMFLOAT3(-1, -1,  1), XMFLOAT2(0,0) }, { XMFLOAT3(1, -1,  1), XMFLOAT2(1,0) },
        { XMFLOAT3(1,  1,  1), XMFLOAT2(1,1) }, { XMFLOAT3(-1,  1,  1), XMFLOAT2(0,1) },
    };
    unsigned int indices[] =
    {
        3,1,0,  2,1,3,
        6,4,5,  7,4,6,
        11,9,8, 10,9,11,
        14,12,13, 15,12,14,
        19,17,16, 18,17,19,
        22,20,21, 23,20,22
    };

    for (unsigned i = 0; i < 24; ++i) m_mesh.m_vertex.push_back(vertices[i]);
    m_mesh.m_numVertex = 24;
    for (unsigned i = 0; i < 36; ++i) m_mesh.m_index.push_back(indices[i]);
    m_mesh.m_numIndex = 36;

    hr = m_vertexBuffer.init(m_device, m_mesh, D3D11_BIND_VERTEX_BUFFER);
    if (FAILED(hr)) return hr;
    hr = m_indexBuffer.init(m_device, m_mesh, D3D11_BIND_INDEX_BUFFER);
    if (FAILED(hr)) return hr;

    m_deviceContext.IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Constant buffers
    if (FAILED(m_cbNeverChanges.init(m_device, sizeof(CBNeverChanges))))       return E_FAIL;
    if (FAILED(m_cbChangeOnResize.init(m_device, sizeof(CBChangeOnResize))))   return E_FAIL;
    if (FAILED(m_cbChangesEveryFrame.init(m_device, sizeof(CBChangesEveryFrame)))) return E_FAIL;

    // Matrices iniciales
    m_World = XMMatrixIdentity();
    XMVECTOR Eye = XMVectorSet(0.0f, 3.0f, -6.0f, 0.0f);
    XMVECTOR At = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    m_View = XMMatrixLookAtLH(Eye, At, Up);

    cbNeverChanges.mView = XMMatrixTranspose(m_View);
    m_Projection = XMMatrixPerspectiveFovLH(
        XM_PIDIV4, (FLOAT)m_window.m_width / (FLOAT)m_window.m_height, 0.01f, 100.0f);
    cbChangesOnResize.mProjection = XMMatrixTranspose(m_Projection);

    // Color base blanco (para no “tintar” el checker)
    m_vMeshColor = XMFLOAT4(1, 1, 1, 1);

    return S_OK;
}

void BaseApp::update(float /*deltaTime*/) {
    // Rotación simple con tiempo
    static DWORD t0 = GetTickCount();
    float secs = (GetTickCount() - t0) / 1000.0f;

    cbNeverChanges.mView = XMMatrixTranspose(m_View);
    m_cbNeverChanges.update(m_deviceContext, nullptr, 0, nullptr, &cbNeverChanges, 0, 0);

    m_Projection = XMMatrixPerspectiveFovLH(
        XM_PIDIV4, (FLOAT)m_window.m_width / (FLOAT)m_window.m_height, 0.01f, 100.0f);
    cbChangesOnResize.mProjection = XMMatrixTranspose(m_Projection);
    m_cbChangeOnResize.update(m_deviceContext, nullptr, 0, nullptr, &cbChangesOnResize, 0, 0);

    m_World = XMMatrixRotationY(secs);
    cb.mWorld = XMMatrixTranspose(m_World);
    cb.vMeshColor = m_vMeshColor; // (1,1,1,1)
    m_cbChangesEveryFrame.update(m_deviceContext, nullptr, 0, nullptr, &cb, 0, 0);
}

void BaseApp::render() {
    float ClearColor[4] = { 0.08f, 0.08f, 0.1f, 1.0f };
    m_renderTargetView.render(m_deviceContext, m_depthStencilView, 1, ClearColor);

    m_viewport.render(m_deviceContext);
    m_depthStencilView.render(m_deviceContext);

    // Shaders
    m_shaderProgram.render(m_deviceContext);

    // Buffers
    m_vertexBuffer.render(m_deviceContext, 0, 1);
    m_indexBuffer.render(m_deviceContext, 0, 1, false, DXGI_FORMAT_R32_UINT);

    // Constant buffers VS/PS
    m_cbNeverChanges.render(m_deviceContext, 0, 1);     // b0 VS
    m_cbChangeOnResize.render(m_deviceContext, 1, 1);     // b1 VS
    m_cbChangesEveryFrame.render(m_deviceContext, 2, 1);   // b2 VS
    m_cbChangesEveryFrame.render(m_deviceContext, 2, 1, true); // b2 PS

    // ¡OJO! No hay textura/sampler que bindear
    m_deviceContext.DrawIndexed(m_mesh.m_numIndex, 0, 0);

    m_swapChain.present();
}

void BaseApp::destroy() {
    if (m_deviceContext.m_deviceContext) m_deviceContext.m_deviceContext->ClearState();

    // (No usamos textura/sampler aquí, pero si existen, se limpian)
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

LRESULT BaseApp::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message)
    {
    case WM_CREATE: {
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pCreate->lpCreateParams);
    } return 0;
    case WM_PAINT: {
        PAINTSTRUCT ps; BeginPaint(hWnd, &ps); EndPaint(hWnd, &ps);
    } return 0;
    case WM_DESTROY: PostQuitMessage(0); return 0;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}
