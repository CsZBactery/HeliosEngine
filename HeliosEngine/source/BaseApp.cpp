#include "../include/BaseApp.h"
#include "../include/ResourceManager.h"
#include <direct.h> 

// Inicialización temprana de recursos externos o DLLs
HRESULT BaseApp::awake() {
    HRESULT hr = S_OK;

    // Inicialización de dlls y elementos externos al motor si fuera necesario.

    // Log Success Message
    MESSAGE("Main", "Awake", "Application awake successfully.");
    return hr;
}

int BaseApp::run(HINSTANCE hInst, int nCmdShow) {
    // 1) Initialize Window
    if (FAILED(m_window.init(hInst, nCmdShow, WndProc))) {
        ERROR("Main", "Run", "Failed to initialize window.");
        return 0;
    }

    // 2) Awake Application
    if (FAILED(awake())) {
        ERROR("Main", "Run", "Failed to awake application.");
        return 0;
    }

    // 3) Initialize Device and Device Context
    // Nota: En esta arquitectura, el Device se inicializa dentro de init()
    if (FAILED(init())) {
        ERROR("Main", "Run", "Failed to initialize device and device context.");
        return 0;
    }

    // Main message loop
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
    return (int)msg.wParam;
}

HRESULT BaseApp::init() {
    HRESULT hr = S_OK;

    // --------------------------------------------------------
    // INICIALIZACION DE DEVICE Y SWAPCHAIN
    // --------------------------------------------------------

    // Primero inicializamos el dispositivo
    m_device.init();
    // Obtenemos el contexto inmediato
    m_device.m_device->GetImmediateContext(&m_deviceContext.m_deviceContext);

    // Crear swapchain
    hr = m_swapChain.init(m_device, m_deviceContext, m_backBuffer, m_window);
    if (FAILED(hr)) {
        ERROR("Main", "InitDevice", ("Failed to initialize SwpaChian. HRESULT: " + std::to_string(hr)).c_str());
        return hr;
    }

    // Crear render target view
    hr = m_renderTargetView.init(m_device, m_backBuffer, DXGI_FORMAT_R8G8B8A8_UNORM);
    if (FAILED(hr)) {
        ERROR("Main", "InitDevice", ("Failed to initialize RenderTargetView. HRESULT: " + std::to_string(hr)).c_str());
        return hr;
    }

    // Crear textura de depth stencil
    hr = m_depthStencil.init(m_device,
        m_window.m_width,
        m_window.m_height,
        DXGI_FORMAT_D24_UNORM_S8_UINT,
        D3D11_BIND_DEPTH_STENCIL,
        4,
        0); // SampleQuality 0 según referencia
    if (FAILED(hr)) {
        ERROR("Main", "InitDevice", ("Failed to initialize DepthStencil. HRESULT: " + std::to_string(hr)).c_str());
        return hr;
    }

    // Crear el depth stencil view
    hr = m_depthStencilView.init(m_device, m_depthStencil, DXGI_FORMAT_D24_UNORM_S8_UINT);
    if (FAILED(hr)) {
        ERROR("Main", "InitDevice", ("Failed to initialize DepthStencilView. HRESULT: " + std::to_string(hr)).c_str());
        return hr;
    }

    // Crear el viewport
    hr = m_viewport.init(m_window);
    if (FAILED(hr)) {
        ERROR("Main", "InitDevice", ("Failed to initialize Viewport. HRESULT: " + std::to_string(hr)).c_str());
        return hr;
    }

    // --------------------------------------------------------
    // CARGA DE RECURSOS (MOTO REPSOL)
    // --------------------------------------------------------

    // Set Repsol Actor (Tu modelo)
    m_repsolActor = EU::MakeShared<Actor>(m_device);

    if (!m_repsolActor.isNull()) {
        std::vector<MeshComponent> repsolMeshes;

        // Carga del modelo OBJ (Tu lógica original)
        m_model = new Model3D("Assets/Moto/repsol3.obj", ModelType::OBJ);
        repsolMeshes = m_model->GetMeshes();

        // Carga de Textura (Tu lógica original)
        std::vector<Texture> repsolTextures;
        // Ruta original: "Assets/Textures/BaseColor"
        hr = m_repsolTexture.init(m_device, "Assets/Textures/BaseColor", ExtensionType::PNG);

        if (FAILED(hr)) {
            // Log de error detallado
            char fullPath[1024];
            _fullpath(fullPath, "Assets/Textures/BaseColor.png", 1024);
            std::string err = "Failed to initialize Repsol Texture. Path: " + std::string(fullPath) + " HRESULT: " + std::to_string(hr);
            ERROR("Main", "InitDevice", err.c_str());
            return hr;
        }
        repsolTextures.push_back(m_repsolTexture);

        // Asignación de recursos al Actor
        m_repsolActor->setMesh(m_device, repsolMeshes);
        m_repsolActor->setTextures(repsolTextures);
        m_repsolActor->setName("RepsolBike");
        m_actors.push_back(m_repsolActor);

        // Transformación inicial (Tus valores originales)
        m_repsolActor->getComponent<Transform>()->setTransform(
            EU::Vector3(0.0f, 0.0f, 0.0f),      // Posición
            EU::Vector3(0.0f, 0.0f, 0.0f),      // Rotación
            EU::Vector3(0.1f, 0.1f, 0.1f)       // Escala (reducida)
        );
    }
    else {
        ERROR("Main", "InitDevice", "Failed to create Repsol Actor.");
        return E_FAIL;
    }

    // --------------------------------------------------------
    // SHADERS Y LAYOUTS
    // --------------------------------------------------------

    // Define the input layout de forma explícita (Estilo referencia)
    std::vector<D3D11_INPUT_ELEMENT_DESC> Layout;

    D3D11_INPUT_ELEMENT_DESC position;
    position.SemanticName = "POSITION";
    position.SemanticIndex = 0;
    position.Format = DXGI_FORMAT_R32G32B32_FLOAT;
    position.InputSlot = 0;
    position.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
    position.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    position.InstanceDataStepRate = 0;
    Layout.push_back(position);

    D3D11_INPUT_ELEMENT_DESC texcoord;
    texcoord.SemanticName = "TEXCOORD";
    texcoord.SemanticIndex = 0;
    texcoord.Format = DXGI_FORMAT_R32G32_FLOAT;
    texcoord.InputSlot = 0;
    texcoord.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
    texcoord.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    texcoord.InstanceDataStepRate = 0;
    Layout.push_back(texcoord);

    D3D11_INPUT_ELEMENT_DESC normal;
    normal.SemanticName = "NORMAL";
    normal.SemanticIndex = 0;
    normal.Format = DXGI_FORMAT_R32G32B32_FLOAT;
    normal.InputSlot = 0;
    normal.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
    normal.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    normal.InstanceDataStepRate = 0;
    Layout.push_back(normal);

    // Create the Shader Program (Tu Shader: HeliosEngine.fx)
    // Intentamos ruta completa, si falla intentamos ruta relativa simple
    hr = m_shaderProgram.init(m_device, "Assets/Shaders/HeliosEngine.fx", Layout);
    if (FAILED(hr)) {
        hr = m_shaderProgram.init(m_device, "HeliosEngine.fx", Layout);
    }

    if (FAILED(hr)) {
        ERROR("Main", "InitDevice", ("Failed to initialize ShaderProgram (HeliosEngine.fx). HRESULT: " + std::to_string(hr)).c_str());
        return hr;
    }

    // --------------------------------------------------------
    // CONSTANT BUFFERS
    // --------------------------------------------------------

    hr = m_cbNeverChanges.init(m_device, sizeof(CBNeverChanges));
    if (FAILED(hr)) {
        ERROR("Main", "InitDevice", ("Failed to initialize NeverChanges Buffer. HRESULT: " + std::to_string(hr)).c_str());
        return hr;
    }

    hr = m_cbChangeOnResize.init(m_device, sizeof(CBChangeOnResize));
    if (FAILED(hr)) {
        ERROR("Main", "InitDevice", ("Failed to initialize ChangeOnResize Buffer. HRESULT: " + std::to_string(hr)).c_str());
        return hr;
    }

    // Initialize the view matrix
    XMVECTOR Eye = XMVectorSet(0.0f, 10.0f, -30.0f, 0.0f); // Zoom alejado para ver la moto
    XMVECTOR At = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
    XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    m_View = XMMatrixLookAtLH(Eye, At, Up);

    // Initialize the projection matrix
    cbNeverChanges.mView = XMMatrixTranspose(m_View);
    m_Projection = XMMatrixPerspectiveFovLH(XM_PIDIV4, m_window.m_width / (FLOAT)m_window.m_height, 0.01f, 100.0f);
    cbChangesOnResize.mProjection = XMMatrixTranspose(m_Projection);

    return S_OK;
}

