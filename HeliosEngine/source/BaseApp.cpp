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

// Nuevos includes del profesor para Guardado/Carga y utilidades
#include <algorithm>
#include <cctype>
#include <fstream>
#include <iomanip>

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
    // 1. Inicializar la ventana
    if (FAILED(m_window.init(hInst, nCmdShow, WndProc, this))) {
        ERROR("Main", "Run", "Failed to initialize window.");
        return 0;
    }

    // 2. Despertar los subsistemas lógicos del motor
    if (FAILED(awake())) {
        ERROR("Main", "Run", "Failed to awake application.");
        return 0;
    }

    // 3. Iniciar DirectX 11 y cargar recursos
    if (FAILED(init())) {
        ERROR("Main", "Run", "Failed to initialize device and device context.");
        return 0;
    }

    // 4. Inicializar la interfaz gráfica de usuario (ImGui)
    m_gui.init(m_window, m_device, m_deviceContext);
    m_guiInitialized = true;

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

    hr = m_swapChain.init(m_device, m_deviceContext, m_backBuffer, m_window);
    if (FAILED(hr)) return hr;

    hr = m_renderTargetView.init(m_device, m_backBuffer, DXGI_FORMAT_R8G8B8A8_UNORM);
    if (FAILED(hr)) return hr;

    // MSAA Fix: 4, 16 para evitar crashes en el DepthStencil
    hr = m_depthStencil.init(m_device, m_window.m_width, m_window.m_height, DXGI_FORMAT_D24_UNORM_S8_UINT, D3D11_BIND_DEPTH_STENCIL, 4, 16);
    if (FAILED(hr)) return hr;

    hr = m_depthStencilView.init(m_device, m_depthStencil, DXGI_FORMAT_D24_UNORM_S8_UINT);
    if (FAILED(hr)) return hr;

    hr = m_viewport.init(m_window);
    if (FAILED(hr)) return hr;

    m_d3dReady = true;

    // =========================================================
    // CONFIGURACIÓN DE SHADERS Y CONSTANT BUFFERS
    // =========================================================
    LayoutBuilder builder;
    builder.Add("POSITION", DXGI_FORMAT_R32G32B32_FLOAT)
        .Add("NORMAL", DXGI_FORMAT_R32G32B32_FLOAT)
        .Add("TANGENT", DXGI_FORMAT_R32G32B32_FLOAT)
        .Add("BITANGENT", DXGI_FORMAT_R32G32B32_FLOAT)
        .Add("TEXCOORD", DXGI_FORMAT_R32G32_FLOAT);

    hr = m_shaderProgram.init(m_device, "Assets/Shaders/PBRShader.hlsl", builder);
    if (FAILED(hr)) hr = m_shaderProgram.init(m_device, "PBRShader.hlsl", builder);
    if (FAILED(hr)) return hr;

    hr = m_constantBuffer.init(m_device, sizeof(CBMain));
    if (FAILED(hr)) return hr;

    // =========================================================
    // ESTADOS GLOBALES DE RENDERIZADO (Sampler, Rasterizer)
    // =========================================================
    hr = m_defaultRasterizer.init(m_device, D3D11_FILL_SOLID, D3D11_CULL_NONE, false, true);
    if (FAILED(hr)) return hr;

    hr = m_defaultDepthStencil.init(m_device, true, D3D11_DEPTH_WRITE_MASK_ALL, D3D11_COMPARISON_LESS);
    if (FAILED(hr)) return hr;

    hr = m_defaultSampler.init(m_device); // NUEVO: Inicializar Sampler
    if (FAILED(hr)) return hr;

    // =========================================================
    // CREACIÓN DE MATERIALES (PBR Base)
    // =========================================================
    m_pbrMaterial.setShader(&m_shaderProgram);
    m_pbrMaterial.setRasterizerState(&m_defaultRasterizer);
    m_pbrMaterial.setDepthStencilState(&m_defaultDepthStencil);
    m_pbrMaterial.setSamplerState(&m_defaultSampler);
    m_pbrMaterial.setDomain(MaterialDomain::Opaque);
    m_pbrMaterial.setBlendMode(BlendMode::Opaque);

    m_transparentPbrMaterial.setShader(&m_shaderProgram);
    m_transparentPbrMaterial.setRasterizerState(&m_defaultRasterizer);
    m_transparentPbrMaterial.setDepthStencilState(&m_defaultDepthStencil);
    m_transparentPbrMaterial.setSamplerState(&m_defaultSampler);
    m_transparentPbrMaterial.setDomain(MaterialDomain::Transparent);
    m_transparentPbrMaterial.setBlendMode(BlendMode::Alpha);

    // =========================================================
    // CARGA DE RECURSOS DEL USUARIO (Skybox y Moto Repsol)
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

    m_cyberGun = EU::MakeShared<Actor>(m_device);

    if (!m_cyberGun.isNull()) {
        m_model = new Model3D("Assets/Moto/repsol3.obj", ModelType::OBJ);

        // --- ¡AQUÍ ESTÁ LA CORRECCIÓN CLAVE! ---
        if (!m_model || !m_model->load("Assets/Moto/repsol3.obj")) {
            ERROR("Main", "InitDevice", "Failed to load RepsolBike model.");
            return E_FAIL;
        }
        // ----------------------------------------

        hr = m_AlbedoSRV.init(m_device, "Assets/Textures/BaseColor", ExtensionType::PNG);
        if (FAILED(hr)) return hr;

        // Configurar la instancia del Material para tu Moto (Fingiendo PBR con tu única textura)
        m_cyberGunMaterial.setMaterial(&m_pbrMaterial);
        m_cyberGunMaterial.setAlbedo(&m_AlbedoSRV);
        m_cyberGunMaterial.setNormal(&m_AlbedoSRV);
        m_cyberGunMaterial.setMetallic(&m_AlbedoSRV);
        m_cyberGunMaterial.setRoughness(&m_AlbedoSRV);
        m_cyberGunMaterial.setAO(&m_AlbedoSRV);

        m_cyberGunMaterial.getParams().baseColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
        m_cyberGunMaterial.getParams().metallic = 0.0f; // Bajamos el metálico para que no se vea negra tu moto
        m_cyberGunMaterial.getParams().roughness = 0.8f;
        m_cyberGunMaterial.getParams().ao = 1.0f;
        m_cyberGunMaterial.getParams().normalScale = 1.0f;
        m_cyberGunMaterial.getParams().emissiveStrength = 0.0f;
        m_cyberGunMaterial.getParams().alphaCutoff = 0.5f;

        // Construir la Malla en el formato que exige el nuevo Renderer
        m_cyberGunRenderMesh.destroy();
        for (const MeshComponent& meshComponent : m_model->GetMeshes()) {
            Submesh submesh{};
            hr = submesh.vertexBuffer.init(m_device, meshComponent, D3D11_BIND_VERTEX_BUFFER);
            if (FAILED(hr)) return hr;

            hr = submesh.indexBuffer.init(m_device, meshComponent, D3D11_BIND_INDEX_BUFFER);
            if (FAILED(hr)) return hr;

            submesh.indexCount = meshComponent.m_numIndex;
            submesh.materialSlot = 0;
            m_cyberGunRenderMesh.getSubmeshes().push_back(std::move(submesh));
        }

        // Asignar el componente de Renderizado al Actor
        EU::TSharedPointer<MeshRendererComponent> meshRenderer = m_cyberGun->getComponent<MeshRendererComponent>();
        if (!meshRenderer) {
            meshRenderer = EU::MakeShared<MeshRendererComponent>();
            m_cyberGun->addComponent(meshRenderer);
        }
        meshRenderer->setMesh(&m_cyberGunRenderMesh);
        meshRenderer->setMaterialInstance(&m_cyberGunMaterial);
        meshRenderer->setVisible(true);
        meshRenderer->setCastShadow(true);

        m_cyberGun->setName("RepsolBike");
        m_actors.push_back(m_cyberGun);

        m_cyberGun->getComponent<Transform>()->setTransform(
            EU::Vector3(0.0f, -4.0f, 0.0f),
            EU::Vector3(0.0f, 0.0f, 0.0f),
            EU::Vector3(5.0f, 5.0f, 5.0f)
        );
    }

    // =========================================================
    // CREACIÓN DEL ACTOR LUZ DIRECCIONAL (NUEVO)
    // =========================================================
    m_directionalLightActor = EU::MakeShared<Actor>(m_device);
    if (!m_directionalLightActor.isNull()) {
        m_directionalLightActor->setName("DirectionalLight");
        EU::TSharedPointer<LightComponent> lightComponent = m_directionalLightActor->getComponent<LightComponent>();
        if (!lightComponent) {
            lightComponent = EU::MakeShared<LightComponent>();
            m_directionalLightActor->addComponent(lightComponent);
        }

        lightComponent->getLightData().type = LightType::Directional;
        lightComponent->getLightData().direction = EU::Vector3(-0.20f, -1.0f, 1.0f);
        lightComponent->getLightData().color = EU::Vector3(1.0f, 1.0f, 1.0f);
        lightComponent->getLightData().intensity = 1.0f;
        lightComponent->setCastShadow(false);

        m_actors.push_back(m_directionalLightActor); // Lo agregamos a la lista
    }

    // Registrar actores en el Grafo de Escena
    for (auto& actor : m_actors) {
        m_sceneGraph.addEntity(actor.get());
    }

    // =========================================================
    // INICIALIZACIÓN DE SUBSISTEMAS RESTANTES
    // =========================================================
    m_camera.setLens(XM_PIDIV4, m_window.m_width / (float)m_window.m_height, 0.01f, 100.0f);
    m_camera.setPosition(0.0f, 3.0f, -6.0f);

    m_constantBufferStruct.LightColor = EU::Vector3(1.0f, 1.0f, 1.0f);
    m_constantBufferStruct.LightDir = EU::Vector3(-0.20f, -1.0f, 1.0f);

    m_skybox.init(m_device, &m_deviceContext, m_skyboxTex);

    hr = m_editorViewportPass.init(m_device, 1280, 720);
    if (FAILED(hr)) return hr;

    hr = m_forwardRenderer.init(m_device); // Inicializar nuevo Forward Renderer
    if (FAILED(hr)) return hr;

    // Intentar cargar escena predeterminada
    loadScene(getDefaultScenePath());

    return S_OK;
}

