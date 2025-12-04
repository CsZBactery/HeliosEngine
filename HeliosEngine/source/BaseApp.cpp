#include "../include/BaseApp.h"
#include "../include/ResourceManager.h"
#include <direct.h> // Para imprimir directorio actual

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

BaseApp::BaseApp(HINSTANCE hInst, int nCmdShow) {}

int BaseApp::run(HINSTANCE hInst, int nCmdShow) {
    if (FAILED(m_window.init(hInst, nCmdShow, WndProc))) return 0;
    if (FAILED(init())) return 0;

    MSG msg = {};
    LARGE_INTEGER freq, prev;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&prev);

    while (WM_QUIT != msg.message) {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
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
    destroy();
    return (int)msg.wParam;
}

HRESULT BaseApp::init() {
    HRESULT hr = S_OK;

    // DEBUG: Imprimir dónde estamos buscando los archivos en la consola de salida
    char cwd[1024];
    if (_getcwd(cwd, sizeof(cwd))) {
        std::string pathMsg = "INFO: Directorio de Trabajo: " + std::string(cwd) + "\n";
        OutputDebugStringA(pathMsg.c_str());
    }

    // 1. INICIALIZAR HARDWARE
    m_device.init();
    if (!m_device.m_device) {
        ERROR("BaseApp", "init", "Critical: Device not initialized.");
        return E_FAIL;
    }
    m_device.m_device->GetImmediateContext(&m_deviceContext.m_deviceContext);

    // 2. RECURSOS DE VENTANA
    hr = m_swapChain.init(m_device, m_deviceContext, m_backBuffer, m_window);
    if (FAILED(hr)) return hr;

    hr = m_renderTargetView.init(m_device, m_backBuffer, DXGI_FORMAT_R8G8B8A8_UNORM);
    if (FAILED(hr)) return hr;

    // [CORRECCIÓN CRÍTICA DE MSAA]
    // Tu log indica que el RenderTarget tiene Quality=16. 
    // Forzamos 16 aquí para que coincidan.
    hr = m_depthStencil.init(m_device,
        m_window.m_width,
        m_window.m_height,
        DXGI_FORMAT_D24_UNORM_S8_UINT,
        D3D11_BIND_DEPTH_STENCIL,
        4,   // Sample Count (4x)
        16); // Sample Quality (16) <--- ESTO ELIMINA EL ERROR ROJO

    if (FAILED(hr)) {
        ERROR("BaseApp", "init", "Failed to create DepthStencil Texture.");
        return hr;
    }

    // Usamos los mismos parámetros para la vista
    hr = m_depthStencilView.init(m_device, m_depthStencil, DXGI_FORMAT_D24_UNORM_S8_UINT);
    if (FAILED(hr)) return hr;

    hr = m_viewport.init(m_window);
    if (FAILED(hr)) return hr;

    // 3. CARGAR RECURSOS
    m_repsolActor = EU::MakeShared<Actor>(m_device);

    if (!m_repsolActor.isNull()) {
        // A) Cargar Modelo
        // Verifica si tu archivo se llama "repsol.obj" o "repsol3.obj" en la carpeta Assets/Moto
        m_model = new Model3D("Assets/Moto/repsol.obj", ModelType::OBJ);
        std::vector<MeshComponent> myMeshes = m_model->GetMeshes();

        if (myMeshes.empty()) {
            ERROR("BaseApp", "init", "ERROR: El modelo no cargo. Verifica el nombre (repsol.obj vs repsol3.obj) en Assets/Moto/");
        }

        // B) Cargar Textura
        std::vector<Texture> myTextures;
        hr = m_repsolTexture.init(m_device, "Assets/Textures/LV.png", ExtensionType::PNG);

        if (FAILED(hr)) {
            ERROR("BaseApp", "init", "ERROR: La textura no cargo. Verifica Assets/Textures/LV.png");
        }
        myTextures.push_back(m_repsolTexture);

        m_repsolActor->setMesh(m_device, myMeshes);
        m_repsolActor->setTextures(myTextures);
        m_repsolActor->setName("RepsolBike");
        m_actors.push_back(m_repsolActor);

        // Ajuste de escala por si el modelo es muy grande o pequeño
        m_repsolActor->getComponent<Transform>()->setTransform(
            EU::Vector3(0.0f, 0.0f, 0.0f),
            EU::Vector3(0.0f, 0.0f, 0.0f),
            EU::Vector3(0.1f, 0.1f, 0.1f)
        );
    }

    // 4. SHADERS
    std::vector<D3D11_INPUT_ELEMENT_DESC> Layout = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    hr = m_shaderProgram.init(m_device, "Assets/Shaders/HeliosEngine.fx", Layout);
    if (FAILED(hr)) {
        ERROR("BaseApp", "init", "ERROR: No se cargo Shader. Verifica Assets/Shaders/HeliosEngine.fx");
    }

    // 5. BUFFERS & UI
    m_cbNeverChanges.init(m_device, sizeof(CBNeverChanges));
    m_cbChangeOnResize.init(m_device, sizeof(CBChangeOnResize));

    // Cámara
    XMVECTOR Eye = XMVectorSet(0.0f, 10.0f, -30.0f, 0.0f);
    XMVECTOR At = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
    XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    m_View = XMMatrixLookAtLH(Eye, At, Up);

    cbNeverChanges.mView = XMMatrixTranspose(m_View);
    m_Projection = XMMatrixPerspectiveFovLH(XM_PIDIV4, m_window.m_width / (FLOAT)m_window.m_height, 0.01f, 100.0f);
    cbChangesOnResize.mProjection = XMMatrixTranspose(m_Projection);

    UI.init(m_window.m_hWnd, m_device.m_device, m_deviceContext.m_deviceContext);

    return S_OK;
}

