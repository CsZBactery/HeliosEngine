// ======================================================================================
// Archivo: Skybox.cpp
// Implementación del entorno 3D (Cielo) para HeliosEngine. 
// Utiliza un cubo gigante proyectado alrededor de la cámara.
// ======================================================================================

#include "EngineUtilities/Utilities/Skybox.h"
#include "Device.h"
#include "DeviceContext.h"
#include "EngineUtilities/Utilities/LayoutBuilder.h"

// Inicializa la geometría, shaders y buffers necesarios para dibujar el cielo
HRESULT
Skybox::init(Device& device, DeviceContext* deviceContext, Texture& cubemap) {
    destroy(); // Limpiar recursos previos por seguridad

    // Guardamos la textura del mapa de cubos (las 6 imágenes del cielo)
    m_skyboxTexture = cubemap;

    // 1) GEOMETRÍA DEL CUBO
    // Definimos los 8 vértices de un cubo unitario centrado en el origen (0,0,0).
    // El tamaño físico no importa gracias al truco de la matriz de vista sin traslación.
    const SkyboxVertex vertices[] = {
        {-1,-1,-1}, {-1,+1,-1}, {+1,+1,-1}, {+1,-1,-1}, // Cara trasera (-Z)
        {-1,-1,+1}, {-1,+1,+1}, {+1,+1,+1}, {+1,-1,+1}, // Cara delantera (+Z)
    };

    // Definimos el orden para conectar los puntos y formar los 12 triángulos (36 índices)
    const unsigned int indices[] = {
        0,1,2, 0,2,3, // Atrás (-Z)
        4,6,5, 4,7,6, // Frente (+Z)
        4,5,1, 4,1,0, // Izquierda (-X)
        3,2,6, 3,6,7, // Derecha (+X)
        1,5,6, 1,6,2, // Arriba (+Y)
        4,0,3, 4,3,7  // Abajo (-Y)
    };

    // 2) CREACIÓN DEL ACTOR DEL ENTORNO
    m_skybox = EU::MakeShared<Actor>(device);

    if (!m_skybox.isNull()) {
        std::vector<MeshComponent> skybox;

        // Inyectamos la geometría estática directamente desde la RAM
        m_cubeModel = new Model3D("Skybox", vertices, indices);
        skybox = m_cubeModel->GetMeshes();

        // Asignamos la malla al actor.
        m_skybox->setMesh(device, skybox);
        m_skybox->setName("skybox");
    }
    else {
        ERROR("Skybox", "Init", "Failed to create Skybox Actor.");
        return E_FAIL;
    }

    // 3) CONFIGURACIÓN DE SHADERS (Input Layout usando el Builder del profesor)
    // Para el Skybox, el Shader solo necesita saber la Posición 3D (x, y, z).
    LayoutBuilder builder;
    builder.Add("POSITION", DXGI_FORMAT_R32G32B32_FLOAT);

    HRESULT hr = S_OK;

    // Búsqueda robusta del shader del Skybox
    hr = m_shaderProgram.init(device, "Assets/Shaders/Skybox.hlsl", builder);
    if (FAILED(hr)) hr = m_shaderProgram.init(device, "Skybox.hlsl", builder);
    if (FAILED(hr)) hr = m_shaderProgram.init(device, "Assets/Shaders/Skybox.fx", builder);
    if (FAILED(hr)) hr = m_shaderProgram.init(device, "Skybox.fx", builder);

    if (FAILED(hr)) {
        ERROR("Skybox", "init", ("Failed to initialize ShaderProgram. HRESULT: " + std::to_string(hr)).c_str());
        return hr;
    }

    // Buffer Constante para enviarle la matriz de la cámara al Shader
    hr = m_constantBuffer.init(device, sizeof(CBSkybox));
    if (FAILED(hr)) {
        ERROR("Skybox", "init", ("Failed to initialize Constant Buffer. HRESULT: " + std::to_string(hr)).c_str());
        return hr;
    }

    // Sampler: Define cómo se filtra la textura del cielo
    hr = m_samplerState.init(device);
    if (FAILED(hr)) {
        ERROR("Skybox", "init", "Failed to create new SamplerState");
    }

    // 4) CONFIGURACIÓN DE ESTADOS ESPECÍFICOS PARA EL CIELO

    // Rasterizer: CULL_FRONT -> Dibujamos las caras INTERNAS del cubo porque la cámara está adentro.
    hr = m_rasterizerState.init(device, D3D11_FILL_SOLID, D3D11_CULL_FRONT, false, true);
    if (FAILED(hr)) {
        ERROR("Skybox", "init", "Failed to create new RasterizerState");
    }

    // DepthStencil: WRITE_MASK_ZERO -> El cielo no escribe profundidad (no tapa a los modelos 3D).
    // COMPARISON_LESS_EQUAL -> Asegura que se dibuje en el límite exacto más lejano (Z = 1.0).
    hr = m_depthStencilState.init(device, true, D3D11_DEPTH_WRITE_MASK_ZERO, D3D11_COMPARISON_LESS_EQUAL);
    if (FAILED(hr)) {
        ERROR("Skybox", "init", "Failed to create new DepthStencilState");
    }

    return S_OK;
}

