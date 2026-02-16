#include "BaseApp.h"
#include "ResourceManager.h"
#include <array>
#include <string>
#include "imgui.h"

// Estado del Rasterizador: Controla cómo se dibujan los polígonos (ej. si se ve el interior de los objetos)
ID3D11RasterizerState* g_pRasterizerStateNoCull = nullptr;

// Fase inicial lógica: Se ejecuta antes de que el motor gráfico esté listo
HRESULT BaseApp::awake() {
    HRESULT hr = S_OK;
    // Preparamos el grafo de escena para recibir objetos
    m_sceneGraph.init();
    MESSAGE("Main", "Awake", "Application awake successfully.");
    return hr;
}

// El corazón de la aplicación: Controla el flujo entre Windows y el Motor
int BaseApp::run(HINSTANCE hInst, int nCmdShow) {
    // 1. Creación de la ventana del sistema operativo
    if (FAILED(m_window.init(hInst, nCmdShow, WndProc))) {
        ERROR("Main", "Run", "Failed to initialize window.");
        return 0;
    }
    // 2. Preparación lógica
    if (FAILED(awake())) {
        ERROR("Main", "Run", "Failed to awake application.");
        return 0;
    }
    // 3. Inicialización de DirectX y carga de Assets
    if (FAILED(init())) {
        ERROR("Main", "Run", "Failed to initialize device and device context.");
        return 0;
    }
    // 4. Encendido de la interfaz de usuario
    m_gui.init(m_window, m_device, m_deviceContext);

    // Configuración del temporizador de alta precisión
    MSG msg = {};
    LARGE_INTEGER freq, prev;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&prev);

    // Bucle principal: Se ejecuta hasta que se cierre la ventana
    while (WM_QUIT != msg.message) {
        // Revisamos si Windows tiene mensajes (mouse, teclado, etc.)
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else {
            // Si no hay mensajes, procesamos un frame del motor
            LARGE_INTEGER curr;
            QueryPerformanceCounter(&curr);
            // Calculamos el tiempo transcurrido (DeltaTime) para movimientos fluidos
            float deltaTime = static_cast<float>(curr.QuadPart - prev.QuadPart) / freq.QuadPart;
            prev = curr;

            update(deltaTime); // Lógica
            render();         // Dibujo
        }
    }
    return (int)msg.wParam;
}