void BaseApp::update(float deltaTime) {
    UI.update();
    ImGui::Begin("Control Moto");
    if (!m_repsolActor.isNull()) {
        auto t = m_repsolActor->getComponent<Transform>();
        if (t) {
            EU::Vector3 pos = t->getPosition(); float fPos[3] = { pos.x, pos.y, pos.z };
            if (ImGui::DragFloat3("Pos", fPos, 0.1f)) t->setPosition(EU::Vector3(fPos[0], fPos[1], fPos[2]));

            EU::Vector3 s = t->getScale(); float fS[3] = { s.x, s.y, s.z };
            if (ImGui::DragFloat3("Scale", fS, 0.01f)) t->setScale(EU::Vector3(fS[0], fS[1], fS[2]));

            EU::Vector3 r = t->getRotation(); float fR[3] = { r.x, r.y, r.z };
            if (ImGui::DragFloat3("Rot", fR, 0.1f)) t->setRotation(EU::Vector3(fR[0], fR[1], fR[2]));
        }
    }
    ImGui::End();

    cbNeverChanges.mView = XMMatrixTranspose(m_View);
    m_cbNeverChanges.update(m_deviceContext, nullptr, 0, nullptr, &cbNeverChanges, 0, 0);
    m_cbChangeOnResize.update(m_deviceContext, nullptr, 0, nullptr, &cbChangesOnResize, 0, 0);

    for (auto& actor : m_actors) actor->update(deltaTime, m_deviceContext);
}

void BaseApp::render() {
    float ClearColor[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
    m_renderTargetView.render(m_deviceContext, m_depthStencilView, 1, ClearColor);

    m_viewport.render(m_deviceContext);
    m_depthStencilView.render(m_deviceContext);
    m_shaderProgram.render(m_deviceContext);

    m_cbNeverChanges.render(m_deviceContext, 0, 1);
    m_cbChangeOnResize.render(m_deviceContext, 1, 1);

    for (auto& actor : m_actors) actor->render(m_deviceContext);

    UI.render();
    m_swapChain.present();
}

void BaseApp::destroy() {
    UI.destroy();
    if (m_deviceContext.m_deviceContext) m_deviceContext.m_deviceContext->ClearState();
    if (m_model) { delete m_model; m_model = nullptr; }
    m_cbNeverChanges.destroy(); m_cbChangeOnResize.destroy(); m_shaderProgram.destroy();
    m_depthStencil.destroy(); m_depthStencilView.destroy(); m_renderTargetView.destroy();
    m_swapChain.destroy(); m_backBuffer.destroy(); m_deviceContext.destroy(); m_device.destroy();
}

LRESULT BaseApp::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam)) return true;
    switch (message) {
    case WM_CREATE: {
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pCreate->lpCreateParams);
    } return 0;
    case WM_PAINT: { PAINTSTRUCT ps; BeginPaint(hWnd, &ps); EndPaint(hWnd, &ps); } return 0;
    case WM_DESTROY: PostQuitMessage(0); return 0;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}