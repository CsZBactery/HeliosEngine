#include "../include/Prerequisites.h"
#include "../include/Types.h"   
#include "../include/BaseApp.h"

// ctor trivial
BaseApp::BaseApp(HINSTANCE, int) {}

int BaseApp::run(HINSTANCE hInst, int nCmdShow) {
    if (FAILED(m_window.init(hInst, nCmdShow, WndProc))) return 0;
    if (FAILED(init())) return 0;

    MSG msg = {};
    LARGE_INTEGER freq{}, prev{};
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&prev);

    while (WM_QUIT != msg.message) {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else {
            LARGE_INTEGER curr{};
            QueryPerformanceCounter(&curr);
            const float dt = static_cast<float>(curr.QuadPart - prev.QuadPart) / freq.QuadPart;
            prev = curr;
            update(dt);
            render();
        }
    }
    return static_cast<int>(msg.wParam);
}

HRESULT BaseApp::init() {
    HRESULT hr = S_OK;

    // SwapChain / backbuffer
    hr = m_swapChain.init(m_device, m_deviceContext, m_backBuffer, m_window);
    if (FAILED(hr)) { ERROR(Main, InitDevice, L"SwapChain init failed"); return hr; }

    // RTV desde backbuffer
    hr = m_renderTargetView.init(m_device, m_backBuffer, DXGI_FORMAT_R8G8B8A8_UNORM);
    if (FAILED(hr)) { ERROR(Main, InitDevice, L"RTV init failed"); return hr; }

    // Depth + DSV (usar getters de Window)
    hr = m_depthStencil.init(
        m_device,
        m_window.width(), m_window.height(),
        DXGI_FORMAT_D24_UNORM_S8_UINT,
        D3D11_BIND_DEPTH_STENCIL,
        1, 0);
    if (FAILED(hr)) { ERROR(Main, InitDevice, L"DepthTexture init failed"); return hr; }

    hr = m_depthStencilView.init(m_device, m_depthStencil, DXGI_FORMAT_D24_UNORM_S8_UINT);
    if (FAILED(hr)) { ERROR(Main, InitDevice, L"DSV init failed"); return hr; }

    // Viewport
    hr = m_viewport.init(m_window);
    if (FAILED(hr)) { ERROR(Main, InitDevice, L"Viewport init failed"); return hr; }

    // Input layout (POSITION + TEXCOORD)
    std::vector<D3D11_INPUT_ELEMENT_DESC> Layout;
    {
        D3D11_INPUT_ELEMENT_DESC e{};
        e.SemanticName = "POSITION";
        e.SemanticIndex = 0;
        e.Format = DXGI_FORMAT_R32G32B32_FLOAT;
        e.InputSlot = 0;
        e.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
        e.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        e.InstanceDataStepRate = 0;
        Layout.push_back(e);

        e = {};
        e.SemanticName = "TEXCOORD";
        e.SemanticIndex = 0;
        e.Format = DXGI_FORMAT_R32G32_FLOAT;
        e.InputSlot = 0;
        e.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
        e.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        e.InstanceDataStepRate = 0;
        Layout.push_back(e);
    }

    // Shader program (.fx del profe)
    hr = m_shaderProgram.init(m_device, "WildvineEngine.fx", Layout);
    if (FAILED(hr)) { ERROR(Main, InitDevice, L"ShaderProgram init failed"); return hr; }

    // ==================
    // Geometría (cubo)
    // ==================
    SimpleVertex vertices[] = {
        {{-1,  1, -1},{0,0}}, {{ 1,  1, -1},{1,0}}, {{ 1,  1,  1},{1,1}}, {{-1,  1,  1},{0,1}},
        {{-1, -1, -1},{0,0}}, {{ 1, -1, -1},{1,0}}, {{ 1, -1,  1},{1,1}}, {{-1, -1,  1},{0,1}},
        {{-1, -1,  1},{0,0}}, {{-1, -1, -1},{1,0}}, {{-1,  1, -1},{1,1}}, {{-1,  1,  1},{0,1}},
        {{ 1, -1,  1},{0,0}}, {{ 1, -1, -1},{1,0}}, {{ 1,  1, -1},{1,1}}, {{ 1,  1,  1},{0,1}},
        {{-1, -1, -1},{0,0}}, {{ 1, -1, -1},{1,0}}, {{ 1,  1, -1},{1,1}}, {{-1,  1, -1},{0,1}},
        {{-1, -1,  1},{0,0}}, {{ 1, -1,  1},{1,0}}, {{ 1,  1,  1},{1,1}}, {{-1,  1,  1},{0,1}},
    };
    unsigned int indices[] = {
        3,1,0,  2,1,3,  6,4,5,  7,4,6,  11,9,8,  10,9,11,
        14,12,13, 15,12,14,  19,17,16,  18,17,19,  22,20,21,  23,20,22
    };

    // Llenar MeshComponent (versión del profe con m_vertex/m_index)
    m_mesh.m_vertex.assign(vertices, vertices + 24);
    m_mesh.m_index.assign(indices, indices + 36);
    m_mesh.m_numVertex = 24;
    m_mesh.m_numIndex = 36;

    // Buffers GPU
    hr = m_vertexBuffer.init(m_device, m_mesh, D3D11_BIND_VERTEX_BUFFER);
    if (FAILED(hr)) { ERROR(Main, InitDevice, L"VertexBuffer init failed"); return hr; }

    hr = m_indexBuffer.init(m_device, m_mesh, D3D11_BIND_INDEX_BUFFER);
    if (FAILED(hr)) { ERROR(Main, InitDevice, L"IndexBuffer init failed"); return hr; }

    // Topología
    m_deviceContext.IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Constant buffers
    if (FAILED(m_cbNeverChanges.init(m_device, sizeof(CBNeverChanges))))           return E_FAIL;
    if (FAILED(m_cbChangeOnResize.init(m_device, sizeof(CBChangeOnResize))))       return E_FAIL;
    if (FAILED(m_cbChangesEveryFrame.init(m_device, sizeof(CBChangesEveryFrame)))) return E_FAIL;

    // Textura + sampler
    if (FAILED(m_textureCube.init(m_device, "seafloor", ExtensionType::DDS)))      return E_FAIL;
    if (FAILED(m_samplerState.init(m_device)))                                     return E_FAIL;

    // Cámaras
    using namespace DirectX;
    m_World = XMMatrixIdentity();
    auto Eye = XMVectorSet(0.0f, 3.0f, -6.0f, 0.0f);
    auto At = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    auto Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    m_View = XMMatrixLookAtLH(Eye, At, Up);

    cbNeverChanges.mView = XMMatrixTranspose(m_View);
    m_Projection = XMMatrixPerspectiveFovLH(
        XM_PIDIV4,
        static_cast<float>(m_window.width()) / static_cast<float>(m_window.height()),
        0.01f, 100.0f);
    cbChangesOnResize.mProjection = XMMatrixTranspose(m_Projection);

    return S_OK;
}

