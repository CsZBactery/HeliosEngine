// ======================================================================================
// Archivo: BaseApp.cpp
// Implementación de la clase principal del motor (HeliosEngine). 
// Controla el Game Loop, inicialización de DirectX y renderizado general.
// ======================================================================================

#include "BaseApp.h"
#include "ResourceManager.h"

// ======================================================================================
// FASE 1: AWAKE (Preparación Lógica)
// ======================================================================================
HRESULT
BaseApp::awake() {
    HRESULT hr = S_OK;

    // Inicializacion de dlls y elementos externos al motor (Ej. Grafo de Escena).
    m_sceneGraph.init();

    // Log Success Message
    MESSAGE("Main", "Awake", "Application awake successfully.");
    return hr;
}

// ======================================================================================
// FASE 2: BUCLE PRINCIPAL (Game Loop)
// ======================================================================================
int
BaseApp::run(HINSTANCE hInst, int nCmdShow) {
    // 1) Inicializar la ventana del sistema operativo Windows
    // ¡Ojo! Le pasamos 'this' para que el WndProc pueda comunicarse con nuestra clase
    if (FAILED(m_window.init(hInst, nCmdShow, WndProc, this))) {
        ERROR("Main", "Run", "Failed to initialize window.");
        return 0;
    }
    // 2) Despertar los subsistemas lógicos del motor
    if (FAILED(awake())) {
        ERROR("Main", "Run", "Failed to awake application.");
        return 0;
    }
    // 3) Iniciar DirectX 11 y cargar recursos (Texturas, Modelos) 
    if (FAILED(init())) {
        ERROR("Main", "Run", "Failed to initialize device and device context.");
        return 0;
    }
    // 4) Inicializar la interfaz gráfica de usuario (ImGui)
    m_gui.init(m_window, m_device, m_deviceContext);

    // Preparación del temporizador de alta resolución para calcular el Delta Time
    MSG msg = {};
    LARGE_INTEGER freq, prev;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&prev);

    // Bucle infinito: Se ejecuta hasta que el usuario cierra la ventana
    while (WM_QUIT != msg.message)
    {
        // Procesar mensajes del SO (mouse, teclado, redimensionado)
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            // Calcular el tiempo transcurrido desde el último frame
            LARGE_INTEGER curr;
            QueryPerformanceCounter(&curr);
            float deltaTime = static_cast<float>(curr.QuadPart - prev.QuadPart) / freq.QuadPart;
            prev = curr;

            // Actualizar lógica y dibujar en pantalla
            update(deltaTime);
            render();
        }
    }
    return (int)msg.wParam;
}

