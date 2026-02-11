#include "BaseApp.h"
#include "ResourceManager.h"
#include <array>
#include <string>
#include "imgui.h"

// Variable global interna para el Rasterizer State (para no tocar el .h si no quieres)
// Idealmente esto iría en el .h como m_rasterizerState, pero aquí funciona para salir del paso.
ID3D11RasterizerState* g_pRasterizerStateNoCull = nullptr;

HRESULT
BaseApp::awake() {
    HRESULT hr = S_OK;
    m_sceneGraph.init();
    MESSAGE("Main", "Awake", "Application awake successfully.");
    return hr;
}

int
BaseApp::run(HINSTANCE hInst, int nCmdShow) {
    if (FAILED(m_window.init(hInst, nCmdShow, WndProc))) {
        ERROR("Main", "Run", "Failed to initialize window.");
        return 0;
    }
    if (FAILED(awake())) {
        ERROR("Main", "Run", "Failed to awake application.");
        return 0;
    }
    if (FAILED(init())) {
        ERROR("Main", "Run", "Failed to initialize device and device context.");
        return 0;
    }
    m_gui.init(m_window, m_device, m_deviceContext);

    MSG msg = {};
    LARGE_INTEGER freq, prev;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&prev);
    while (WM_QUIT != msg.message)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
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

HRESULT
BaseApp::init() {
    HRESULT hr = S_OK;

    m_device.init();
    m_device.m_device->GetImmediateContext(&m_deviceContext.m_deviceContext);

    hr = m_swapChain.init(m_device, m_deviceContext, m_backBuffer, m_window);
    if (FAILED(hr)) return hr;

    hr = m_renderTargetView.init(m_device, m_backBuffer, DXGI_FORMAT_R8G8B8A8_UNORM);
    if (FAILED(hr)) return hr;

    // Depth Stencil con Calidad 16 para coincidir con SwapChain
    hr = m_depthStencil.init(m_device, m_window.m_width, m_window.m_height,
        DXGI_FORMAT_D24_UNORM_S8_UINT, D3D11_BIND_DEPTH_STENCIL, 4, 16);
    if (FAILED(hr)) return hr;

    hr = m_depthStencilView.init(m_device, m_depthStencil, DXGI_FORMAT_D24_UNORM_S8_UINT);
    if (FAILED(hr)) return hr;

    hr = m_viewport.init(m_window);
    if (FAILED(hr)) return hr;

    // --- CONFIGURAR RASTERIZER (Ver ambas caras) ---
    D3D11_RASTERIZER_DESC rasterDesc;
    ZeroMemory(&rasterDesc, sizeof(rasterDesc));
    rasterDesc.FillMode = D3D11_FILL_SOLID;
    rasterDesc.CullMode = D3D11_CULL_NONE; // Dibuja todo
    rasterDesc.FrontCounterClockwise = false;
    rasterDesc.DepthClipEnable = true;
    rasterDesc.MultisampleEnable = true;

    m_device.m_device->CreateRasterizerState(&rasterDesc, &g_pRasterizerStateNoCull);
    m_deviceContext.m_deviceContext->RSSetState(g_pRasterizerStateNoCull);

    // --- CARGA DE RECURSOS ---
    std::array<std::string, 6> faces = {
        "Skybox/cubemap_0.png", "Skybox/cubemap_1.png", "Skybox/cubemap_2.png",
        "Skybox/cubemap_3.png", "Skybox/cubemap_4.png", "Skybox/cubemap_5.png"
    };
    m_skyboxTex.CreateCubemap(m_device, m_deviceContext, faces, true);

    m_repsolActor = EU::MakeShared<Actor>(m_device);
    if (!m_repsolActor.isNull()) {
        m_model = new Model3D("Assets/Moto/repsol3.obj", ModelType::OBJ);

        std::vector<Texture> repsolTextures;
        hr = m_repsolTexture.init(m_device, "Assets/Textures/BaseColor", ExtensionType::PNG);
        if (FAILED(hr)) {
            ERROR("Main", "Init", "Failed to load texture BaseColor.png");
            // No retornamos error fatal para que al menos se vea la forma
        }
        else {
            repsolTextures.push_back(m_repsolTexture);
        }

        m_repsolActor->setMesh(m_device, m_model->GetMeshes());
        m_repsolActor->setTextures(repsolTextures);
        m_repsolActor->setName("RepsolBike");

        // Escala normal (1,1,1)
        m_repsolActor->getComponent<Transform>()->setTransform(
            EU::Vector3(0, 0, 0), EU::Vector3(0, 0, 0), EU::Vector3(1.0f, 1.0f, 1.0f));

        m_actors.push_back(m_repsolActor);
    }

    for (auto& actor : m_actors) {
        m_sceneGraph.addEntity(actor.get());
    }

    // --- SHADERS ---
    std::vector<D3D11_INPUT_ELEMENT_DESC> Layout;
    D3D11_INPUT_ELEMENT_DESC position = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 };
    Layout.push_back(position);
    D3D11_INPUT_ELEMENT_DESC texcoord = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 };
    Layout.push_back(texcoord);
    D3D11_INPUT_ELEMENT_DESC normal = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 };
    Layout.push_back(normal);

    hr = m_shaderProgram.init(m_device, "Assets/Shaders/HeliosEngine.fx", Layout);
    if (FAILED(hr)) hr = m_shaderProgram.init(m_device, "HeliosEngine.fx", Layout);
    if (FAILED(hr)) return hr;

    m_cbNeverChanges.init(m_device, sizeof(CBNeverChanges));
    m_cbChangeOnResize.init(m_device, sizeof(CBChangeOnResize));

    // --- CAMARA ---
    m_camera.setLens(XM_PIDIV4, m_window.m_width / (float)m_window.m_height, 0.1f, 1000.0f);
    m_camera.setPosition(0.0f, 5.0f, -20.0f);

    // --- CORRECCIÓN FINAL: LUCES ---
    // ¡¡¡ESTO ES LO QUE TE FALTABA!!!
    cbNeverChanges.mView = XMMatrixTranspose(m_camera.getView());
    cbNeverChanges.mLightDir = XMVectorSet(-0.577f, -0.577f, 0.577f, 1.0f); // Luz diagonal
    cbNeverChanges.mLightColor = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);       // Luz Blanca

    // Enviamos los datos a la GPU ahora mismo
    m_cbNeverChanges.update(m_deviceContext, nullptr, 0, nullptr, &cbNeverChanges, 0, 0);

    cbChangesOnResize.mProjection = XMMatrixTranspose(m_camera.getProj());
    m_cbChangeOnResize.update(m_deviceContext, nullptr, 0, nullptr, &cbChangesOnResize, 0, 0);

    return S_OK;
}