// ======================================================================================
// FASE 4: UPDATE (Lógica de cada Frame)
// ======================================================================================
void BaseApp::update(float deltaTime) {

    // Actualización de la GUI (ImGui)
    m_gui.update(m_viewport, m_window);

    m_gui.drawViewportPanel(m_editorViewportPass.getSRV());

    // Configuración de UI del Inspector
    m_gui.outliner(m_actors);
    EU::TSharedPointer<Actor> selectedActor;
    if (m_gui.selectedActorIndex >= 0 && m_gui.selectedActorIndex < static_cast<int>(m_actors.size())) {
        selectedActor = m_actors[m_gui.selectedActorIndex];
    }
    m_gui.inspectorGeneral(selectedActor);
    m_gui.editTransform(m_camera, m_window, selectedActor);

    // Captura del evento "Save Scene" desde ImGui
    // asumiendo que el profe añadió m_gui.consumeSaveSceneRequest() en GUI.h
    // Si tienes error aquí, coméntalo hasta que el profe pase su GUI actualizado.
    // if (m_gui.consumeSaveSceneRequest()) { saveScene(getDefaultScenePath()); }

    // =========================================================
    // LÓGICA DE CREACIÓN DE CUBO ADAPTADA A LA NUEVA ARQUITECTURA
    // =========================================================
    if (m_gui.m_requestSpawnCube) {
        auto newCube = EU::MakeShared<Actor>(m_device);
        if (!newCube.isNull()) {
            static Model3D* s_cubeModel = nullptr;
            static Mesh* s_cubeMesh = nullptr;
            static MaterialInstance* s_cubeMat = nullptr;

            // Instanciar malla en memoria solo la primera vez para no generar memory leaks
            if (!s_cubeModel) {
                s_cubeModel = new Model3D("Assets/Models/cube.obj", ModelType::OBJ);
                s_cubeMesh = new Mesh();
                for (const auto& mc : s_cubeModel->GetMeshes()) {
                    Submesh sm;
                    sm.vertexBuffer.init(m_device, mc, D3D11_BIND_VERTEX_BUFFER);
                    sm.indexBuffer.init(m_device, mc, D3D11_BIND_INDEX_BUFFER);
                    sm.indexCount = mc.m_numIndex;
                    sm.materialSlot = 0;
                    s_cubeMesh->getSubmeshes().push_back(std::move(sm));
                }
                s_cubeMat = new MaterialInstance();
                s_cubeMat->setMaterial(&m_pbrMaterial);
                s_cubeMat->setAlbedo(&m_AlbedoSRV); // Material blanco base
                s_cubeMat->setNormal(&m_AlbedoSRV);
                s_cubeMat->setMetallic(&m_AlbedoSRV);
                s_cubeMat->setRoughness(&m_AlbedoSRV);
                s_cubeMat->setAO(&m_AlbedoSRV);
            }

            auto mr = EU::MakeShared<MeshRendererComponent>();
            mr->setMesh(s_cubeMesh);
            mr->setMaterialInstance(s_cubeMat);
            mr->setVisible(true);
            mr->setCastShadow(true);
            newCube->addComponent(mr);

            newCube->setName("New Part");
            newCube->getComponent<Transform>()->setTransform(
                EU::Vector3(0.0f, 0.0f, 0.0f), EU::Vector3(0.0f, 0.0f, 0.0f), EU::Vector3(1.0f, 1.0f, 1.0f)
            );

            m_actors.push_back(newCube);
            m_sceneGraph.addEntity(newCube.get());
        }
        m_gui.m_requestSpawnCube = false;
    }

    // =========================================================
    // LUZ EN INTERFAZ
    // =========================================================
    ImGui::Begin("Lighting Settings");
    float fDir[3] = { m_constantBufferStruct.LightDir.x, m_constantBufferStruct.LightDir.y, m_constantBufferStruct.LightDir.z };
    m_gui.vec3Control("Light Direction", fDir, 0.1f);
    m_constantBufferStruct.LightDir = EU::Vector3(fDir[0], fDir[1], fDir[2]);

    float fCol[3] = { m_constantBufferStruct.LightColor.x, m_constantBufferStruct.LightColor.y, m_constantBufferStruct.LightColor.z };
    m_gui.vec3Control("Light Color", fCol, 0.1f);
    m_constantBufferStruct.LightColor = EU::Vector3(fCol[0], fCol[1], fCol[2]);
    ImGui::End();

    // Sincronizar el componente de luz del actor con el Constant Buffer
    if (!m_directionalLightActor.isNull()) {
        EU::TSharedPointer<LightComponent> lightComponent = m_directionalLightActor->getComponent<LightComponent>();
        if (lightComponent) {
            lightComponent->getLightData().direction = m_constantBufferStruct.LightDir;
            lightComponent->getLightData().color = m_constantBufferStruct.LightColor;
        }
    }

    // =========================================================
    // REDIMENSIONAMIENTO DEL EDITOR VIEWPORT
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

    // Actualización de matrices
    m_camera.updateViewMatrix();
    XMStoreFloat4x4(&m_constantBufferStruct.View, XMMatrixTranspose(m_camera.getView()));
    XMStoreFloat4x4(&m_constantBufferStruct.Projection, XMMatrixTranspose(m_camera.getProj()));
    m_constantBufferStruct.CameraPos = m_camera.getPosition();

    m_skybox.update(m_deviceContext, m_camera);
    m_constantBuffer.update(m_deviceContext, nullptr, 0, nullptr, &m_constantBufferStruct, 0, 0);
    m_sceneGraph.update(deltaTime, m_deviceContext);
}

