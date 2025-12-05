#include "../include/BaseApp.h"
#include "../include/ResourceManager.h"
#include <direct.h> 

// Necesario para que ImGui capture inputs de mouse y teclado
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

BaseApp::BaseApp(HINSTANCE hInst, int nCmdShow) {}

int BaseApp::run(HINSTANCE hInst, int nCmdShow) {

    // Aseguramos formato estandar de C para evitar problemas con puntos/comas en floats
    setlocale(LC_ALL, "C");

    // Intentamos levantar la ventana y el motor. Si algo falla, cerramos.
    if (FAILED(m_window.init(hInst, nCmdShow, WndProc))) return 0;
    if (FAILED(init())) return 0;

    MSG msg = {};

    // Configuracion del Timer de alta resolucion para el DeltaTime
    LARGE_INTEGER freq, prev;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&prev);

    // --- GAME LOOP ---
    while (WM_QUIT != msg.message) {
        // Si hay mensajes de Windows (input, resize, close), los procesamos
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else {
            // Si no hay mensajes, renderizamos el frame
            LARGE_INTEGER curr;
            QueryPerformanceCounter(&curr);

            // Calcular tiempo entre frames (en segundos)
            float deltaTime = static_cast<float>(curr.QuadPart - prev.QuadPart) / freq.QuadPart;
            prev = curr;

            update(deltaTime);
            render();
        }
    }

    // Al salir del loop, limpiamos todo
    destroy();
    return (int)msg.wParam;
}

HRESULT BaseApp::init() {
    HRESULT hr = S_OK;

    // --------------------------------------------------------
    // 1. INICIALIZACION DE DIRECTX (DEVICE & CONTEXT)
    // --------------------------------------------------------
    m_device.init();
    if (!m_device.m_device) {
        ERROR("BaseApp", "init", "Critical: Device not initialized.");
        return E_FAIL;
    }
    // Obtenemos el contexto inmediato para dibujar
    m_device.m_device->GetImmediateContext(&m_deviceContext.m_deviceContext);

    // --------------------------------------------------------
    // 2. CONFIGURACION DE LA SWAPCHAIN Y VISTAS
    // --------------------------------------------------------
    hr = m_swapChain.init(m_device, m_deviceContext, m_backBuffer, m_window);
    if (FAILED(hr)) return hr;

    // Vista para dibujar en el BackBuffer
    hr = m_renderTargetView.init(m_device, m_backBuffer, DXGI_FORMAT_R8G8B8A8_UNORM);
    if (FAILED(hr)) return hr;

    // 3. CONFIGURACION DEL DEPTH STENCIL (Z-BUFFER)
    // Usamos MSAA x4 (Calidad 16) para bordes mas suaves
    hr = m_depthStencil.init(m_device, m_window.m_width, m_window.m_height,
        DXGI_FORMAT_D24_UNORM_S8_UINT, D3D11_BIND_DEPTH_STENCIL, 4, 16);
    if (FAILED(hr)) return hr;

    hr = m_depthStencilView.init(m_device, m_depthStencil, DXGI_FORMAT_D24_UNORM_S8_UINT);
    if (FAILED(hr)) return hr;

    // Configurar el Viewport para que cubra toda la ventana
    hr = m_viewport.init(m_window);
    if (FAILED(hr)) return hr;

    // --------------------------------------------------------
    // 4. CARGA DE RECURSOS (MODELOS Y TEXTURAS)
    // --------------------------------------------------------
    m_repsolActor = EU::MakeShared<Actor>(m_device);

    if (!m_repsolActor.isNull()) {
        // Cargar Geometria (.obj)
        m_model = new Model3D("Assets/Moto/repsol3.obj", ModelType::OBJ);
        std::vector<MeshComponent> myMeshes = m_model->GetMeshes();

        // Cargar Textura
        std::vector<Texture> myTextures;
        // OJO: La ruta es relativa al ejecutable o al working directory
        hr = m_repsolTexture.init(m_device, "Assets/Textures/BaseColor", ExtensionType::PNG);

        if (SUCCEEDED(hr)) {
            myTextures.push_back(m_repsolTexture);
            OutputDebugStringA("[EXITO] Textura cargada!\n");
        }
        else {
            // Debugging para ver donde esta buscando realmente el archivo
            char fullPath[1024];
            _fullpath(fullPath, "Assets/Textures/BaseColor.png", 1024);
            std::string err = "[ERROR] No se encuentra: " + std::string(fullPath) + "\n";
            OutputDebugStringA(err.c_str());
        }

        // Asignar recursos al actor
        m_repsolActor->setMesh(m_device, myMeshes);
        m_repsolActor->setTextures(myTextures);
        m_repsolActor->setName("RepsolBike");
        m_actors.push_back(m_repsolActor);

        // Posicion inicial de la moto
        m_repsolActor->getComponent<Transform>()->setTransform(
            EU::Vector3(0.0f, 0.0f, 0.0f),
            EU::Vector3(0.0f, 0.0f, 0.0f),
            EU::Vector3(0.1f, 0.1f, 0.1f) // Escala reducida
        );
    }

    // --------------------------------------------------------
    // 5. COMPILACION DE SHADERS Y LAYOUTS
    // --------------------------------------------------------
    std::vector<D3D11_INPUT_ELEMENT_DESC> Layout = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    // Intentamos cargar el shader, con fallback por si falla la ruta completa
    hr = m_shaderProgram.init(m_device, "Assets/Shaders/HeliosEngine.fx", Layout);
    if (FAILED(hr)) m_shaderProgram.init(m_device, "HeliosEngine.fx", Layout);

    // --------------------------------------------------------
    // 6. BUFFERS GLOBALES Y UI
    // --------------------------------------------------------
    m_cbNeverChanges.init(m_device, sizeof(CBNeverChanges));
    m_cbChangeOnResize.init(m_device, sizeof(CBChangeOnResize));

    // Matriz de Proyeccion (Perspectiva)
    m_Projection = XMMatrixPerspectiveFovLH(XM_PIDIV4, m_window.m_width / (FLOAT)m_window.m_height, 0.01f, 100.0f);
    cbChangesOnResize.mProjection = XMMatrixTranspose(m_Projection);

    // Iniciar ImGui
    UI.init(m_window.m_hWnd, m_device.m_device, m_deviceContext.m_deviceContext);

    return S_OK;
}

