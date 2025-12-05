#include "../include/ECS/Actor.h"
#include "../include/MeshComponent.h"
#include "../include/Device.h"
#include "../include/DeviceContext.h"
#include "../include/ECS/Transform.h" 

Actor::Actor(Device& device) {
    // Agregamos los componentes base para que el actor sea funcional desde el inicio
    EU::TSharedPointer<Transform> transform = EU::MakeShared<Transform>();
    addComponent(transform);

    EU::TSharedPointer<MeshComponent> meshComponent = EU::MakeShared<MeshComponent>();
    addComponent(meshComponent);

    HRESULT hr;
    std::string classNameType = "Actor -> " + m_name;

    // Inicializamos el Constant Buffer para enviar matrices al shader por frame
    hr = m_modelBuffer.init(device, sizeof(CBChangesEveryFrame));
    if (FAILED(hr)) {
        ERROR("Actor", classNameType.c_str(), "Error al crear CBChangesEveryFrame");
    }

    // Preparamos el Sampler para poder mapear texturas mas adelante
    hr = m_sampler.init(device);
    if (FAILED(hr)) {
        ERROR("Actor", classNameType.c_str(), "Error al crear SamplerState");
    }

    // Configuraciones opcionales (rasterizer, luces) quedaron pendientes
}

void
Actor::update(float deltaTime, DeviceContext& deviceContext) {
    // Ciclo principal: actualizamos la logica de todos los componentes hijos
    for (auto& component : m_components) {
        if (component) {
            component->update(deltaTime);
        }
    }

    // Obtenemos la matriz de mundo actualizada desde el Transform
    // Transponemos la matriz porque HLSL espera orden por columnas
    m_model.mWorld = XMMatrixTranspose(getComponent<Transform>()->matrix);

    // Color base por defecto (blanco)
    m_model.vMeshColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

    // Enviamos los nuevos datos de la matriz a la GPU
    m_modelBuffer.update(deviceContext, nullptr, 0, nullptr, &m_model, 0, 0);
}

void
Actor::render(DeviceContext& deviceContext) {
    // Configuramos como se leen las texturas
    m_sampler.render(deviceContext, 0, 1);

    // Indicamos que vamos a dibujar triangulos
    deviceContext.IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Iteramos sobre las mallas cargadas para dibujarlas
    for (unsigned int i = 0; i < m_meshes.size(); i++) {
        // Vinculamos los buffers de vertices e indices al pipeline
        m_vertexBuffers[i].render(deviceContext, 0, 1);
        m_indexBuffers[i].render(deviceContext, 0, 1, false, DXGI_FORMAT_R32_UINT);

        // Pasamos la matriz de transformacion al Vertex Shader (slot 2)
        m_modelBuffer.render(deviceContext, 2, 1, true);

        // Si el actor tiene texturas, vinculamos la primera al slot t0
        // Nota: Esto asume un material simple con solo Albedo por ahora
        if (m_textures.size() > 0) {
            if (i < m_textures.size()) {
                if (m_textures.size() >= 1) {
                    m_textures[0].render(deviceContext, 0, 1);
                }
            }
        }

        // Llamada final de dibujo
        deviceContext.DrawIndexed(m_meshes[i].m_numIndex, 0, 0);
    }
}

void
Actor::destroy() {
    // Liberar memoria de los buffers de geometria
    for (auto& vertexBuffer : m_vertexBuffers) {
        vertexBuffer.destroy();
    }

    for (auto& indexBuffer : m_indexBuffers) {
        indexBuffer.destroy();
    }

    // Liberar texturas y buffers de constantes
    for (auto& tex : m_textures) {
        tex.destroy();
    }
    m_modelBuffer.destroy();
    m_sampler.destroy();
}

void
Actor::setMesh(Device& device, std::vector<MeshComponent> meshes) {
    // Guardamos los datos de la malla en CPU
    m_meshes = meshes;
    HRESULT hr;

    // Generamos los buffers reales en GPU para cada sub-malla
    for (auto& mesh : m_meshes) {

        // Crear Vertex Buffer
        Buffer vertexBuffer;
        hr = vertexBuffer.init(device, mesh, D3D11_BIND_VERTEX_BUFFER);
        if (FAILED(hr)) {
            ERROR("Actor", "setMesh", "Fallo al crear vertexBuffer");
        }
        else {
            m_vertexBuffers.push_back(vertexBuffer);
        }

        // Crear Index Buffer
        Buffer indexBuffer;
        hr = indexBuffer.init(device, mesh, D3D11_BIND_INDEX_BUFFER);
        if (FAILED(hr)) {
            ERROR("Actor", "setMesh", "Fallo al crear indexBuffer");
        }
        else {
            m_indexBuffers.push_back(indexBuffer);
        }
    }
}