// ======================================================================================
// FASE 5: RENDER (NUEVO FORWARD RENDERER)
// ======================================================================================
void BaseApp::render() {

    handleEditorViewportResize();

    float clearColor[4] = { 0.1f, 0.1f, 0.1f, 1.0f };

    // 1. Recolectar objetos de la escena para enviarlos al motor de renderizado
    m_renderScene.clear();
    m_sceneGraph.gatherRenderScene(m_renderScene, m_camera);
    m_renderScene.skybox = &m_skybox;

    // 2. Ejecutar el pipeline moderno (Reemplaza las docenas de llamadas previas)
    m_forwardRenderer.render(
        m_deviceContext,
        m_camera,
        m_renderScene,
        m_editorViewportPass
    );

    // 3. Dibujar al BackBuffer principal (Lienzo final)
    m_renderTargetView.render(m_deviceContext, m_depthStencilView, 1, clearColor);
    m_viewport.render(m_deviceContext);
    m_depthStencilView.render(m_deviceContext);

    // 4. Dibujar Interfaz y Presentar
    m_gui.render();
    m_swapChain.present();
}

// ======================================================================================
// FASE 6: DESTROY (Limpieza de Memoria)
// ======================================================================================
void BaseApp::destroy() {
    if (m_deviceContext.m_deviceContext) m_deviceContext.m_deviceContext->ClearState();

    m_sceneGraph.destroy();
    m_editorViewportPass.destroy();
    m_forwardRenderer.destroy();
    m_cyberGunRenderMesh.destroy();
    m_drakefireRenderMesh.destroy();

    m_AlbedoSRV.destroy();
    m_MetallicSRV.destroy();
    m_NormalSRV.destroy();
    m_RoughnessSRV.destroy();
    m_AOSRV.destroy();
    m_EmissiveSRV.destroy();

    m_skybox.destroy();
    m_defaultRasterizer.destroy();
    m_defaultDepthStencil.destroy();
    m_defaultSampler.destroy();

    m_constantBuffer.destroy();
    m_shaderProgram.destroy();
    m_depthStencil.destroy();
    m_depthStencilView.destroy();
    m_renderTargetView.destroy();
    m_swapChain.destroy();
    m_backBuffer.destroy();

    if (m_guiInitialized) {
        m_gui.destroy();
        m_guiInitialized = false;
    }

    delete m_model;
    m_model = nullptr;

    m_deviceContext.destroy();
    m_device.destroy();
}

