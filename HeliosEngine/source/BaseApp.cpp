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

// Variable global (temporal) para el estado del rasterizador sin culling
ID3D11RasterizerState* g_pRasterizerStateNoCull = nullptr;

// ======================================================================================
// FASE 1: AWAKE (Preparación Lógica)
// ======================================================================================
HRESULT 
BaseApp::awake() {
    HRESULT hr = S_OK;

    // Inicialización de sistemas lógicos (sin hardware de GPU aún)
    m_sceneGraph.init();

    MESSAGE("Main", "Awake", "Application awake successfully.");
    return hr;
}

// ======================================================================================
// FASE 2: BUCLE PRINCIPAL (Game Loop)
// ======================================================================================
int 
BaseApp::run(HINSTANCE hInst, int nCmdShow) {
    // 1. Inicializar la ventana (OJO: Se pasa 'this' para que el WndProc pueda acceder a onResize)
    if (FAILED(m_window.init(hInst, nCmdShow, WndProc, this))) {
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

    // Preparación del temporizador de alta resolución
    MSG msg = {};
    LARGE_INTEGER freq, prev;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&prev);

    // Bucle infinito: Se ejecuta hasta que el usuario cierra la ventana
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

// ======================================================================================
// FASE 3: INICIALIZACIÓN DE HARDWARE (DirectX 11)
// ======================================================================================
HRESULT BaseApp::init() {
    HRESULT hr = S_OK;

    // 1. Crear SwapChain (Doble Buffer e Inicialización implícita de Device) 
    hr = m_swapChain.init(m_device, m_deviceContext, m_backBuffer, m_window);
    if (FAILED(hr)) return hr;

    // 2. Crear el lienzo principal (Render Target)
    hr = m_renderTargetView.init(m_device, m_backBuffer, DXGI_FORMAT_R8G8B8A8_UNORM);
    if (FAILED(hr)) return hr;

    // 3. Crear Buffer de Profundidad (Z-Buffer)
    hr = m_depthStencil.init(m_device, m_window.m_width, m_window.m_height, DXGI_FORMAT_D24_UNORM_S8_UINT, D3D11_BIND_DEPTH_STENCIL, 4, 16);
    if (FAILED(hr)) return hr;

    hr = m_depthStencilView.init(m_device, m_depthStencil, DXGI_FORMAT_D24_UNORM_S8_UINT);
    if (FAILED(hr)) return hr;

    // 4. Configurar Viewport Principal
    hr = m_viewport.init(m_window);
    if (FAILED(hr)) return hr;

    m_d3dReady = true;

    // =========================================================
    // CARGA DE RECURSOS 
    // =========================================================

    std::array<std::string, 6> faces = {
        "Assets/Skybox/cubemap_0.png",
        "Assets/Skybox/cubemap_1.png",
        "Assets/Skybox/cubemap_2.png",
        "Assets/Skybox/cubemap_3.png",
        "Assets/Skybox/cubemap_4.png",
        "Assets/Skybox/cubemap_5.png"
    };
    m_skyboxTex.CreateCubemap(m_device, m_deviceContext, faces, false);

    // Crear y ensamblar el Actor Principal (Moto Repsol)
    m_cyberGun = EU::MakeShared<Actor>(m_device);

    if (!m_cyberGun.isNull()) {
        // Cargar Modelo 
        m_model = new Model3D("Assets/Moto/repsol3.obj", ModelType::OBJ);
        std::vector<MeshComponent> meshes = m_model->GetMeshes();

        // Cargar Textura
        std::vector<Texture> textures;
        hr = m_AlbedoSRV.init(m_device, "Assets/Textures/BaseColor", ExtensionType::PNG);

        if (FAILED(hr)) {
            ERROR("Main", "InitDevice", "Failed to load texture BaseColor.png.");
            return hr;
        }
        textures.push_back(m_AlbedoSRV);

        // Asignar al Actor
        m_cyberGun->setMesh(m_device, meshes);
        m_cyberGun->setTextures(textures);
        m_cyberGun->setName("RepsolBike");
        m_actors.push_back(m_cyberGun);

        // Posición Inicial
        m_cyberGun->getComponent<Transform>()->setTransform(
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

    // =========================================================
    // CONFIGURACIÓN DE SHADERS 
    // =========================================================

    // Ahora le decimos a DirectX el tamaño exacto del vértice
    LayoutBuilder builder;
    builder.Add("POSITION", DXGI_FORMAT_R32G32B32_FLOAT)
        .Add("NORMAL", DXGI_FORMAT_R32G32B32_FLOAT)
        .Add("TANGENT", DXGI_FORMAT_R32G32B32_FLOAT)
        .Add("BITANGENT", DXGI_FORMAT_R32G32B32_FLOAT)
        .Add("TEXCOORD", DXGI_FORMAT_R32G32_FLOAT);

    hr = m_shaderProgram.init(m_device, "Assets/Shaders/HeliosEngine.fx", builder);
    if (FAILED(hr)) hr = m_shaderProgram.init(m_device, "HeliosEngine.fx", builder);

    if (FAILED(hr)) {
        ERROR("Main", "InitDevice", ("Failed to initialize ShaderProgram. HRESULT: " + std::to_string(hr)).c_str());
        return hr;
    }

    // ¡CORRECCIÓN! Usamos los nombres exactos de tus Buffers de HeliosEngine.fx
    hr = m_cbNeverChanges.init(m_device, sizeof(CBNeverChanges));
    if (FAILED(hr)) return hr;

    hr = m_cbChangeOnResize.init(m_device, sizeof(CBChangeOnResize));
    if (FAILED(hr)) return hr;

    // 10. Configurar Cámara
    m_camera.setLens(XM_PIDIV4, m_window.m_width / (float)m_window.m_height, 0.01f, 100.0f);
    m_camera.setPosition(0.0f, 3.0f, -6.0f);

    // Transponer la matriz para que HLSL la entienda correctamente
    cbNeverChanges.mView = XMMatrixTranspose(m_camera.getView());
    cbChangesOnResize.mProjection = XMMatrixTranspose(m_camera.getProj());

    // 11. Inicializar Skybox y Estados Base
    m_skybox.init(m_device, &m_deviceContext, m_skyboxTex);

    hr = m_defaultRasterizer.init(m_device, D3D11_FILL_SOLID, D3D11_CULL_NONE, false, true);
    if (FAILED(hr)) return hr;

    hr = m_defaultDepthStencil.init(m_device, true, D3D11_DEPTH_WRITE_MASK_ALL, D3D11_COMPARISON_LESS);
    if (FAILED(hr)) return hr;

    // 12. Inicializar el Pase de Editor (VITAL PARA EL VIEWPORT)
    hr = m_editorViewportPass.init(m_device, 1280, 720);
    if (FAILED(hr)) {
        ERROR("Main", "InitDevice", "Failed to initialize EditorViewportPass.");
        return hr;
    }

    return S_OK;
}

/// ======================================================================================
// FASE 4: UPDATE (Lógica de cada Frame)
// ======================================================================================
void 
BaseApp::update(float deltaTime) {

    // Actualización de la GUI (ImGui)
    m_gui.update(m_viewport, m_window);

    // Enviar textura del juego al panel Viewport de ImGui
    m_gui.drawViewportPanel(m_editorViewportPass.getSRV());

    // Paneles del Editor
    m_gui.outliner(m_actors);
    if (!m_actors.empty() && m_gui.selectedActorIndex < m_actors.size()) {
        m_gui.inspectorGeneral(m_actors[m_gui.selectedActorIndex]);
        m_gui.editTransform(m_camera, m_window, m_actors[m_gui.selectedActorIndex]);
    }

    // =========================================================
    // LÓGICA DE REDIMENSIONAMIENTO DEL EDITOR VIEWPORT
    // =========================================================
    unsigned int desiredW = static_cast<unsigned int>(m_gui.m_viewportSize.x);
    unsigned int desiredH = static_cast<unsigned int>(m_gui.m_viewportSize.y);
    const unsigned int kMinViewportSize = 64;

    if (desiredW < kMinViewportSize) desiredW = kMinViewportSize;
    if (desiredH < kMinViewportSize) desiredH = kMinViewportSize;

    if (desiredW != m_lastRequestedViewportWidth || desiredH != m_lastRequestedViewportHeight) {
        m_lastRequestedViewportWidth = desiredW;
        m_lastRequestedViewportHeight = desiredH;
        m_viewportResizeStableFrames = 0;
    }
    else {
        m_viewportResizeStableFrames++;
    }

    const int kStableFramesRequired = 2;
    if (m_viewportResizeStableFrames >= kStableFramesRequired) {
        if (desiredW != m_editorViewportPass.getWidth() || desiredH != m_editorViewportPass.getHeight()) {
            m_editorViewportResizePending = true;
            m_pendingViewportWidth = desiredW;
            m_pendingViewportHeight = desiredH;
        }
    }

    // =========================================================
    // ACTUALIZACIÓN DE MATRICES
    // =========================================================

    // Calcular nueva posición/rotación de la cámara
    m_camera.updateViewMatrix();

    // Actualizar matrices en RAM y transponerlas para DirectX
    cbNeverChanges.mView = XMMatrixTranspose(m_camera.getView());
    cbChangesOnResize.mProjection = XMMatrixTranspose(m_camera.getProj());

    // Enviar matrices actualizadas a la VRAM de la tarjeta de video
    m_cbNeverChanges.update(m_deviceContext, nullptr, 0, nullptr, &cbNeverChanges, 0, 0);
    m_cbChangeOnResize.update(m_deviceContext, nullptr, 0, nullptr, &cbChangesOnResize, 0, 0);

    // Actualizamos el Skybox
    m_skybox.update(m_deviceContext, m_camera);

    // Actualizar la jerarquía de todos los actores
    m_sceneGraph.update(deltaTime, m_deviceContext);
}

// ======================================================================================
// FASE 5: RENDER (Dibujo en GPU)
// ======================================================================================
void 
BaseApp::render() {

    // 0. Redimensionar el viewport del editor si el usuario arrastró la ventana
    handleEditorViewportResize();

    // =========================================================
    // PASE 1: DIBUJAR LA ESCENA EN LA TEXTURA DEL EDITOR
    // =========================================================
    float clearColor[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
    const float viewportClear[4] = { 0.10f, 0.10f, 0.10f, 1.0f };

    m_editorViewportPass.begin(m_deviceContext, viewportClear);
    m_editorViewportPass.setViewport(m_deviceContext);
    m_editorViewportPass.clearDepth(m_deviceContext);

    // A) Dibujar el Fondo (Skybox)
    m_skybox.render(m_deviceContext);

    // B) Restaurar estados para los modelos
    m_defaultRasterizer.render(m_deviceContext);
    m_defaultDepthStencil.render(m_deviceContext, 0, false);

    ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
    m_deviceContext.m_deviceContext->PSSetShaderResources(10, 1, nullSRV);
    m_deviceContext.m_deviceContext->PSSetShaderResources(0, 1, nullSRV);

    m_shaderProgram.render(m_deviceContext);

    // =========================================================
    // ¡CORRECCIÓN AQUÍ! (Usar tu variable original)
    // =========================================================
    m_cbNeverChanges.render(m_deviceContext, 0, 1);
    m_cbChangeOnResize.render(m_deviceContext, 1, 1);

    // C) Dibujar la Escena (El Xbox)
    m_sceneGraph.render(m_deviceContext);

    // =========================================================
    // PASE 2: DIBUJAR IMGUI EN EL BACKBUFFER DE LA PANTALLA
    // =========================================================
    m_renderTargetView.render(m_deviceContext, m_depthStencilView, 1, clearColor);
    m_viewport.render(m_deviceContext);
    m_depthStencilView.render(m_deviceContext);

    // Dibujar la interfaz de ImGui
    m_gui.render();

    // 5. Intercambiar los buffers
    m_swapChain.present();
}

// ======================================================================================
// FASE 6: DESTROY (Limpieza de Memoria)
// ======================================================================================
void 
BaseApp::destroy() {
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

    m_editorViewportPass.destroy();
    m_AlbedoSRV.destroy();

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
    case WM_SIZE: {
        if (wParam == SIZE_MINIMIZED) return 0;
        UINT newW = LOWORD(lParam);
        UINT newH = HIWORD(lParam);
        if (newW == 0 || newH == 0) return 0;

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
// REDIMENSIONAMIENTO WINDOWS
// ======================================================================================
void 
BaseApp::onResize(UINT newW, UINT newH) {
    if (!m_d3dReady) {
        m_window.m_width = (int)newW;
        m_window.m_height = (int)newH;
        return;
    }

    if (!m_deviceContext.m_deviceContext || !m_swapChain.m_swapChain) return;
    if (newW == 0 || newH == 0) return;

    m_window.m_width = (int)newW;
    m_window.m_height = (int)newH;

    ID3D11RenderTargetView* nullRTV = nullptr;
    m_deviceContext.m_deviceContext->OMSetRenderTargets(1, &nullRTV, nullptr);

    m_renderTargetView.destroy();
    m_depthStencilView.destroy();
    m_depthStencil.destroy();
    m_backBuffer.destroy();

    HRESULT hr = m_swapChain.resizeBuffers(newW, newH);
    if (FAILED(hr)) return;

    hr = m_swapChain.getBackBuffer(m_backBuffer);
    if (FAILED(hr)) return;

    hr = m_renderTargetView.init(m_device, m_backBuffer, DXGI_FORMAT_R8G8B8A8_UNORM);
    if (FAILED(hr)) return;

    hr = m_depthStencil.init(m_device, newW, newH, DXGI_FORMAT_D24_UNORM_S8_UINT, D3D11_BIND_DEPTH_STENCIL, 4, 0);
    if (FAILED(hr)) return;

    hr = m_depthStencilView.init(m_device, m_depthStencil, DXGI_FORMAT_D24_UNORM_S8_UINT);
    if (FAILED(hr)) return;

    m_viewport.init(m_window);
    m_camera.setLens(XM_PIDIV4, newW / (float)newH, 0.01f, 100.0f);
}

// ======================================================================================
// REDIMENSIONAMIENTO EDITOR IMGUI
// ======================================================================================
void 
BaseApp::handleEditorViewportResize() {
    if (!m_editorViewportResizePending) return;

    m_deviceContext.m_deviceContext->OMSetRenderTargets(0, nullptr, nullptr);

    ID3D11ShaderResourceView* nullSRVs[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT] = {};
    m_deviceContext.m_deviceContext->PSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, nullSRVs);

    EditorViewportPass newPass;
    HRESULT hr = newPass.init(m_device, m_pendingViewportWidth, m_pendingViewportHeight);
    if (FAILED(hr)) {
        m_editorViewportResizePending = false;
        return;
    }

    m_editorViewportPass.swap(newPass);
    m_editorViewportResizePending = false;
}