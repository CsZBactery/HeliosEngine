// ======================================================================================
// Archivo: BaseApp.cpp
// Implementación de la clase principal del motor. 
// Controla el Game Loop, inicialización de DirectX y renderizado general.
// ======================================================================================

#include "BaseApp.h"
#include "ResourceManager.h"
#include <array>
#include <string>
#include "imgui.h"

// Variable global (temporal) para el estado del rasterizador sin culling (si aún la necesitas para algo externo)
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
    // Inicialización implícita de device y deviceContext
    // m_device.init(); ya se hace en la creación de las variables internamente en algunos diseños,
    // o se confía en que las funciones siguientes lo rellenen. El profe no llama a init() explícito en m_device.

    // 2. Crear SwapChain (Doble Buffer) 
    hr = m_swapChain.init(m_device, m_deviceContext, m_backBuffer, m_window);
    if (FAILED(hr)) {
        ERROR("Main", "InitDevice", ("Failed to initialize SwapChain. HRESULT: " + std::to_string(hr)).c_str());
        return hr;
    }

    // 3. Crear el lienzo principal (Render Target)
    hr = m_renderTargetView.init(m_device, m_backBuffer, DXGI_FORMAT_R8G8B8A8_UNORM);
    if (FAILED(hr)) {
        ERROR("Main", "InitDevice", ("Failed to initialize RenderTargetView. HRESULT: " + std::to_string(hr)).c_str());
        return hr;
    }

    // 4. Crear Buffer de Profundidad (Z-Buffer)
    // CORRECCIÓN: Emparejamos el Multisampling a 4 muestras (mc:4) y Calidad 16 (mq:16)
    // para que coincida exactamente con lo que generó el SwapChain.
    hr = m_depthStencil.init(m_device, m_window.m_width, m_window.m_height, DXGI_FORMAT_D24_UNORM_S8_UINT, D3D11_BIND_DEPTH_STENCIL, 4, 16);

    if (FAILED(hr)) {
        ERROR("Main", "InitDevice", ("Failed to initialize DepthStencil. HRESULT: " + std::to_string(hr)).c_str());
        return hr;
    }

    hr = m_depthStencilView.init(m_device, m_depthStencil, DXGI_FORMAT_D24_UNORM_S8_UINT);
    if (FAILED(hr)) {
        ERROR("Main", "InitDevice", ("Failed to initialize DepthStencilView. HRESULT: " + std::to_string(hr)).c_str());
        return hr;
    }

    // 5. Configurar Viewport
    hr = m_viewport.init(m_window);
    if (FAILED(hr)) {
        ERROR("Main", "InitDevice", ("Failed to initialize Viewport. HRESULT: " + std::to_string(hr)).c_str());
        return hr;
    }

    // 6. Cargar texturas del Entorno (Skybox)
    std::array<std::string, 6> faces = {
        "Assets/Skybox/cubemap_0.png",
        "Assets/Skybox/cubemap_1.png",
        "Assets/Skybox/cubemap_2.png",
        "Assets/Skybox/cubemap_3.png",
        "Assets/Skybox/cubemap_4.png",
        "Assets/Skybox/cubemap_5.png"
    };
    m_skyboxTex.CreateCubemap(m_device, m_deviceContext, faces, true);

    // 7. Crear y ensamblar el Actor Principal (Ej. CyberGun / Moto)
    m_repsolActor = EU::MakeShared<Actor>(m_device);

    if (!m_repsolActor.isNull()) {
        // Cargar Modelo 
        m_model = new Model3D("Assets/Moto/repsol3.obj", ModelType::OBJ);
        std::vector<MeshComponent> meshes = m_model->GetMeshes();

        // Cargar Textura
        std::vector<Texture> textures;
        hr = m_repsolTexture.init(m_device, "Assets/Textures/BaseColor", ExtensionType::PNG);
        if (FAILED(hr)) {
            ERROR("Main", "InitDevice", ("Failed to load texture BaseColor.png. HRESULT: " + std::to_string(hr)).c_str());
            return hr;
        }
        textures.push_back(m_repsolTexture);

        // Asignar al Actor
        m_repsolActor->setMesh(m_device, meshes);
        m_repsolActor->setTextures(textures);
        m_repsolActor->setName("RepsolBike");
        m_actors.push_back(m_repsolActor);

        // Posición Inicial
        m_repsolActor->getComponent<Transform>()->setTransform(
            EU::Vector3(0.0f, -4.0f, 0.0f),
            EU::Vector3(0.0f, 0.0f, 0.0f),
            EU::Vector3(5.0f, 5.0f, 5.0f)
        );
    }
    else {
        ERROR("Main", "InitDevice", "Failed to create main Actor.");
        return E_FAIL;
    }

    // Registrar actores en el Grafo de Escena
    for (auto& actor : m_actors) {
        m_sceneGraph.addEntity(actor.get());
    }

    // 8. Configurar Input Layout (Cómo la CPU envía vértices a la GPU)
    std::vector<D3D11_INPUT_ELEMENT_DESC> Layout;
    Layout.push_back({ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 });
    Layout.push_back({ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 });

    // DESCOMENTADO: Tu shader original HeliosEngine.fx SÍ necesita las Normales.
    Layout.push_back({ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 });

    // 💥 CORRECCIÓN: Buscamos tu shader HeliosEngine, no el Wildvine del profe 💥
    hr = m_shaderProgram.init(m_device, "Assets/Shaders/HeliosEngine.fx", Layout);
    if (FAILED(hr)) hr = m_shaderProgram.init(m_device, "HeliosEngine.fx", Layout); // Fallback

    if (FAILED(hr)) {
        ERROR("Main", "InitDevice", ("Failed to initialize ShaderProgram. HRESULT: " + std::to_string(hr)).c_str());
        return hr;
    }

    // 9. Crear Buffers Constantes
    hr = m_cbNeverChanges.init(m_device, sizeof(CBNeverChanges));
    if (FAILED(hr)) return hr;

    hr = m_cbChangeOnResize.init(m_device, sizeof(CBChangeOnResize));
    if (FAILED(hr)) return hr;

    // 10. Configurar Cámara
    m_camera.setLens(XM_PIDIV4, m_window.m_width / (float)m_window.m_height, 0.01f, 100.0f);
    m_camera.setPosition(0.0f, 3.0f, -6.0f);

    cbNeverChanges.mView = XMMatrixTranspose(m_camera.getView());
    cbChangesOnResize.mProjection = XMMatrixTranspose(m_camera.getProj());

    // 11. Inicializar el Skybox y los Estados Base (ESTO ES CRUCIAL PARA VER EL CIELO BIEN)
    m_skybox.init(m_device, &m_deviceContext, m_skyboxTex);

    hr = m_defaultRasterizer.init(m_device, D3D11_FILL_SOLID, D3D11_CULL_BACK, false, true);
    if (FAILED(hr)) {
        ERROR("Main", "InitDevice", "Failed to initialize default Rasterizer.");
        return hr;
    }

    hr = m_defaultDepthStencil.init(m_device, true, D3D11_DEPTH_WRITE_MASK_ALL, D3D11_COMPARISON_LESS);
    if (FAILED(hr)) {
        ERROR("Main", "InitDevice", "Failed to initialize default DepthStencilState.");
        return hr;
    }

    return S_OK;
}