// ======================================================================================
// FASE 3: INICIALIZACIÓN DE HARDWARE (DirectX 11) Y CARGA DE RECURSOS
// ======================================================================================
HRESULT
BaseApp::init() {
    HRESULT hr = S_OK;

    // 1. Crear SwapChain (Doble Buffer e Inicialización del Device implícita)
    hr = m_swapChain.init(m_device, m_deviceContext, m_backBuffer, m_window);
    if (FAILED(hr)) {
        ERROR("Main", "InitDevice", ("Failed to initialize SwapChain. HRESULT: " + std::to_string(hr)).c_str());
        return hr;
    }

    // 2. Crear el lienzo principal (Render Target)
    hr = m_renderTargetView.init(m_device, m_backBuffer, DXGI_FORMAT_R8G8B8A8_UNORM);
    if (FAILED(hr)) {
        ERROR("Main", "InitDevice", ("Failed to initialize RenderTargetView. HRESULT: " + std::to_string(hr)).c_str());
        return hr;
    }

    // 3. Crear Buffer de Profundidad (Z-Buffer Principal)
    hr = m_depthStencil.init(m_device, m_window.m_width, m_window.m_height, DXGI_FORMAT_D24_UNORM_S8_UINT, D3D11_BIND_DEPTH_STENCIL, 4, 0);
    if (FAILED(hr)) {
        ERROR("Main", "InitDevice", ("Failed to initialize DepthStencil. HRESULT: " + std::to_string(hr)).c_str());
        return hr;
    }

    hr = m_depthStencilView.init(m_device, m_depthStencil, DXGI_FORMAT_D24_UNORM_S8_UINT);
    if (FAILED(hr)) {
        ERROR("Main", "InitDevice", ("Failed to initialize DepthStencilView. HRESULT: " + std::to_string(hr)).c_str());
        return hr;
    }

    // 4. Configurar Viewport Principal
    hr = m_viewport.init(m_window);
    if (FAILED(hr)) {
        ERROR("Main", "InitDevice", ("Failed to initialize Viewport. HRESULT: " + std::to_string(hr)).c_str());
        return hr;
    }

    m_d3dReady = true;

    // =========================================================
    // CARGA DE RECURSOS (MODELOS Y TEXTURAS)
    // =========================================================

    // A) Cargar texturas del Entorno (Skybox)
    std::array<std::string, 6> faces = {
        "Skybox/cubemap_0.png",
        "Skybox/cubemap_1.png",
        "Skybox/cubemap_2.png",
        "Skybox/cubemap_3.png",
        "Skybox/cubemap_4.png",
        "Skybox/cubemap_5.png"
    };
    m_skyboxTex.CreateCubemap(m_device, m_deviceContext, faces, false);

    // B) Crear y ensamblar el Actor Principal (CyberGun con PBR)
    m_cyberGun = EU::MakeShared<Actor>(m_device);

    if (!m_cyberGun.isNull()) {
        // Cargar Modelo FBX
        std::vector<MeshComponent> cyberGunMeshes;
        m_model = new Model3D("CyberGun.fbx", ModelType::FBX);
        cyberGunMeshes = m_model->GetMeshes();

        // Cargar Texturas PBR (Physically Based Rendering)
        std::vector<Texture> cyberGunTextures;
        hr = m_AlbedoSRV.init(m_device, "Textures/CyberGun/base.tga", PNG);
        if (FAILED(hr)) return hr;

        hr = m_MetallicSRV.init(m_device, "Textures/CyberGun/metallic.tga", PNG);
        if (FAILED(hr)) return hr;

        hr = m_RoughnessSRV.init(m_device, "Textures/CyberGun/roughness.tga", PNG);
        if (FAILED(hr)) return hr;

        hr = m_AOSRV.init(m_device, "Textures/CyberGun/ao.tga", PNG);
        if (FAILED(hr)) return hr;

        hr = m_NormalSRV.init(m_device, "Textures/CyberGun/normal.tga", PNG);
        if (FAILED(hr)) return hr;

        // El orden de inserción es crítico para los registros del Pixel Shader (t0, t1, t2...)
        cyberGunTextures.push_back(m_AlbedoSRV);
        cyberGunTextures.push_back(m_NormalSRV);
        cyberGunTextures.push_back(m_MetallicSRV);
        cyberGunTextures.push_back(m_RoughnessSRV);
        cyberGunTextures.push_back(m_AOSRV);

        // Ensamblaje del Actor
        m_cyberGun->setMesh(m_device, cyberGunMeshes);
        m_cyberGun->setTextures(cyberGunTextures);
        m_cyberGun->setName("CyberGun");
        m_actors.push_back(m_cyberGun);

        // Posición Inicial para que se vea bien en cámara
        m_cyberGun->getComponent<Transform>()->setTransform(
            EU::Vector3(2.0f, -1.90f, 11.60f),
            EU::Vector3(-0.60f, 3.0f, -0.20f),
            EU::Vector3(1.0f, 1.0f, 1.0f)
        );
    }
    else {
        ERROR("Main", "InitDevice", "Failed to create cyber Gun Actor.");
        return E_FAIL;
    }

    // Registrar actores en el Grafo de Escena
    for (auto& actor : m_actors) {
        m_sceneGraph.addEntity(actor.get());
    }

    // =========================================================
    // CONFIGURACIÓN DEL PIPELINE GRÁFICO (SHADERS Y BUFFERS)
    // =========================================================

    // 5. Configurar Input Layout avanzado (Incluye Tangentes para Normal Mapping)
    LayoutBuilder builder;
    builder.Add("POSITION", DXGI_FORMAT_R32G32B32_FLOAT)
        .Add("NORMAL", DXGI_FORMAT_R32G32B32_FLOAT)
        .Add("TANGENT", DXGI_FORMAT_R32G32B32_FLOAT)
        .Add("BITANGENT", DXGI_FORMAT_R32G32B32_FLOAT)
        .Add("TEXCOORD", DXGI_FORMAT_R32G32_FLOAT);

    // Crear el Programa de Shader PBR
    hr = m_shaderProgram.init(m_device, "PBRShader.hlsl", builder);
    if (FAILED(hr)) {
        ERROR("Main", "InitDevice", ("Failed to initialize ShaderProgram. HRESULT: " + std::to_string(hr)).c_str());
        return hr;
    }

    // Crear el Buffer Constante Principal unificado
    hr = m_constantBuffer.init(m_device, sizeof(CBMain));
    if (FAILED(hr)) {
        ERROR("Main", "InitDevice", ("Failed to initialize Constant Buffer. HRESULT: " + std::to_string(hr)).c_str());
        return hr;
    }

    // 6. Configurar Cámara Inicial
    m_camera.setLens(XM_PIDIV4, m_window.m_width / (float)m_window.m_height, 0.01f, 100.0f);
    m_camera.setPosition(0.0f, 3.0f, -6.0f);

    // Configurar Luz Global Falsa (Direccional)
    m_constantBufferStruct.LightColor = EU::Vector3(1.0f, 1.0f, 1.0f);
    m_constantBufferStruct.LightDir = EU::Vector3(-0.20f, -1.0f, 1.0f);

    // 7. Inicializar Subsistemas del Motor
    m_skybox.init(m_device, &m_deviceContext, m_skyboxTex);

    hr = m_defaultRasterizer.init(m_device, D3D11_FILL_SOLID, D3D11_CULL_BACK, false, true);
    if (FAILED(hr)) return hr;

    hr = m_defaultDepthStencil.init(m_device, true, D3D11_DEPTH_WRITE_MASK_ALL, D3D11_COMPARISON_LESS);
    if (FAILED(hr)) return hr;

    // 8. Inicializar el Pase de Editor (Render To Texture para ImGui)
    hr = m_editorViewportPass.init(m_device, 1280, 720);
    if (FAILED(hr)) {
        ERROR("Main", "InitDevice", ("Failed to initialize EditorViewportPass. HRESULT: " + std::to_string(hr)).c_str());
        return hr;
    }

    return S_OK;
}