// ======================================================================================
// LÓGICA DE ACTUALIZACIÓN (CPU)
// Calcula la matriz de Vista para el truco del cielo infinito.
// ======================================================================================
void Skybox::update(DeviceContext& deviceContext, Camera& camera) {
    // Le borramos la posición a la cámara. Solo nos importa a dónde mira (Rotación).
    // Esto crea la ilusión óptica de que el cielo está infinitamente lejos y nunca lo alcanzas.
    XMMATRIX viewNoT = camera.GetViewNoTranslation();
    XMMATRIX vp = viewNoT * camera.getProj();

    CBSkybox cb{};
    cb.mviewProj = XMMatrixTranspose(vp); // Transponemos para que HLSL lo lea correctamente

    // Enviamos la matriz a la VRAM
    m_constantBuffer.update(deviceContext, nullptr, 0, nullptr, &cb, 0, 0);
}

// ======================================================================================
// DIBUJADO DEL CIELO (GPU)
// ======================================================================================
void Skybox::render(DeviceContext& deviceContext) {
    // Guardia de seguridad: Evita crashes si la textura o modelo no cargaron
    if (!m_cubeModel || !m_skyboxTexture.m_textureFromImg) return;

    // 1) Aplicamos las reglas especiales de dibujo (estar adentro del cubo, pintar al fondo)
    m_rasterizerState.render(deviceContext);
    m_depthStencilState.render(deviceContext, 0, false);

    // 2) Vinculamos la matriz que calculamos en el update()
    m_constantBuffer.render(deviceContext, 0, 1);

    // 3) Activamos Shaders y Filtros
    m_shaderProgram.render(deviceContext);

    // IMPORTANTÍSIMO: Usamos el Slot 10 para no interferir con el PBR (Materiales del xbox)
    m_samplerState.render(deviceContext, 10, 1);
    m_skyboxTexture.render(deviceContext, 10, 1);

    // 4) Renderizamos usando la función especializada que agregamos a Actor
    // Esto asegura que el Input Assembler se configure correctamente antes del DrawIndexed
    m_skybox->renderForSkybox(deviceContext);

    // 5) FASE DE LIMPIEZA
    // Limpiamos los slots de textura para evitar texturas fantasma ("mismatch") 
    // en los siguientes modelos o en la UI 2D.
    ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
    deviceContext.m_deviceContext->PSSetShaderResources(10, 1, nullSRV);
    deviceContext.m_deviceContext->PSSetShaderResources(0, 1, nullSRV);
}

// ======================================================================================
// FASE DE LIMPIEZA (DESTRUCTOR)
// Libera la memoria de la tarjeta gráfica y la RAM ocupada por el entorno
// ======================================================================================
void Skybox::destroy() {
    // Liberar el modelo 3D dinámico
    if (m_cubeModel) {
        delete m_cubeModel;
        m_cubeModel = nullptr;
    }

    // Liberar buffers, shaders, estados y texturas
    m_constantBuffer.destroy();
    m_shaderProgram.destroy();
    m_samplerState.destroy();
    m_skyboxTexture.destroy();

    // Limpiamos los estados de DirectX
    m_rasterizerState.destroy();
    m_depthStencilState.destroy();
}