void BaseApp::update(float dt) {
    static float t = 0.0f; t += dt;

    // View/Proj
    cbNeverChanges.mView = DirectX::XMMatrixTranspose(m_View);
    m_cbNeverChanges.update(m_deviceContext, nullptr, 0, nullptr, &cbNeverChanges, 0, 0);

    m_Projection = DirectX::XMMatrixPerspectiveFovLH(
        DirectX::XM_PIDIV4,
        static_cast<float>(m_window.width()) / static_cast<float>(m_window.height()),
        0.01f, 100.0f);
    cbChangesOnResize.mProjection = DirectX::XMMatrixTranspose(m_Projection);
    m_cbChangeOnResize.update(m_deviceContext, nullptr, 0, nullptr, &cbChangesOnResize, 0, 0);

    // Color animado
    m_vMeshColor.x = (sinf(t * 1.0f) + 1.0f) * 0.5f;
    m_vMeshColor.y = (cosf(t * 3.0f) + 1.0f) * 0.5f;
    m_vMeshColor.z = (sinf(t * 5.0f) + 1.0f) * 0.5f;
    m_vMeshColor.w = 1.0f;

    // Mundo + CB frame
    m_World = DirectX::XMMatrixRotationY(t);
    cb.mWorld = DirectX::XMMatrixTranspose(m_World);
    cb.vMeshColor = m_vMeshColor;
    m_cbChangesEveryFrame.update(m_deviceContext, nullptr, 0, nullptr, &cb, 0, 0);
}

void BaseApp::render() {
    // RTV + clear
    float ClearColor[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
    m_renderTargetView.render(m_deviceContext, m_depthStencilView, 1, ClearColor);

    // Viewport + DSV
    m_viewport.render(m_deviceContext);
    m_depthStencilView.render(m_deviceContext);

    // Shaders
    m_shaderProgram.render(m_deviceContext);

    // Buffers (geom)
    m_vertexBuffer.render(m_deviceContext, 0, 1);
    // m_index es de tipo unsigned int -> DXGI_FORMAT_R32_UINT
    m_indexBuffer.render(m_deviceContext, 0, 1, false, DXGI_FORMAT_R32_UINT);

    // Const buffers VS/PS
    m_cbNeverChanges.render(m_deviceContext, 0, 1);
    m_cbChangeOnResize.render(m_deviceContext, 1, 1);
    m_cbChangesEveryFrame.render(m_deviceContext, 2, 1);
    m_cbChangesEveryFrame.render(m_deviceContext, 2, 1, true); // también al PS

    // Textura + sampler
    m_textureCube.render(m_deviceContext, 0, 1);
    m_samplerState.render(m_deviceContext, 0, 1);

    // Draw
    m_deviceContext.DrawIndexed(m_mesh.m_numIndex, 0, 0);

    // Present
    m_swapChain.present();
}

void BaseApp::destroy() {
    m_deviceContext.clearState();

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

LRESULT CALLBACK BaseApp::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE: {
        auto* cs = reinterpret_cast<CREATESTRUCT*>(lParam);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)cs->lpCreateParams);
    } return 0;
    case WM_PAINT: {
        PAINTSTRUCT ps; BeginPaint(hWnd, &ps); EndPaint(hWnd, &ps);
    } return 0;
    case WM_DESTROY: PostQuitMessage(0); return 0;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}