// ======================================================================================
// FASE 4: UPDATE (Lógica de cada Frame)
// ======================================================================================
void
BaseApp::update(float deltaTime) {

    // Actualización de la GUI (ImGui)
    m_gui.update(m_viewport, m_window);

    // Dibuja la textura del juego dentro del panel de ImGui
    m_gui.drawViewportPanel(m_editorViewportPass.getSRV());

    // Dibuja los paneles de herramientas del editor
    m_gui.outliner(m_actors);
    if (m_gui.selectedActorIndex >= 0 && m_gui.selectedActorIndex < m_actors.size()) {
        m_gui.inspectorGeneral(m_actors[m_gui.selectedActorIndex]);
        m_gui.editTransform(m_camera, m_window, m_actors[m_gui.selectedActorIndex]);
    }

    // =========================================================
    // LÓGICA DE REDIMENSIONAMIENTO DEL EDITOR VIEWPORT (Debounce)
    // =========================================================
    unsigned int desiredW = static_cast<unsigned int>(m_gui.m_viewportSize.x);
    unsigned int desiredH = static_cast<unsigned int>(m_gui.m_viewportSize.y);
    const unsigned int kMinViewportSize = 64;

    if (desiredW < kMinViewportSize) desiredW = kMinViewportSize;
    if (desiredH < kMinViewportSize) desiredH = kMinViewportSize;

    // Si cambió el tamaño solicitado, reiniciamos el contador de estabilidad
    if (desiredW != m_lastRequestedViewportWidth || desiredH != m_lastRequestedViewportHeight) {
        m_lastRequestedViewportWidth = desiredW;
        m_lastRequestedViewportHeight = desiredH;
        m_viewportResizeStableFrames = 0;
    }
    else {
        m_viewportResizeStableFrames++;
    }

    // Solo pedimos recrear la textura a la GPU cuando el usuario dejó de arrastrar la ventana
    // por al menos un par de frames. Esto evita tirones graves de rendimiento (Stuttering).
    const int kStableFramesRequired = 2;
    if (m_viewportResizeStableFrames >= kStableFramesRequired) {
        if (desiredW != m_editorViewportPass.getWidth() || desiredH != m_editorViewportPass.getHeight()) {
            m_editorViewportResizePending = true;
            m_pendingViewportWidth = desiredW;
            m_pendingViewportHeight = desiredH;
        }
    }

    // Actualizar Matriz de Vista y Proyección de la cámara principal
    m_camera.updateViewMatrix();

    // Llenar la estructura de datos que viaja al Vertex Shader (PBR)
    XMStoreFloat4x4(&m_constantBufferStruct.View, XMMatrixTranspose(m_camera.getView()));
    XMStoreFloat4x4(&m_constantBufferStruct.Projection, XMMatrixTranspose(m_camera.getProj()));
    m_constantBufferStruct.CameraPos = m_camera.getPosition();

    // Controles de luz en la UI
    m_gui.vec3Control("Light Direction", &m_constantBufferStruct.LightDir.x, 0.1f);
    m_gui.vec3Control("Light Color", &m_constantBufferStruct.LightColor.x, 0.1f);

    // Actualizamos el Skybox (solo necesita rotación y proyección)
    m_skybox.update(m_deviceContext, m_camera);

    // Enviamos los datos actualizados al Buffer en la VRAM
    m_constantBuffer.update(m_deviceContext, nullptr, 0, nullptr, &m_constantBufferStruct, 0, 0);

    // Actualizar la jerarquía (matrices de mundo) de todos los actores
    m_sceneGraph.update(deltaTime, m_deviceContext);
}