// Configuración profunda de recursos gráficos
HRESULT BaseApp::init() {
    HRESULT hr = S_OK;

    // Conexión con la tarjeta de video (GPU)
    m_device.init();
    m_device.m_device->GetImmediateContext(&m_deviceContext.m_deviceContext);

    // SwapChain: Permite el doble buffer (dibujar en uno oculto y luego mostrarlo)
    hr = m_swapChain.init(m_device, m_deviceContext, m_backBuffer, m_window);
    if (FAILED(hr)) return hr;

    // RenderTarget: Define el "lienzo" donde los shaders escribirán colores
    hr = m_renderTargetView.init(m_device, m_backBuffer, DXGI_FORMAT_R8G8B8A8_UNORM);
    if (FAILED(hr)) return hr;

    // DepthStencil: El "Z-Buffer", evita que objetos lejanos se dibujen encima de los cercanos
    hr = m_depthStencil.init(m_device, m_window.m_width, m_window.m_height,
        DXGI_FORMAT_D24_UNORM_S8_UINT, D3D11_BIND_DEPTH_STENCIL, 4, 16);
    if (FAILED(hr)) return hr;

    hr = m_depthStencilView.init(m_device, m_depthStencil, DXGI_FORMAT_D24_UNORM_S8_UINT);
    if (FAILED(hr)) return hr;

    // Viewport: Define en qué parte de la ventana dibujaremos (toda el área disponible)
    hr = m_viewport.init(m_window);
    if (FAILED(hr)) return hr;

    // Configuración del Rasterizador para no descartar caras (CULL_NONE)
    // Esto permite ver el interior de los modelos y el cielo correctamente
    D3D11_RASTERIZER_DESC rasterDesc;
    ZeroMemory(&rasterDesc, sizeof(rasterDesc));
    rasterDesc.FillMode = D3D11_FILL_SOLID;
    rasterDesc.CullMode = D3D11_CULL_NONE;
    rasterDesc.FrontCounterClockwise = false;
    rasterDesc.DepthClipEnable = true;
    rasterDesc.MultisampleEnable = true;
    m_device.m_device->CreateRasterizerState(&rasterDesc, &g_pRasterizerStateNoCull);
    m_deviceContext.m_deviceContext->RSSetState(g_pRasterizerStateNoCull);

    // Carga de las 6 texturas para el Cubemap (Skybox/Cielo)
    std::array<std::string, 6> faces = {
        "Skybox/cubemap_0.png", "Skybox/cubemap_1.png", "Skybox/cubemap_2.png",
        "Skybox/cubemap_3.png", "Skybox/cubemap_4.png", "Skybox/cubemap_5.png"
    };
    m_skyboxTex.CreateCubemap(m_device, m_deviceContext, faces, true);

    // Inicialización del Actor (Modelo 3D)
    m_repsolActor = EU::MakeShared<Actor>(m_device);
    if (!m_repsolActor.isNull()) {
        m_model = new Model3D("Assets/Moto/repsol3.obj", ModelType::OBJ);

        // Cargamos la textura base del modelo
        std::vector<Texture> repsolTextures;
        hr = m_repsolTexture.init(m_device, "Assets/Textures/BaseColor", ExtensionType::PNG);

        if (FAILED(hr)) {
            ERROR("Main", "Init", "Failed to load texture BaseColor.png");
        }
        else {
            repsolTextures.push_back(m_repsolTexture);
        }

        // Armamos el actor con su malla y textura
        m_repsolActor->setMesh(m_device, m_model->GetMeshes());
        m_repsolActor->setTextures(repsolTextures);
        m_repsolActor->setName("RepsolBike");

        // Ajustamos su lugar en el mundo
        m_repsolActor->getComponent<Transform>()->setTransform(
            EU::Vector3(0.0f, -4.0f, 0.0f),       // Un poco abajo del centro
            EU::Vector3(0.0f, 0.0f, 0.0f),        // Sin rotación inicial
            EU::Vector3(5.0f, 5.0f, 5.0f)         // Aumentamos su tamaño 5 veces
        );
        m_actors.push_back(m_repsolActor);
    }

    // Registramos todos los actores en el grafo de escena para que se actualicen automáticamente
    for (auto& actor : m_actors) m_sceneGraph.addEntity(actor.get());

    // Input Layout: Mapea cómo los datos de los vértices (Pos, UV, Normal) entran al Shader
    std::vector<D3D11_INPUT_ELEMENT_DESC> Layout;
    D3D11_INPUT_ELEMENT_DESC posDesc = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 };
    Layout.push_back(posDesc);
    D3D11_INPUT_ELEMENT_DESC texDesc = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 };
    Layout.push_back(texDesc);
    D3D11_INPUT_ELEMENT_DESC normDesc = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 };
    Layout.push_back(normDesc);

    // Carga y compilación de los Shaders principales
    hr = m_shaderProgram.init(m_device, "Assets/Shaders/HeliosEngine.fx", Layout);
    if (FAILED(hr)) hr = m_shaderProgram.init(m_device, "HeliosEngine.fx", Layout);
    if (FAILED(hr)) return hr;

    // Preparamos los buffers de memoria constante en la GPU
    m_cbNeverChanges.init(m_device, sizeof(CBNeverChanges));
    m_cbChangeOnResize.init(m_device, sizeof(CBChangeOnResize));

    // Configuración de la lente de la cámara (FOV de 45 grados y rango de visión)
    m_camera.setLens(XM_PIDIV4, m_window.m_width / (float)m_window.m_height, 0.1f, 1000.0f);
    m_camera.setPosition(0.0f, 0.0f, -35.0f); // Nos alejamos para ver el objeto completo

    // Configuración inicial de luces
    cbNeverChanges.mView = XMMatrixTranspose(m_camera.getView());
    cbNeverChanges.mLightDir = XMVectorSet(-0.577f, -0.577f, 0.577f, 1.0f); // Luz diagonal
    cbNeverChanges.mLightColor = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);        // Luz blanca
    m_cbNeverChanges.update(m_deviceContext, nullptr, 0, nullptr, &cbNeverChanges, 0, 0);

    // Actualizamos la matriz de proyección inicial
    cbChangesOnResize.mProjection = XMMatrixTranspose(m_camera.getProj());
    m_cbChangeOnResize.update(m_deviceContext, nullptr, 0, nullptr, &cbChangesOnResize, 0, 0);

    return S_OK;
}