void BaseApp::update(float deltaTime) {
    // Update our time (Lógica referencia)
    static float t = 0.0f;
    if (m_swapChain.m_driverType == D3D_DRIVER_TYPE_REFERENCE) {
        t += (float)XM_PI * 0.0125f;
    }
    else {
        static DWORD dwTimeStart = 0;
        DWORD dwTimeCur = GetTickCount();
        if (dwTimeStart == 0)
            dwTimeStart = dwTimeCur;
        t = (dwTimeCur - dwTimeStart) / 1000.0f;
    }

    // Actualizar la matriz de proyección y vista
    cbNeverChanges.mView = XMMatrixTranspose(m_View);
    m_cbNeverChanges.update(m_deviceContext, nullptr, 0, nullptr, &cbNeverChanges, 0, 0);

    m_Projection = XMMatrixPerspectiveFovLH(XM_PIDIV4, m_window.m_width / (FLOAT)m_window.m_height, 0.01f, 100.0f);
    cbChangesOnResize.mProjection = XMMatrixTranspose(m_Projection);
    m_cbChangeOnResize.update(m_deviceContext, nullptr, 0, nullptr, &cbChangesOnResize, 0, 0);

    // Update Actors
    for (auto& actor : m_actors) {
        actor->update(deltaTime, m_deviceContext);
    }
}