// ======================================================================================
// FASE 5: RENDER (Dibujo en GPU)
// ======================================================================================
void
BaseApp::render() {

    // 0. Si el Viewport cambió de tamaño, recreamos sus texturas antes de dibujar
    handleEditorViewportResize();

    // =========================================================
    // PASE 1: DIBUJAR EL JUEGO EN LA TEXTURA DEL EDITOR
    // =========================================================
    float clearColor[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
    const float viewportClear[4] = { 0.10f, 0.10f, 0.10f, 1.0f };

    m_editorViewportPass.begin(m_deviceContext, viewportClear);
    m_editorViewportPass.setViewport(m_deviceContext);
    m_editorViewportPass.clearDepth(m_deviceContext);

    // A) Dibujar el Fondo
    m_skybox.render(m_deviceContext);

    // B) Restaurar estados por defecto y preparar pipeline para modelos 3D
    m_defaultRasterizer.render(m_deviceContext);
    m_defaultDepthStencil.render(m_deviceContext, 0, false);

    m_shaderProgram.render(m_deviceContext);
    m_constantBuffer.render(m_deviceContext, 0, 1, true); // true = Enviar también al Pixel Shader (Para la luz)

    // C) Dibujar la Escena (La CyberGun PBR)
    m_sceneGraph.render(m_deviceContext);


    // =========================================================
    // PASE 2: VOLVER AL BACKBUFFER PRINCIPAL (PANTALLA COMPLETA)
    // =========================================================
    m_renderTargetView.render(m_deviceContext, m_depthStencilView, 1, clearColor);
    m_viewport.render(m_deviceContext);
    m_depthStencilView.render(m_deviceContext);

    // Dibujar la interfaz de ImGui (Que incluye el panel con la textura generada en el Pase 1)
    m_gui.render();

    // 5. Intercambiar los buffers (Mostrar en pantalla)
    m_swapChain.present();
}

// ======================================================================================
// FASE 6: DESTROY (Limpieza de Memoria)
// ======================================================================================
void
BaseApp::destroy() {
    if (m_deviceContext.m_deviceContext) m_deviceContext.m_deviceContext->ClearState();

    m_sceneGraph.destroy();
    m_editorViewportPass.destroy();

    m_AlbedoSRV.destroy();
    m_MetallicSRV.destroy();
    m_NormalSRV.destroy();
    m_RoughnessSRV.destroy();
    m_AOSRV.destroy();

    m_defaultRasterizer.destroy();
    m_defaultDepthStencil.destroy();
    m_shaderProgram.destroy();

    m_depthStencil.destroy();
    m_depthStencilView.destroy();
    m_renderTargetView.destroy();
    m_swapChain.destroy();
    m_backBuffer.destroy();

    m_gui.destroy();
    m_deviceContext.destroy();
    m_device.destroy();
}

// ======================================================================================
// PROCEDIMIENTO DE VENTANA (Captura de Eventos Windows)
// ======================================================================================
LRESULT
BaseApp::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    // Le damos prioridad a ImGui para leer clics y teclas
    if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam)) {
        return true;
    }

    switch (message) {
    case WM_CREATE: {
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pCreate->lpCreateParams);
        return 0;
    }
    case WM_PAINT: {
        PAINTSTRUCT ps;
        BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
        return 0;
    }
    case WM_SIZE: {
        // Evita recrear buffers cuando la ventana está minimizada
        if (wParam == SIZE_MINIMIZED) return 0;

        UINT newW = LOWORD(lParam);
        UINT newH = HIWORD(lParam);
        if (newW == 0 || newH == 0) return 0;

        // Recupera la instancia de BaseApp y lanza el evento de redimensionado principal
        BaseApp* app = reinterpret_cast<BaseApp*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
        if (app) app->onResize(newW, newH);
        return 0;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}