// ======================================================================================
// NUEVAS FUNCIONES DE GUARDADO / CARGA DE ESCENAS
// ======================================================================================
std::string BaseApp::getDefaultScenePath() const {
    CreateDirectoryA("Saved", nullptr);
    return "Saved/DefaultScene.wvscene";
}

bool BaseApp::saveScene(const std::string& path) {
    std::ofstream stream(path, std::ios::trunc);
    if (!stream.is_open()) {
        ERROR("Main", "saveScene", ("Failed to open scene file for writing: " + path).c_str());
        return false;
    }

    stream << "WVSCENE 1\n";
    stream << "ACTOR_COUNT " << m_actors.size() << "\n";

    for (size_t actorIndex = 0; actorIndex < m_actors.size(); ++actorIndex) {
        const EU::TSharedPointer<Actor>& actor = m_actors[actorIndex];
        if (actor.isNull()) continue;

        stream << "ACTOR " << actorIndex << " " << std::quoted(actor->getName()) << "\n";

        EU::TSharedPointer<Transform> transform = actor->getComponent<Transform>();
        if (transform) {
            const EU::Vector3& position = transform->getPosition();
            const EU::Vector3& rotation = transform->getRotation();
            const EU::Vector3& scale = transform->getScale();
            stream << "POSITION " << position.x << " " << position.y << " " << position.z << "\n";
            stream << "ROTATION " << rotation.x << " " << rotation.y << " " << rotation.z << "\n";
            stream << "SCALE " << scale.x << " " << scale.y << " " << scale.z << "\n";
        }

        EU::TSharedPointer<MeshRendererComponent> meshRenderer = actor->getComponent<MeshRendererComponent>();
        if (meshRenderer) {
            stream << "VISIBLE " << (meshRenderer->isVisible() ? 1 : 0) << "\n";
            stream << "CAST_SHADOW " << (meshRenderer->canCastShadow() ? 1 : 0) << "\n";

            const std::vector<MaterialInstance*>& materials = meshRenderer->getMaterialInstances();
            stream << "MATERIAL_COUNT " << materials.size() << "\n";
            for (size_t i = 0; i < materials.size(); ++i) {
                MaterialInstance* materialInstance = materials[i];
                if (!materialInstance) {
                    stream << "MATERIAL " << i << " 0 0 1 1 1 1 0 1 1 1 0.5\n";
                    continue;
                }

                Material* material = materialInstance->getMaterial();
                const MaterialParams& params = materialInstance->getParams();
                const int domain = material ? static_cast<int>(material->getDomain()) : 0;
                const int blendMode = material ? static_cast<int>(material->getBlendMode()) : 0;

                stream << "MATERIAL " << i << " " << domain << " " << blendMode << " "
                    << params.baseColor.x << " " << params.baseColor.y << " " << params.baseColor.z << " " << params.baseColor.w << " "
                    << params.metallic << " " << params.roughness << " " << params.ao << " "
                    << params.normalScale << " " << params.alphaCutoff << "\n";
            }
        }
        stream << "END_ACTOR\n";
    }

    stream << "LIGHT "
        << m_constantBufferStruct.LightDir.x << " " << m_constantBufferStruct.LightDir.y << " " << m_constantBufferStruct.LightDir.z << " "
        << m_constantBufferStruct.LightColor.x << " " << m_constantBufferStruct.LightColor.y << " " << m_constantBufferStruct.LightColor.z << "\n";

    stream << "END_SCENE\n";
    const std::wstring pathW(path.begin(), path.end());
    MESSAGE("Main", "saveScene", L"Saved scene to '" << pathW << L"'")
        return true;
}

