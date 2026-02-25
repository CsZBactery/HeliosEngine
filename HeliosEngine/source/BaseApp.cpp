// ======================================================================================
// Archivo: BaseApp.cpp
// Implementación de la clase principal del motor. 
// Controla el Game Loop, inicialización de DirectX y renderizado.
// ======================================================================================

#include "BaseApp.h"
#include "ResourceManager.h"
#include <array>
#include <string>
#include "imgui.h"

// Estado del Rasterizador: Controla cómo se dibujan los polígonos (ej. si se ve el interior de los objetos)
ID3D11RasterizerState* g_pRasterizerStateNoCull = nullptr;

// ======================================================================================
// FASE 1: AWAKE (Preparación Lógica)
// ======================================================================================
HRESULT BaseApp::awake() {
    HRESULT hr = S_OK;

    // Inicialización de sistemas lógicos (sin hardware de GPU aún)
    m_sceneGraph.init();

    MESSAGE("Main", "Awake", "Application awake successfully.");
    return hr;
}

// ======================================================================================
// FASE 2: BUCLE PRINCIPAL (Game Loop)
// ======================================================================================
int BaseApp::run(HINSTANCE hInst, int nCmdShow) {
    // 1. Inicializar la ventana del sistema operativo Windows
    if (FAILED(m_window.init(hInst, nCmdShow, WndProc))) {
        ERROR("Main", "Run", "Failed to initialize window.");
        return 0;
    }

    // 2. Despertar los subsistemas lógicos del motor
    if (FAILED(awake())) {
        ERROR("Main", "Run", "Failed to awake application.");
        return 0;
    }

    // 3. Iniciar DirectX 11 y cargar recursos (Texturas, Modelos) 
    if (FAILED(init())) {
        ERROR("Main", "Run", "Failed to initialize device and device context.");
        return 0;
    }

    // 4. Inicializar la interfaz gráfica de usuario (ImGui)
    m_gui.init(m_window, m_device, m_deviceContext);

    // Preparación del temporizador de alta resolución para calcular el Delta Time
    MSG msg = {};
    LARGE_INTEGER freq, prev;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&prev);

    // Bucle infinito: Se ejecuta hasta que el usuario cierra la ventana
    while (WM_QUIT != msg.message) {
        // Procesar mensajes del SO (mouse, teclado, redimensionado)
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else {
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
// FASE 3: INICIALIZACIÓN DE HARDWARE (DirectX 11)
// ======================================================================================
HRESULT BaseApp::init() {
    HRESULT hr = S_OK;

    // 1. Crear el Dispositivo (conexión física con la GPU) y su Contexto (emisor de comandos)
    m_device.init();
    m_device.m_device->GetImmediateContext(&m_deviceContext.m_deviceContext);

    // 2. Crear SwapChain (Técnica de Doble Buffer para evitar parpadeos en pantalla) 
    hr = m_swapChain.init(m_device, m_deviceContext, m_backBuffer, m_window);
    if (FAILED(hr)) return hr;

    // 3. Crear el lienzo principal donde se escriben los colores (Render Target)
    hr = m_renderTargetView.init(m_device, m_backBuffer, DXGI_FORMAT_R8G8B8A8_UNORM);
    if (FAILED(hr)) return hr;

    // 4. Crear el Buffer de Profundidad (Z-Buffer) para ocultar objetos lejanos.
    // NOTA: Se usan 4 muestras y calidad 16 (MSAA) para coincidir con el SwapChain y evitar Crash.
    hr = m_depthStencil.init(m_device, m_window.m_width, m_window.m_height, DXGI_FORMAT_D24_UNORM_S8_UINT, D3D11_BIND_DEPTH_STENCIL, 4, 16);
    if (FAILED(hr)) return hr;

    hr = m_depthStencilView.init(m_device, m_depthStencil, DXGI_FORMAT_D24_UNORM_S8_UINT);
    if (FAILED(hr)) return hr;

    // 5. Configurar el área de proyección en la pantalla (Viewport)
    hr = m_viewport.init(m_window);
    if (FAILED(hr)) return hr;

    // 6. Configurar el Rasterizador (No Culling)
    // Permite renderizar tanto la cara frontal como la trasera de los polígonos.
    D3D11_RASTERIZER_DESC rasterDesc;
    ZeroMemory(&rasterDesc, sizeof(rasterDesc));
    rasterDesc.FillMode = D3D11_FILL_SOLID;
    rasterDesc.CullMode = D3D11_CULL_NONE; // Dibuja ambos lados
    rasterDesc.FrontCounterClockwise = false;
    rasterDesc.DepthClipEnable = true;
    rasterDesc.MultisampleEnable = true;
    m_device.m_device->CreateRasterizerState(&rasterDesc, &g_pRasterizerStateNoCull);
    m_deviceContext.m_deviceContext->RSSetState(g_pRasterizerStateNoCull);

    // 7. Cargar texturas del Entorno (Skybox)
    std::array<std::string, 6> faces = {
        "Skybox/cubemap_0.png", "Skybox/cubemap_1.png", "Skybox/cubemap_2.png",
        "Skybox/cubemap_3.png", "Skybox/cubemap_4.png", "Skybox/cubemap_5.png"
    };
    m_skyboxTex.CreateCubemap(m_device, m_deviceContext, faces, true);

    // 8. Crear y ensamblar el Actor Principal (Modelo de moto/Xbox)
    m_repsolActor = EU::MakeShared<Actor>(m_device);
    if (!m_repsolActor.isNull()) {
        m_model = new Model3D("Assets/Moto/repsol3.obj", ModelType::OBJ);

        std::vector<Texture> repsolTextures;
        hr = m_repsolTexture.init(m_device, "Assets/Textures/BaseColor", ExtensionType::PNG);

        if (FAILED(hr)) {
            ERROR("Main", "Init", "Failed to load texture BaseColor.png");
        }
        else {
            repsolTextures.push_back(m_repsolTexture);
        }

        m_repsolActor->setMesh(m_device, m_model->GetMeshes());
        m_repsolActor->setTextures(repsolTextures);
        m_repsolActor->setName("RepsolBike");

        // Ajustar posición inicial (Centrado, sin rotación y escalado x5)
        m_repsolActor->getComponent<Transform>()->setTransform(
            EU::Vector3(0.0f, -4.0f, 0.0f),
            EU::Vector3(0.0f, 0.0f, 0.0f),
            EU::Vector3(5.0f, 5.0f, 5.0f)
        );
        m_actors.push_back(m_repsolActor);
    }

    // Registrar en el Grafo de Escena
    for (auto& actor : m_actors) m_sceneGraph.addEntity(actor.get());

    // 9. Configurar cómo la CPU envía los vértices a la GPU (Input Layout)
    std::vector<D3D11_INPUT_ELEMENT_DESC> Layout;
    Layout.push_back({ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 });
    Layout.push_back({ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 });
    Layout.push_back({ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 });

    hr = m_shaderProgram.init(m_device, "Assets/Shaders/HeliosEngine.fx", Layout);
    if (FAILED(hr)) hr = m_shaderProgram.init(m_device, "HeliosEngine.fx", Layout);
    if (FAILED(hr)) return hr;

    // 10. Crear Buffers Constantes (Envío de variables Globales al Shader)
    m_cbNeverChanges.init(m_device, sizeof(CBNeverChanges));
    m_cbChangeOnResize.init(m_device, sizeof(CBChangeOnResize));

    // 11. Configurar Cámara y Luces
    m_camera.setLens(XM_PIDIV4, m_window.m_width / (float)m_window.m_height, 0.1f, 1000.0f);
    m_camera.setPosition(0.0f, 0.0f, -35.0f); // Alejar la cámara en el eje Z

    cbNeverChanges.mView = XMMatrixTranspose(m_camera.getView());
    cbNeverChanges.mLightDir = XMVectorSet(-0.577f, -0.577f, 0.577f, 1.0f);
    cbNeverChanges.mLightColor = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
    m_cbNeverChanges.update(m_deviceContext, nullptr, 0, nullptr, &cbNeverChanges, 0, 0);

    cbChangesOnResize.mProjection = XMMatrixTranspose(m_camera.getProj());
    m_cbChangeOnResize.update(m_deviceContext, nullptr, 0, nullptr, &cbChangesOnResize, 0, 0);

    return S_OK;
}

// ======================================================================================
// FASE 4: UPDATE (Lógica de cada Frame)
// ======================================================================================
void BaseApp::update(float deltaTime) {
    // Control de redimensionamiento de ventana (Ajusta la distorsión)
    RECT rc;
    GetClientRect(m_window.m_hWnd, &rc);
    float width = static_cast<float>(rc.right - rc.left);
    float height = static_cast<float>(rc.bottom - rc.top);

    if (height > 0) {
        m_camera.setLens(XM_PIDIV4, width / height, 0.1f, 1000.0f);
        cbChangesOnResize.mProjection = XMMatrixTranspose(m_camera.getProj());
    }

    // Actualización de la GUI (ImGui)
    m_gui.update(m_viewport, m_window);

    // Posicionamiento dinámico de ventanas ImGui ancladas a los bordes
    ImGui::SetNextWindowPos(ImVec2(width - 320.0f, 20.0f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(300.0f, 300.0f), ImGuiCond_FirstUseEver);
    m_gui.outliner(m_actors);

    if (!m_actors.empty() && m_gui.selectedActorIndex < m_actors.size()) {
        ImGui::SetNextWindowPos(ImVec2(width - 320.0f, 340.0f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(300.0f, 300.0f), ImGuiCond_FirstUseEver);
        m_gui.inspectorGeneral(m_actors[m_gui.selectedActorIndex]);
    }

    // Ventana de depuración del Cubemap
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

    // Actualizar Cámara y Luces en los Constant Buffers
    m_camera.updateViewMatrix();
    cbNeverChanges.mView = XMMatrixTranspose(m_camera.getView());
    cbNeverChanges.mLightDir = XMVectorSet(-0.577f, -0.577f, 0.577f, 1.0f);
    cbNeverChanges.mLightColor = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);

    m_cbNeverChanges.update(m_deviceContext, nullptr, 0, nullptr, &cbNeverChanges, 0, 0);
    m_cbChangeOnResize.update(m_deviceContext, nullptr, 0, nullptr, &cbChangesOnResize, 0, 0);

    // Actualizar la jerarquía de todos los actores
    m_sceneGraph.update(deltaTime, m_deviceContext);

    // Habilitar la edición visual (Gizmos) del objeto seleccionado
    if (!m_actors.empty() && m_gui.selectedActorIndex < m_actors.size()) {
        m_gui.editTransform(m_camera.getView(), m_camera.getProj(), m_actors[m_gui.selectedActorIndex]);
    }
}

// ======================================================================================
// FASE 5: RENDER (Dibujo en GPU)
// ======================================================================================
void BaseApp::render() {
    // 1. Limpiar el lienzo con un color sólido (Gris oscuro profesional)
    float ClearColor[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
    m_renderTargetView.render(m_deviceContext, m_depthStencilView, 1, ClearColor);

    // 2. Aplicar Viewport y limpiar el buffer de profundidad
    m_viewport.render(m_deviceContext);
    m_depthStencilView.render(m_deviceContext);

    // 3. Aplicar reglas de rasterización (mostrar las caras traseras)
    if (g_pRasterizerStateNoCull) {
        m_deviceContext.m_deviceContext->RSSetState(g_pRasterizerStateNoCull);
    }

    // 4. Activar los programas (Shaders) en la GPU
    m_shaderProgram.render(m_deviceContext);
    m_cbNeverChanges.render(m_deviceContext, 0, 1);
    m_cbChangeOnResize.render(m_deviceContext, 1, 1);

    // 5. Ordenar el dibujo de la geometría
    m_sceneGraph.render(m_deviceContext);

    // 6. Dibujar la Interfaz de Usuario (Siempre se dibuja al final para quedar encima)
    m_gui.render();

    // 7. Intercambiar los buffers (Mostrar el frame al usuario)
    m_swapChain.present();
}

// ======================================================================================
// FASE 6: DESTROY (Limpieza de Memoria)
// ======================================================================================
void BaseApp::destroy() {
    // Limpia el estado de la GPU
    if (m_deviceContext.m_deviceContext) m_deviceContext.m_deviceContext->ClearState();

    if (g_pRasterizerStateNoCull) { g_pRasterizerStateNoCull->Release(); g_pRasterizerStateNoCull = nullptr; }

    // Destruye en orden inverso de creación
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

// ======================================================================================
// PROCEDIMIENTO DE VENTANA (Captura de Eventos Windows)
// ======================================================================================
LRESULT BaseApp::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    // Interceptar inputs (clics, teclas) para la interfaz ImGui
    if (ImGui::GetCurrentContext() != nullptr) {
        if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam)) return true;
    }

    // Procesar eventos nativos de Windows
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
        PostQuitMessage(0); // Solicita cerrar la app rompiendo el bucle 'run'
        return 0;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}