void BaseApp::update(float deltaTime) {
    // Iniciar frame de ImGui
    UI.update();

    // Variable estatica para mantener el valor del zoom entre frames
    static float cameraZoom = 30.0f;

    // --- VENTANA DE DEBUG (ImGui) ---
    ImGui::Begin("Control de Escena");

    // 1. Control de Camara
    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Camara");
    ImGui::DragFloat("Zoom", &cameraZoom, 0.5f, 1.0f, 100.0f);
    ImGui::SameLine();
    if (ImGui::Button("R##Cam")) cameraZoom = 30.0f; // Reset

    ImGui::Separator();

    // 2. Control de Transformacion del Actor
    if (!m_repsolActor.isNull()) {
        auto t = m_repsolActor->getComponent<Transform>();
        if (t) {
            ImGui::TextColored(ImVec4(0, 1, 1, 1), "Transformacion");

            // Posicion
            EU::Vector3 pos = t->getPosition(); float fPos[3] = { pos.x, pos.y, pos.z };
            ImGui::PushItemWidth(150);
            if (ImGui::DragFloat3("Pos", fPos, 0.1f)) t->setPosition(EU::Vector3(fPos[0], fPos[1], fPos[2]));
            ImGui::PopItemWidth(); ImGui::SameLine();
            if (ImGui::Button("R##Pos")) t->setPosition(EU::Vector3(0, 0, 0));

            // Rotacion
            EU::Vector3 rot = t->getRotation(); float fRot[3] = { rot.x, rot.y, rot.z };
            ImGui::PushItemWidth(150);
            if (ImGui::DragFloat3("Rot", fRot, 0.1f)) t->setRotation(EU::Vector3(fRot[0], fRot[1], fRot[2]));
            ImGui::PopItemWidth(); ImGui::SameLine();
            if (ImGui::Button("R##Rot")) t->setRotation(EU::Vector3(0, 0, 0));

            // Escala
            EU::Vector3 s = t->getScale(); float fS[3] = { s.x, s.y, s.z };
            ImGui::PushItemWidth(150);
            if (ImGui::DragFloat3("Scale", fS, 0.01f)) t->setScale(EU::Vector3(fS[0], fS[1], fS[2]));
            ImGui::PopItemWidth(); ImGui::SameLine();
            if (ImGui::Button("R##Sca")) t->setScale(EU::Vector3(0.1f, 0.1f, 0.1f));
        }
    }
    ImGui::End();

    // --- LOGICA DE CAMARA ---
    // Actualizamos la Vista basandonos en el zoom modificado por la UI
    XMVECTOR Eye = XMVectorSet(0.0f, 10.0f, -cameraZoom, 0.0f);
    XMVECTOR At = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
    XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    m_View = XMMatrixLookAtLH(Eye, At, Up);

    // Actualizar Buffers Constantes Globales
    cbNeverChanges.mView = XMMatrixTranspose(m_View);
    m_cbNeverChanges.update(m_deviceContext, nullptr, 0, nullptr, &cbNeverChanges, 0, 0);
    m_cbChangeOnResize.update(m_deviceContext, nullptr, 0, nullptr, &cbChangesOnResize, 0, 0);

    // Actualizar logica de todos los actores
    for (auto& actor : m_actors) actor->update(deltaTime, m_deviceContext);
}

void BaseApp::render() {
    // 1. Limpiar Pantalla (Fondo gris oscuro)
    float ClearColor[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
    m_renderTargetView.render(m_deviceContext, m_depthStencilView, 1, ClearColor);

    // 2. Configurar Pipeline
    m_viewport.render(m_deviceContext);
    m_depthStencilView.render(m_deviceContext); // Activar Z-Buffer
    m_shaderProgram.render(m_deviceContext);    // Activar Shaders

    // 3. Bindear Buffers Constantes Globales
    m_cbNeverChanges.render(m_deviceContext, 0, 1);
    m_cbChangeOnResize.render(m_deviceContext, 1, 1);

    // 4. Dibujar Actores
    for (auto& actor : m_actors) actor->render(m_deviceContext);

    // 5. Dibujar UI y Presentar (Swap Buffers)
    UI.render();
    m_swapChain.present();
}

void BaseApp::destroy() {
    // Limpieza de memoria en orden inverso a la creacion (generalmente)
    UI.destroy();
    if (m_deviceContext.m_deviceContext) m_deviceContext.m_deviceContext->ClearState();

    if (m_model) { delete m_model; m_model = nullptr; }

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
}

LRESULT BaseApp::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    // Pasar eventos a ImGui primero (clicks, teclado)
    if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam)) return true;

    switch (message) {
    case WM_CREATE: {
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pCreate->lpCreateParams);
    } return 0;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
    } return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}