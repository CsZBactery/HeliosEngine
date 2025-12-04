#include "../include/ECS/Actor.h"
#include "../include/MeshComponent.h"
#include "../include/Device.h"
#include "../include/DeviceContext.h"

#include "../include/ECS/Transform.h" 

Actor::Actor(Device& device) {
    // Setup Default Components
    EU::TSharedPointer<Transform> transform = EU::MakeShared<Transform>();
    addComponent(transform);
    EU::TSharedPointer<MeshComponent> meshComponent = EU::MakeShared<MeshComponent>();
    addComponent(meshComponent);

    HRESULT hr;
    std::string classNameType = "Actor -> " + m_name;

    hr = m_modelBuffer.init(device, sizeof(CBChangesEveryFrame));
    if (FAILED(hr)) {
        ERROR("Actor", classNameType.c_str(), "Failed to create new CBChangesEveryFrame");
    }

    hr = m_sampler.init(device);
    if (FAILED(hr)) {
        ERROR("Actor", classNameType.c_str(), "Failed to create new SamplerState");
    }


    //hr = m_rasterizer.init(device);
    // ... (resto del c�digo comentado del profesor)
    //m_LightPos = XMFLOAT4(2.0f, 4.0f, -2.0f, 1.0f);
}

void
Actor::update(float deltaTime, DeviceContext& deviceContext) {
    // Update all components
    for (auto& component : m_components) {
        if (component) {
            component->update(deltaTime);
        }
    }

    // Update the model buffer
    // Requiere #include "Transform.h" arriba
    m_model.mWorld = XMMatrixTranspose(getComponent<Transform>()->matrix);

    m_model.vMeshColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

    // Update the constant buffer
    m_modelBuffer.update(deviceContext, nullptr, 0, nullptr, &m_model, 0, 0);
}

void
Actor::render(DeviceContext& deviceContext) {
    // 1) Proyectar sombra primero (sobre el suelo)
    //if (canCastShadow()) {
    //    renderShadow(deviceContext);
    //}

    // 2) Estados de raster, blend y sampler para el modelo
    m_sampler.render(deviceContext, 0, 1);

    deviceContext.IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Update buffer and render all components
    for (unsigned int i = 0; i < m_meshes.size(); i++) {
        m_vertexBuffers[i].render(deviceContext, 0, 1);
        m_indexBuffers[i].render(deviceContext, 0, 1, false, DXGI_FORMAT_R32_UINT);

        // Bind del CB �normal� (world + color)
        m_modelBuffer.render(deviceContext, 2, 1, true);

        // Render mesh texture
        // L�GICA DEL PROFESOR (PBR): Solo renderiza si tienes el set completo de 5 texturas
        if (m_textures.size() > 0) {
            if (i < m_textures.size()) {
                if (m_textures.size() >= 1) {
                    m_textures[0].render(deviceContext, 0, 1); // Albedo -> t0
                    //m_textures[1].render(deviceContext, 1, 1); // Normal -> t1
                    // ... etc
                }
            }
        }
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

    //m_rasterizer.destroy();
    //m_blendstate.destroy();
    m_sampler.destroy();
}

void
Actor::setMesh(Device& device, std::vector<MeshComponent> meshes) {
    // NOTA IMPORTANTE: En el script de tu profesor esto sal�a vac�o.
    // Pero necesitas este c�digo para crear los VertexBuffers, si no, no se ve nada.

    m_meshes = meshes;
    HRESULT hr;
    for (auto& mesh : m_meshes) {

        Buffer vertexBuffer;
        hr = vertexBuffer.init(device, mesh, D3D11_BIND_VERTEX_BUFFER);
        if (FAILED(hr)) {
            ERROR("Actor", "setMesh", "Failed to create new vertexBuffer");
        }
        else {
            m_vertexBuffers.push_back(vertexBuffer);
        }

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