bool BaseApp::loadScene(const std::string& path) {
    std::ifstream stream(path);
    if (!stream.is_open()) return false;

    std::string token;
    stream >> token;
    if (token != "WVSCENE") return false;

    int version = 0;
    stream >> version;
    if (version != 1) return false;

    EU::TSharedPointer<Actor> currentActor;
    while (stream >> token) {
        if (token == "ACTOR_COUNT") { size_t ignoredCount = 0; stream >> ignoredCount; }
        else if (token == "ACTOR") {
            size_t actorIndex = 0; std::string actorName;
            stream >> actorIndex >> std::quoted(actorName);
            currentActor = EU::TSharedPointer<Actor>();
            if (actorIndex < m_actors.size()) currentActor = m_actors[actorIndex];
            if (!currentActor.isNull()) currentActor->setName(actorName);
        }
        else if (token == "POSITION" && !currentActor.isNull()) {
            float x = 0.0f, y = 0.0f, z = 0.0f; stream >> x >> y >> z;
            EU::TSharedPointer<Transform> transform = currentActor->getComponent<Transform>();
            if (transform) transform->setPosition(EU::Vector3(x, y, z));
        }
        else if (token == "ROTATION" && !currentActor.isNull()) {
            float x = 0.0f, y = 0.0f, z = 0.0f; stream >> x >> y >> z;
            EU::TSharedPointer<Transform> transform = currentActor->getComponent<Transform>();
            if (transform) transform->setRotation(EU::Vector3(x, y, z));
        }
        else if (token == "SCALE" && !currentActor.isNull()) {
            float x = 1.0f, y = 1.0f, z = 1.0f; stream >> x >> y >> z;
            EU::TSharedPointer<Transform> transform = currentActor->getComponent<Transform>();
            if (transform) transform->setScale(EU::Vector3(x, y, z));
        }
        else if (token == "VISIBLE" && !currentActor.isNull()) {
            int value = 1; stream >> value;
            EU::TSharedPointer<MeshRendererComponent> meshRenderer = currentActor->getComponent<MeshRendererComponent>();
            if (meshRenderer) meshRenderer->setVisible(value != 0);
        }
        else if (token == "CAST_SHADOW" && !currentActor.isNull()) {
            int value = 1; stream >> value;
            EU::TSharedPointer<MeshRendererComponent> meshRenderer = currentActor->getComponent<MeshRendererComponent>();
            if (meshRenderer) meshRenderer->setCastShadow(value != 0);
        }
        else if (token == "MATERIAL_COUNT") { size_t ignoredCount = 0; stream >> ignoredCount; }
        else if (token == "MATERIAL" && !currentActor.isNull()) {
            size_t materialIndex = 0; int domain = 0; int blendMode = 0; MaterialParams params{};
            stream >> materialIndex >> domain >> blendMode >> params.baseColor.x >> params.baseColor.y >> params.baseColor.z >> params.baseColor.w
                >> params.metallic >> params.roughness >> params.ao >> params.normalScale >> params.alphaCutoff;

            EU::TSharedPointer<MeshRendererComponent> meshRenderer = currentActor->getComponent<MeshRendererComponent>();
            if (meshRenderer) {
                const std::vector<MaterialInstance*>& materials = meshRenderer->getMaterialInstances();
                if (materialIndex < materials.size() && materials[materialIndex]) {
                    materials[materialIndex]->getParams() = params;
                    Material* material = materials[materialIndex]->getMaterial();
                    if (material) {
                        material->setDomain(static_cast<MaterialDomain>(domain));
                        material->setBlendMode(static_cast<BlendMode>(blendMode));
                    }
                }
            }
        }
        else if (token == "LIGHT") {
            stream >> m_constantBufferStruct.LightDir.x >> m_constantBufferStruct.LightDir.y >> m_constantBufferStruct.LightDir.z
                >> m_constantBufferStruct.LightColor.x >> m_constantBufferStruct.LightColor.y >> m_constantBufferStruct.LightColor.z;
            if (!m_directionalLightActor.isNull()) {
                EU::TSharedPointer<LightComponent> lightComponent = m_directionalLightActor->getComponent<LightComponent>();
                if (lightComponent) {
                    lightComponent->getLightData().direction = m_constantBufferStruct.LightDir;
                    lightComponent->getLightData().color = m_constantBufferStruct.LightColor;
                }
            }
        }
        else if (token == "END_ACTOR") { currentActor = EU::TSharedPointer<Actor>(); }
        else if (token == "END_SCENE") { break; }
    }
    const std::wstring pathW(path.begin(), path.end());
    MESSAGE("Main", "loadScene", L"Loaded scene from '" << pathW << L"'")
        return true;
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
void BaseApp::onResize(UINT newW, UINT newH) {
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

    // MSAA Fix conservado: 4, 16
    hr = m_depthStencil.init(m_device, newW, newH, DXGI_FORMAT_D24_UNORM_S8_UINT, D3D11_BIND_DEPTH_STENCIL, 4, 16);
    if (FAILED(hr)) return;

    hr = m_depthStencilView.init(m_device, m_depthStencil, DXGI_FORMAT_D24_UNORM_S8_UINT);
    if (FAILED(hr)) return;

    m_viewport.init(m_window);
    m_camera.setLens(XM_PIDIV4, newW / (float)newH, 0.01f, 100.0f);
}

void BaseApp::handleEditorViewportResize() {
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