void BaseApp::update(float deltaTime)
{
    static float t = 0.0f;
    static DWORD dwTimeStart = 0;
    DWORD dwTimeCur = GetTickCount();
    if (dwTimeStart == 0) dwTimeStart = dwTimeCur;
    t = (dwTimeCur - dwTimeStart) / 1000.0f;

    m_gui.update(m_viewport, m_window);

    if (!m_actors.empty() && m_gui.selectedActorIndex < m_actors.size()) {
        m_gui.inspectorGeneral(m_actors[m_gui.selectedActorIndex]);
    }
    m_gui.outliner(m_actors);

    // Skybox Debug
    static ID3D11ShaderResourceView* faceSRV[6] = { nullptr };
    if (!faceSRV[0]) {
        for (UINT i = 0; i < 6; ++i) {
            faceSRV[i] = m_skyboxTex.CreateCubemapFaceSRV(m_device.m_device, m_skyboxTex.m_texture, DXGI_FORMAT_R8G8B8A8_UNORM, i, 1);
        }
    }
    ImGui::Begin("Debug Skybox Faces");
    for (int i = 0; i < 6; ++i) {
        ImGui::Image((ImTextureID)faceSRV[i], ImVec2(64, 64));
        if ((i % 3) != 2) ImGui::SameLine();
    }
    ImGui::End();

    m_camera.updateViewMatrix();
    cbNeverChanges.mView = XMMatrixTranspose(m_camera.getView());
    // Mantenemos la luz actualizada
    cbNeverChanges.mLightDir = XMVectorSet(-0.577f, -0.577f, 0.577f, 1.0f);
    cbNeverChanges.mLightColor = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);

    m_cbNeverChanges.update(m_deviceContext, nullptr, 0, nullptr, &cbNeverChanges, 0, 0);
    m_cbChangeOnResize.update(m_deviceContext, nullptr, 0, nullptr, &cbChangesOnResize, 0, 0);

    m_sceneGraph.update(deltaTime, m_deviceContext);

    if (!m_actors.empty() && m_gui.selectedActorIndex < m_actors.size()) {
        m_gui.editTransform(m_camera.getView(), m_camera.getProj(), m_actors[m_gui.selectedActorIndex]);
    }
}

void
BaseApp::render() {
    float ClearColor[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
    m_renderTargetView.render(m_deviceContext, m_depthStencilView, 1, ClearColor);
    m_viewport.render(m_deviceContext);
    m_depthStencilView.render(m_deviceContext);

    // Activar Rasterizer (NO CULLING) para ver todo
    if (g_pRasterizerStateNoCull) {
        m_deviceContext.m_deviceContext->RSSetState(g_pRasterizerStateNoCull);
    }

    m_shaderProgram.render(m_deviceContext);
    m_cbNeverChanges.render(m_deviceContext, 0, 1);
    m_cbChangeOnResize.render(m_deviceContext, 1, 1);

    m_sceneGraph.render(m_deviceContext);
    m_gui.render();
    m_swapChain.present();
}

void
BaseApp::destroy() {
    if (m_deviceContext.m_deviceContext) m_deviceContext.m_deviceContext->ClearState();

    if (g_pRasterizerStateNoCull) { g_pRasterizerStateNoCull->Release(); g_pRasterizerStateNoCull = nullptr; }

    m_sceneGraph.destroy();
    m_cbNeverChanges.destroy();
    m_cbChangeOnResize.destroy();
    m_shaderProgram.destroy();
    m_depthStencil.destroy();
    m_depthStencilView.destroy();
    m_renderTargetView.destroy();
    m_swapChain.destroy();
    m_backBuffer.destroy();
    m_gui.destroy();
    m_deviceContext.destroy();
    m_device.destroy();

    if (m_model) { delete m_model; m_model = nullptr; }
}

LRESULT BaseApp::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    if (ImGui::GetCurrentContext() != nullptr) {
        if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam)) return true;
    }

    switch (message)
    {
    case WM_CREATE:
    {
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pCreate->lpCreateParams);
    }
    return 0;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
    }
    return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}