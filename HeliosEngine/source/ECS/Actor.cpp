// ======================================================================================
// Archivo: Actor.cpp
// Implementación de la entidad base del motor (GameObject). 
// Maneja sus propios buffers, texturas y componentes (Transform, Mesh).
// ======================================================================================

#include "ECS/Actor.h"
#include "MeshComponent.h"
#include "Device.h"
#include "DeviceContext.h"

Actor::Actor(Device& device) {
    // 1. Añadir componentes por defecto (Todo actor tiene posición y forma)
    EU::TSharedPointer<Transform> transform = EU::MakeShared<Transform>();
    addComponent(transform);

    EU::TSharedPointer<MeshComponent> meshComponent = EU::MakeShared<MeshComponent>();
    addComponent(meshComponent);

    HRESULT hr;
    std::string classNameType = "Actor -> " + m_name;

    // 2. Inicializar Constant Buffer para la matriz de mundo específica de este actor
    hr = m_modelBuffer.init(device, sizeof(CBChangesEveryFrame));
    if (FAILED(hr)) {
        ERROR("Actor", classNameType.c_str(), "Failed to create new CBChangesEveryFrame");
    }

    // 3. Llamar al método virtual de inicialización temprana
    awake();

    // 4. Inicializar el Sampler State para el filtrado de texturas de este actor
    hr = m_sampler.init(device);
    if (FAILED(hr)) {
        ERROR("Actor", classNameType.c_str(), "Failed to create new SamplerState");
    }

    // ----------------------------------------------------
    // RECURSOS COMENTADOS (RASTER, BLEND, SHADOWS)
    // (Pendientes para futuras etapas del motor de iluminación)
    // ----------------------------------------------------

    //hr = m_rasterizer.init(device);
    //hr = m_blendstate.init(device);
    //hr = m_shaderShadow.CreateShader(device, PIXEL_SHADER, "HybridEngine.fx");
    //hr = m_shaderBuffer.init(device, sizeof(CBChangesEveryFrame));
    //hr = m_shadowBlendState.init(device);
    //hr = m_shadowDepthStencilState.init(device, true, false);
    //m_LightPos = XMFLOAT4(2.0f, 4.0f, -2.0f, 1.0f);
}

void
Actor::update(float deltaTime, DeviceContext& deviceContext) {
    // 1. Actualizar lógica de todos los componentes adheridos (Ej. Físicas, Animaciones)
    for (auto& component : m_components) {
        if (component) {
            component->update(deltaTime);
        }
    }

    // 2. Actualizar los datos del Constant Buffer en la RAM (Matriz de transformación)
    m_model.mWorld = XMMatrixTranspose(getComponent<Transform>()->matrix);
    m_model.vMeshColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

    // 3. Enviar la matriz actualizada a la memoria de video (VRAM)
    m_modelBuffer.update(deviceContext, nullptr, 0, nullptr, &m_model, 0, 0);
}

void 
Actor::render(DeviceContext& deviceContext) {
    // Activa el filtrado de texturas
    m_sampler.render(deviceContext, 0, 1);

    // Indica a DirectX que la topología es una lista de triángulos conectados
    deviceContext.IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Dibuja cada sub-malla (Mesh) que compone a este Actor
    for (unsigned int i = 0; i < m_meshes.size(); i++)
    {
        // A) Enviar Geometría y Matrices
        m_vertexBuffers[i].render(deviceContext, 0, 1);
        m_indexBuffers[i].render(deviceContext, 0, 1, false, DXGI_FORMAT_R32_UINT);

        // Slot 2 para el Constant Buffer de modelo (Coincide con HeliosEngine.fx)
        m_modelBuffer.render(deviceContext, 2, 1, true);

        // B) Limpiar texturas de pasadas anteriores por seguridad
        ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
        deviceContext.m_deviceContext->PSSetShaderResources(0, 1, nullSRV);

        // C) Enviar Material a la GPU
        // MODIFICACIÓN: En lugar de limitar la textura según el número de malla,
        // forzamos a que TODAS las mallas de este actor usen la lista de texturas cargada.
        for (int k = 0; k < m_textures.size(); k++) {
            m_textures[k].render(deviceContext, k, 1);
        }

        // D) Comando final: Dibujar los píxeles
        deviceContext.DrawIndexed(m_meshes[i].m_numIndex, 0, 0);
    }
}

// Método optimizado específicamente para dibujar la geometría del entorno (Skybox)
// sin afectar ni usar el pipeline de texturas PBR estándar.
void
Actor::renderForSkybox(DeviceContext& deviceContext) {
    deviceContext.IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Renderiza la geometría del cubo
    for (unsigned int i = 0; i < m_meshes.size(); i++) {
        m_vertexBuffers[i].render(deviceContext, 0, 1);
        m_indexBuffers[i].render(deviceContext, 0, 1, false, DXGI_FORMAT_R32_UINT);

        // Ejecuta la orden de dibujo
        deviceContext.DrawIndexed(m_meshes[i].m_numIndex, 0, 0);
    }
}

void
Actor::destroy() {
    for (auto& vertexBuffer : m_vertexBuffers) {
        vertexBuffer.destroy();
    }
    for (auto& indexBuffer : m_indexBuffers) {
        indexBuffer.destroy();
    }
    for (auto& tex : m_textures) {
        tex.destroy();
    }

    m_modelBuffer.destroy();
    m_sampler.destroy();

    //m_rasterizer.destroy();
    //m_blendstate.destroy();
}

void
Actor::setMesh(Device& device, std::vector<MeshComponent> meshes) {
    m_meshes = meshes;
    HRESULT hr;

    for (auto& mesh : m_meshes) {
        // Crear Vertex Buffer en la GPU
        Buffer vertexBuffer;
        hr = vertexBuffer.init(device, mesh, D3D11_BIND_VERTEX_BUFFER);
        if (FAILED(hr)) {
            ERROR("Actor", "setMesh", "Failed to create new vertexBuffer");
        }
        else {
            m_vertexBuffers.push_back(vertexBuffer);
        }

        // Crear Index Buffer en la GPU
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