// Lógica que se repite en cada frame
void BaseApp::update(float deltaTime) {
    // 1. Manejo del tiempo interno
    static float t = 0.0f;
    static DWORD dwTimeStart = 0;
    DWORD dwTimeCur = GetTickCount();
    if (dwTimeStart == 0) dwTimeStart = dwTimeCur;
    t = (dwTimeCur - dwTimeStart) / 1000.0f;

    // 2. Ajuste Dinámico: Corrige el estiramiento si el usuario cambia el tamaño de la ventana
    RECT rc;
    GetClientRect(m_window.m_hWnd, &rc);
    float width = static_cast<float>(rc.right - rc.left);
    float height = static_cast<float>(rc.bottom - rc.top);

    if (height > 0) {
        m_camera.setLens(XM_PIDIV4, width / height, 0.1f, 1000.0f);
        cbChangesOnResize.mProjection = XMMatrixTranspose(m_camera.getProj());
    }

    // 3. Interfaz de Usuario: Actualizamos posiciones de las ventanas de ImGui
    m_gui.update(m_viewport, m_window);

    // Ventana de Jerarquía (Lado derecho superior)
    ImGui::SetNextWindowPos(ImVec2(width - 320.0f, 20.0f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(300.0f, 300.0f), ImGuiCond_FirstUseEver);
    m_gui.outliner(m_actors);

    // Ventana de Inspector (Lado derecho inferior)
    if (!m_actors.empty() && m_gui.selectedActorIndex < m_actors.size()) {
        ImGui::SetNextWindowPos(ImVec2(width - 320.0f, 340.0f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(300.0f, 300.0f), ImGuiCond_FirstUseEver);
        m_gui.inspectorGeneral(m_actors[m_gui.selectedActorIndex]);
    }

    // Ventana de Skybox: Generamos vistas previas 2D de las caras del cubo
    static ID3D11ShaderResourceView* faceSRV[6] = { nullptr };
    if (!faceSRV[0]) {
        for (UINT i = 0; i < 6; ++i) {
            faceSRV[i] = m_skyboxTex.CreateCubemapFaceSRV(m_device.m_device, m_skyboxTex.m_texture, DXGI_FORMAT_R8G8B8A8_UNORM, i, 1);
        }
    }

    ImGui::SetNextWindowPos(ImVec2(20.0f, 20.0f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(220.0f, 260.0f), ImGuiCond_FirstUseEver);

    ImGui::Begin("Cubemap");
    ImGui::Text("Skybox Preview");
    if (faceSRV[0]) ImGui::Image((ImTextureID)faceSRV[0], ImVec2(200, 200));
    else ImGui::Text("Textura no disponible");
    ImGui::End();

    // 4. Actualización de Matrices y Luces en GPU
    m_camera.updateViewMatrix();
    cbNeverChanges.mView = XMMatrixTranspose(m_camera.getView());
    cbNeverChanges.mLightDir = XMVectorSet(-0.577f, -0.577f, 0.577f, 1.0f);
    cbNeverChanges.mLightColor = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);

    m_cbNeverChanges.update(m_deviceContext, nullptr, 0, nullptr, &cbNeverChanges, 0, 0);
    m_cbChangeOnResize.update(m_deviceContext, nullptr, 0, nullptr, &cbChangesOnResize, 0, 0);

    // 5. Actualización de la jerarquía de todos los objetos en escena
    m_sceneGraph.update(deltaTime, m_deviceContext);

    // Gizmos: Permite mover objetos con flechas de colores en pantalla
    if (!m_actors.empty() && m_gui.selectedActorIndex < m_actors.size()) {
        m_gui.editTransform(m_camera.getView(), m_camera.getProj(), m_actors[m_gui.selectedActorIndex]);
    }
}

// Proceso de dibujo final
void BaseApp::render() {
    // 1. Limpiamos la pantalla con un color gris muy oscuro (0.1f)
    float ClearColor[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
    m_renderTargetView.render(m_deviceContext, m_depthStencilView, 1, ClearColor);

    // 2. Preparamos el área de dibujo y el buffer de profundidad
    m_viewport.render(m_deviceContext);
    m_depthStencilView.render(m_deviceContext);

    // 3. Aplicamos el estado del rasterizador (No Culling)
    if (g_pRasterizerStateNoCull) {
        m_deviceContext.m_deviceContext->RSSetState(g_pRasterizerStateNoCull);
    }

    // 4. Activamos Shaders y Buffers de datos
    m_shaderProgram.render(m_deviceContext);
    m_cbNeverChanges.render(m_deviceContext, 0, 1);
    m_cbChangeOnResize.render(m_deviceContext, 1, 1);

    // 5. Dibujamos todos los objetos registrados en el Grafo de Escena
    m_sceneGraph.render(m_deviceContext);

    // 6. Dibujamos la interfaz de usuario encima de todo
    m_gui.render();

    // 7. Mostramos el resultado final en la pantalla del usuario
    m_swapChain.present();
}

// Limpieza total al cerrar el programa
void BaseApp::destroy() {
    if (m_deviceContext.m_deviceContext) m_deviceContext.m_deviceContext->ClearState();

    // Liberamos recursos de DirectX para evitar fugas de memoria
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

// Gestor de mensajes de Windows: Envía los inputs a ImGui o los procesa internamente
LRESULT BaseApp::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    if (ImGui::GetCurrentContext() != nullptr) {
        if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam)) return true;
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
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}