// ======================================================================================
// FASE 4: UPDATE (Lógica de cada Frame)
// ======================================================================================
void BaseApp::update(float deltaTime) {
    // Control de redimensionamiento de ventana
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

    m_gui.outliner(m_actors);
    if (!m_actors.empty() && m_gui.selectedActorIndex < m_actors.size()) {
        m_gui.inspectorGeneral(m_actors[m_gui.selectedActorIndex]);
    }

    // Ventana de depuración del Cubemap (Opcional, si quieres ver las caras desglosadas)
    static ID3D11ShaderResourceView* faceSRV[6] = { nullptr };
    if (!faceSRV[0] && m_skyboxTex.m_texture) {
        for (UINT i = 0; i < 6; ++i) {
            faceSRV[i] = m_skyboxTex.CreateCubemapFaceSRV(m_device.m_device, m_skyboxTex.m_texture, DXGI_FORMAT_R8G8B8A8_UNORM, i, 1);
        }
    }
    ImGui::Begin("Cubemap");
    ImGui::Text("Skybox Preview");
    if (faceSRV[0]) ImGui::Image((ImTextureID)faceSRV[0], ImVec2(200, 200));
    else ImGui::Text("Textura no disponible");
    ImGui::End();

    // Actualizar Matriz de Vista en el Constant Buffer
    m_camera.updateViewMatrix();
    cbNeverChanges.mView = XMMatrixTranspose(m_camera.getView());

    // NOTA: El shader del profe parece no usar LightDir ni LightColor en cbNeverChanges
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
    // 1. Limpiar el lienzo con un color sólido
    float ClearColor[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
    m_renderTargetView.render(m_deviceContext, m_depthStencilView, 1, ClearColor);

    // 2. Aplicar Viewport y limpiar el buffer de profundidad
    m_viewport.render(m_deviceContext);
    m_depthStencilView.render(m_deviceContext);

    // ---------------------------------------------------------
    // PASO 1: DIBUJAR EL SKYBOX (Fondo del mundo)
    // ---------------------------------------------------------
    m_skybox.render(m_deviceContext, m_camera);

    // ---------------------------------------------------------
    // PASO 2: RESTAURAR ESTADOS + PREPARAR PIPELINE PARA MODELOS
    // ---------------------------------------------------------
    m_defaultRasterizer.render(m_deviceContext);
    m_defaultDepthStencil.render(m_deviceContext, 0, false);

    // Limpia SRVs por seguridad (Evita que el cubemap interfiera con las texturas 2D normales)
    ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
    m_deviceContext.m_deviceContext->PSSetShaderResources(10, 1, nullSRV);
    m_deviceContext.m_deviceContext->PSSetShaderResources(0, 1, nullSRV);

    // Re-bindea shader principal y layout de la escena
    m_shaderProgram.render(m_deviceContext);
    m_deviceContext.IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Mandar matrices globales (Vista / Proyección)
    m_cbNeverChanges.render(m_deviceContext, 0, 1);
    m_cbChangeOnResize.render(m_deviceContext, 1, 1);

    // ---------------------------------------------------------
    // PASO 3: DIBUJAR LA ESCENA (Modelos 3D)
    // ---------------------------------------------------------
    m_sceneGraph.render(m_deviceContext);

    // ---------------------------------------------------------
    // PASO 4: DIBUJAR LA INTERFAZ GRÁFICA (ImGui)
    // ---------------------------------------------------------
    m_gui.render();

    // 5. Intercambiar los buffers (Mostrar en pantalla)
    m_swapChain.present();
}

// ======================================================================================
// FASE 6: DESTROY (Limpieza de Memoria)
// ======================================================================================
void BaseApp::destroy() {
    if (m_deviceContext.m_deviceContext) m_deviceContext.m_deviceContext->ClearState();

    if (g_pRasterizerStateNoCull) { g_pRasterizerStateNoCull->Release(); g_pRasterizerStateNoCull = nullptr; }

    m_skybox.destroy();
    m_defaultRasterizer.destroy();
    m_defaultDepthStencil.destroy();

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

    if (m_model) { delete m_model; m_model = nullptr; }

    m_deviceContext.destroy();
    m_device.destroy();
}

// ======================================================================================
// PROCEDIMIENTO DE VENTANA (Captura de Eventos Windows)
// ======================================================================================
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