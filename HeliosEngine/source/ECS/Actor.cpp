#include "../include/ECS/Actor.h"
#include "../include/MeshComponent.h"
#include "../include/Device.h"
#include "../include/DeviceContext.h"
#include "../include/ECS/Transform.h" 

Actor::Actor(Device& device) {
    // ----------------------------------------------------
    // 1. Configuración de Componentes por defecto
    // ----------------------------------------------------
    EU::TSharedPointer<Transform> transform = EU::MakeShared<Transform>();
    addComponent(transform);

    EU::TSharedPointer<MeshComponent> meshComponent = EU::MakeShared<MeshComponent>();
    addComponent(meshComponent);

    // ----------------------------------------------------
    // 2. Inicialización de Recursos Gráficos
    // ----------------------------------------------------
    HRESULT hr;
    std::string classNameType = "Actor -> " + m_name;

    // Inicializar Constant Buffer (Matrices por frame)
    hr = m_modelBuffer.init(device, sizeof(CBChangesEveryFrame));
    if (FAILED(hr)) {
        ERROR("Actor", classNameType.c_str(), "Failed to create new CBChangesEveryFrame");
    }

    // Llamada a awake() antes de terminar la inicialización gráfica
    awake();

    // Inicializar Sampler State
    hr = m_sampler.init(device);
    if (FAILED(hr)) {
        ERROR("Actor", classNameType.c_str(), "Failed to create new SamplerState");
    }

    // ----------------------------------------------------
    // 3. Recursos Futuros (Comentados)
    // ----------------------------------------------------

    // Rasterizer (Wireframe, Cull mode)
    /*
    hr = m_rasterizer.init(device);
    if (FAILED(hr)) {
        ERROR("Actor", classNameType.c_str(), "Failed to create new Rasterizer");
    }
    */

    // Blend State (Transparencias)
    /*
    hr = m_blendstate.init(device);
    if (FAILED(hr)) {
        ERROR("Actor", classNameType.c_str(), "Failed to create new BlendState");
    }
    */

    // Shadow Mapping Setup
    /*
    hr = m_shaderShadow.CreateShader(device, PIXEL_SHADER, "HybridEngine.fx");
    if (FAILED(hr)) {
        ERROR("Main", "InitDevice", ("Failed to initialize Shadow Shader. HRESULT: " + std::to_string(hr)).c_str());
    }

    hr = m_shaderBuffer.init(device, sizeof(CBChangesEveryFrame));
    if (FAILED(hr)) {
        ERROR("Main", "InitDevice", ("Failed to initialize Shadow Buffer. HRESULT: " + std::to_string(hr)).c_str());
    }

    hr = m_shadowBlendState.init(device);
    if (FAILED(hr)) {
        ERROR("Main", "InitDevice", ("Failed to initialize Shadow Blend State. HRESULT: " + std::to_string(hr)).c_str());
    }

    hr = m_shadowDepthStencilState.init(device, true, false);
    if (FAILED(hr)) {
        ERROR("Main", "InitDevice", ("Failed to initialize Depth Stencil State. HRESULT: " + std::to_string(hr)).c_str());
    }

    m_LightPos = XMFLOAT4(2.0f, 4.0f, -2.0f, 1.0f);
    */
}

void Actor::update(float deltaTime, DeviceContext& deviceContext) {
    // Actualizar todos los componentes
    for (auto& component : m_components) {
        if (component) {
            component->update(deltaTime);
        }
    }

    // Actualizar la estructura del buffer del modelo
    // Transponemos la matriz porque HLSL usa orden por columnas
    m_model.mWorld = XMMatrixTranspose(getComponent<Transform>()->matrix);
    m_model.vMeshColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

    // Actualizar el Constant Buffer en GPU
    m_modelBuffer.update(deviceContext, nullptr, 0, nullptr, &m_model, 0, 0);
}

void Actor::render(DeviceContext& deviceContext) {
    // ----------------------------------------------------
    // 1. Pase de Sombras (Comentado)
    // ----------------------------------------------------
    /*
    if (canCastShadow()) {
        renderShadow(deviceContext);
    }
    */

    // ----------------------------------------------------
    // 2. Pase Principal
    // ----------------------------------------------------

    // Configurar estados (Blend y Rasterizer comentados)
    // m_blendstate.render(deviceContext);
    // m_rasterizer.render(deviceContext);

    // Configurar Sampler
    m_sampler.render(deviceContext, 0, 1);

    // Topología
    deviceContext.IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Renderizar todas las sub-mallas
    for (unsigned int i = 0; i < m_meshes.size(); i++) {
        // Vincular Vertex e Index Buffers
        m_vertexBuffers[i].render(deviceContext, 0, 1);
        m_indexBuffers[i].render(deviceContext, 0, 1, false, DXGI_FORMAT_R32_UINT);

        // Vincular el Constant Buffer estándar (World + Color) al slot 2
        m_modelBuffer.render(deviceContext, 2, 1, true);

        // Vincular Texturas
        if (m_meshes.size() > 0 && m_textures.size() > 0) {
            // Aseguramos no salirnos del rango si hay menos texturas que mallas
            if (i < m_textures.size()) {
                // Renderizar textura Albedo en slot t0
                if (m_textures.size() >= 1) {
                    m_textures[0].render(deviceContext, 0, 1);

                    // Slots reservados para PBR (Comentados)
                    // m_textures[1].render(deviceContext, 1, 1); // Normal -> t1
                    // m_textures[2].render(deviceContext, 2, 1); // Metallic -> t2
                    // m_textures[3].render(deviceContext, 3, 1); // Roughness -> t3
                    // m_textures[4].render(deviceContext, 4, 1); // AO -> t4
                }
            }
        }

        // Dibujar geometría
        deviceContext.DrawIndexed(m_meshes[i].m_numIndex, 0, 0);
    }
}

void Actor::destroy() {
    // Liberar Buffers de geometría
    for (auto& vertexBuffer : m_vertexBuffers) {
        vertexBuffer.destroy();
    }

    for (auto& indexBuffer : m_indexBuffers) {
        indexBuffer.destroy();
    }

    // Liberar Texturas
    for (auto& tex : m_textures) {
        tex.destroy();
    }

    // Liberar Buffers y Estados
    m_modelBuffer.destroy();
    m_sampler.destroy();

    // Liberar recursos futuros (Comentados)
    // m_rasterizer.destroy();
    // m_blendstate.destroy();
}

void Actor::setMesh(Device& device, std::vector<MeshComponent> meshes) {
    m_meshes = meshes;
    HRESULT hr;

    for (auto& mesh : m_meshes) {
        // Crear Vertex Buffer
        Buffer vertexBuffer;
        hr = vertexBuffer.init(device, mesh, D3D11_BIND_VERTEX_BUFFER);
        if (FAILED(hr)) {
            ERROR("Actor", "setMesh", "Failed to create new vertexBuffer");
        }
        else {
            m_vertexBuffers.push_back(vertexBuffer);
        }

        // Crear Index Buffer
        Buffer indexBuffer;
        hr = indexBuffer.init(device, mesh, D3D11_BIND_INDEX_BUFFER);
        if (FAILED(hr)) {
            ERROR("Actor", "setMesh", "Failed to create new indexBuffer");
        }
        else {
            m_indexBuffers.push_back(indexBuffer);
        }
    }
}