void BaseApp::render() {
    // Set Render Target View
    float ClearColor[4] = { 0.1f, 0.1f, 0.1f, 1.0f }; // Gris oscuro
    m_renderTargetView.render(m_deviceContext, m_depthStencilView, 1, ClearColor);

    // Set Viewport
    m_viewport.render(m_deviceContext);

    // Set depth stencil view
    m_depthStencilView.render(m_deviceContext);

    // Set shader program
    m_shaderProgram.render(m_deviceContext);

    // Asignar buffers constantes
    m_cbNeverChanges.render(m_deviceContext, 0, 1);
    m_cbChangeOnResize.render(m_deviceContext, 1, 1);

    // Render all actors
    for (auto& actor : m_actors) {
        actor->render(m_deviceContext);
    }

    // Present our back buffer to our front buffer
    m_swapChain.present();
}

void BaseApp::destroy() {
    if (m_deviceContext.m_deviceContext) m_deviceContext.m_deviceContext->ClearState();

    // Limpieza de recursos
    m_cbNeverChanges.destroy();
    m_cbChangeOnResize.destroy();
    m_shaderProgram.destroy();
    m_depthStencil.destroy();
    m_depthStencilView.destroy();
    m_renderTargetView.destroy();
    m_swapChain.destroy();
    m_backBuffer.destroy();
    m_deviceContext.destroy();
    m_device.destroy();

    // Cleanup de Model (Puntero raw)
    if (m_model) {
        delete m_model;
        m_model = nullptr;
    }
}

LRESULT BaseApp::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    // Handler de ImGui comentado
    // if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
    //   return true;

    switch (message) {
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