// ======================================================================================
// GESTIÓN DE REDIMENSIONADO DE LA VENTANA PRINCIPAL (OS WINDOW)
// ======================================================================================
void BaseApp::onResize(UINT newW, UINT newH) {
    // 1) Si DirectX aún no inicializa, solo actualizamos el tamaño lógico
    if (!m_d3dReady) {
        m_window.m_width = (int)newW;
        m_window.m_height = (int)newH;
        return;
    }

    if (!m_deviceContext.m_deviceContext || !m_swapChain.m_swapChain) return;
    if (newW == 0 || newH == 0) return;

    m_window.m_width = (int)newW;
    m_window.m_height = (int)newH;

    // 2) Desbindear targets actuales (CLAVE antes de destruir buffers en la GPU)
    ID3D11RenderTargetView* nullRTV = nullptr;
    m_deviceContext.m_deviceContext->OMSetRenderTargets(1, &nullRTV, nullptr);

    // 3) Libera recursos dependientes del tamaño de la ventana
    m_renderTargetView.destroy();
    m_depthStencilView.destroy();
    m_depthStencil.destroy();
    m_backBuffer.destroy();

    // 4) Resize SwapChain
    HRESULT hr = m_swapChain.resizeBuffers(newW, newH);
    if (FAILED(hr)) return;

    // 5) Re-obtén backbuffer
    hr = m_swapChain.getBackBuffer(m_backBuffer);
    if (FAILED(hr)) return;

    // 6) Re-crea RTV
    hr = m_renderTargetView.init(m_device, m_backBuffer, DXGI_FORMAT_R8G8B8A8_UNORM);
    if (FAILED(hr)) return;

    // 7) Re-crea Depth y DSV
    hr = m_depthStencil.init(m_device, newW, newH, DXGI_FORMAT_D24_UNORM_S8_UINT, D3D11_BIND_DEPTH_STENCIL, 4, 0);
    if (FAILED(hr)) return;

    hr = m_depthStencilView.init(m_device, m_depthStencil, DXGI_FORMAT_D24_UNORM_S8_UINT);
    if (FAILED(hr)) return;

    // 8) Actualizar Viewport Principal
    m_viewport.init(m_window);

    // 9) Actualizar Ratio de Cámara
    m_camera.setLens(XM_PIDIV4, newW / (float)newH, 0.01f, 100.0f);
}

// ======================================================================================
// GESTIÓN DE REDIMENSIONADO DEL PANEL DE EDITOR (IMGUI VIEWPORT)
// ======================================================================================
void BaseApp::handleEditorViewportResize() {
    if (!m_editorViewportResizePending)
        return;

    // Desbindear para evitar conflictos (Resource in use)
    m_deviceContext.m_deviceContext->OMSetRenderTargets(0, nullptr, nullptr);

    ID3D11ShaderResourceView* nullSRVs[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT] = {};
    m_deviceContext.m_deviceContext->PSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, nullSRVs);

    // Crear un pase temporal con la nueva resolución
    EditorViewportPass newPass;
    HRESULT hr = newPass.init(m_device, m_pendingViewportWidth, m_pendingViewportHeight);
    if (FAILED(hr)) {
        // Si falla (ej. RAM de video llena), abortamos y conservamos el pass actual
        m_editorViewportResizePending = false;
        return;
    }

    // Intercambio seguro (El pass viejo se destruirá solo al salir de la función)
    m_editorViewportPass.swap(newPass);
    m_editorViewportResizePending = false;
}