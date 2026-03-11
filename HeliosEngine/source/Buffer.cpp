// ======================================================================================
// Archivo: Buffer.cpp
// Implementación del gestor de memoria en VRAM (Vértices, Índices y Constantes).
// ======================================================================================

#include "Buffer.h"
#include "Device.h"
#include "DeviceContext.h"

// ======================================================================================
// Inicialización para Vértices e Índices
// ======================================================================================
HRESULT
Buffer::init(Device& device, const MeshComponent& mesh, unsigned int bindFlag) {
    // Validaciones básicas de seguridad
    if (!device.m_device) {
        ERROR("Buffer", "init", "Device is null."); // Corregida etiqueta
        return E_POINTER;
    }

    // Si pedimos Vertex Buffer, asegurarnos que la malla tenga vértices normales O vértices de Skybox
    if ((bindFlag & D3D11_BIND_VERTEX_BUFFER) && mesh.m_vertex.empty() && mesh.m_skyVertex.empty()) {
        ERROR("Buffer", "init", "Vertex buffer is empty");
        return E_INVALIDARG;
    }
    // Si pedimos Index Buffer, asegurarnos que la malla tenga índices
    if ((bindFlag & D3D11_BIND_INDEX_BUFFER) && mesh.m_index.empty()) {
        ERROR("Buffer", "init", "Index buffer is empty");
        return E_INVALIDARG;
    }

    D3D11_BUFFER_DESC desc = {};
    D3D11_SUBRESOURCE_DATA data = {};

    // Configuración por defecto: Memoria GPU (Default), sin acceso directo desde CPU
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.CPUAccessFlags = 0;
    m_bindFlag = bindFlag;
    desc.BindFlags = (D3D11_BIND_FLAG)bindFlag;

    // Calculamos el tamaño total del buffer basándonos en el tipo solicitado
    if (bindFlag & D3D11_BIND_VERTEX_BUFFER) {

        // --- SOPORTE PARA SKYBOX ---
        // Verificamos si la malla tiene vértices de Skybox en lugar de vértices normales
        if (mesh.m_skyVertex.size() > 0 && mesh.m_vertex.size() == 0) {
            m_stride = sizeof(SkyboxVertex);
            desc.ByteWidth = m_stride * static_cast<unsigned int>(mesh.m_skyVertex.size());
            data.pSysMem = mesh.m_skyVertex.data();
        }
        else {
            m_stride = sizeof(SimpleVertex); // Importante para que la GPU sepa cuánto "camina" por vértice
            desc.ByteWidth = m_stride * static_cast<unsigned int>(mesh.m_vertex.size());
            data.pSysMem = mesh.m_vertex.data(); // Puntero a los datos crudos en RAM
        }
    }
    else if (bindFlag & D3D11_BIND_INDEX_BUFFER) {
        m_stride = sizeof(unsigned int);
        desc.ByteWidth = m_stride * static_cast<unsigned int>(mesh.m_index.size());
        desc.BindFlags = (D3D11_BIND_FLAG)bindFlag;
        data.pSysMem = mesh.m_index.data();
    }

    // Llamamos a la API interna para crear el recurso en DirectX
    return createBuffer(device, desc, &data);
}

// ======================================================================================
// Inicialización para Constant Buffers (Buffers de tamaño fijo sin datos iniciales)
// ======================================================================================
HRESULT
Buffer::init(Device& device, unsigned int ByteWidth) {
    // Sobrecarga específica para CONSTANT BUFFERS
    // Estos buffers (matrices, luces) suelen ser dinámicos y pequeños

    if (!device.m_device) {
        ERROR("Buffer", "init", "Device is null."); // Corregida etiqueta
        return E_POINTER;
    }
    if (ByteWidth == 0) {
        ERROR("Buffer", "init", "ByteWidth is zero");
        return E_INVALIDARG;
    }
    m_stride = ByteWidth;

    D3D11_BUFFER_DESC desc = {};
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.ByteWidth = ByteWidth; // Ojo: DEBE ser múltiplo de 16 bytes según las reglas de HLSL
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    m_bindFlag = desc.BindFlags;

    // Pasamos nullptr en data porque se actualizará más tarde en cada frame con update()
    return createBuffer(device, desc, nullptr);
}

// ======================================================================================
// Envío de datos desde la CPU (RAM) hacia la GPU (VRAM)
// ======================================================================================
void
Buffer::update(DeviceContext& deviceContext,
    ID3D11Resource* pDstResource,
    unsigned int DstSubresource,
    const D3D11_BOX* pDstBox,
    const void* pSrcData,
    unsigned int SrcRowPitch,
    unsigned int SrcDepthPitch) {

    // Validaciones
    if (!m_buffer) {
        ERROR("Buffer", "update", "m_buffer is null."); // Corregida etiqueta
        return;
    }
    if (!pSrcData) {
        ERROR("Buffer", "update", "pSrcData is null."); // Corregida etiqueta
        return;
    }

    // Función clave para enviar datos de CPU a GPU.
    // Se usa muchísimo para actualizar matrices de mundo/vista/proyección en tiempo real
    deviceContext.m_deviceContext->UpdateSubresource(m_buffer,
        DstSubresource,
        pDstBox,
        pSrcData,
        SrcRowPitch,
        SrcDepthPitch);
}

// ======================================================================================
// Vinculación (Binding) del buffer al pipeline gráfico
// ======================================================================================
void
Buffer::render(DeviceContext& deviceContext,
    unsigned int StartSlot,
    unsigned int NumBuffers,
    bool setPixelShader,
    DXGI_FORMAT format) {

    if (!deviceContext.m_deviceContext) {
        ERROR("Buffer", "render", "DeviceContext is nullptr."); // Corregida etiqueta
        return;
    }
    if (!m_buffer) {
        ERROR("Buffer", "render", "m_buffer is null.");
        return;
    }

    // Dependiendo del tipo de buffer, lo "enchufamos" en una etapa distinta del pipeline
    switch (m_bindFlag) {
    case D3D11_BIND_VERTEX_BUFFER:
        // Input Assembler: Aquí le decimos a la GPU "estos son los vértices a dibujar"
        deviceContext.m_deviceContext->IASetVertexBuffers(StartSlot, NumBuffers, &m_buffer, &m_stride, &m_offset);
        break;

    case D3D11_BIND_CONSTANT_BUFFER:
        // Vertex Shader: Matrices y datos globales de geometría
        deviceContext.m_deviceContext->VSSetConstantBuffers(StartSlot, NumBuffers, &m_buffer);

        // Opcionalmente, también lo enviamos al Pixel Shader (ej: datos de luces, colores de material PBR)
        if (setPixelShader) {
            deviceContext.m_deviceContext->PSSetConstantBuffers(StartSlot, NumBuffers, &m_buffer);
        }
        break;

    case D3D11_BIND_INDEX_BUFFER:
        // Input Assembler: Definimos el orden de dibujo (índices) para reutilizar vértices compartidos
        deviceContext.m_deviceContext->IASetIndexBuffer(m_buffer, format, m_offset);
        break;

    default:
        ERROR("Buffer", "render", "Unsupported BindFlag");
        break;
    }
}

// ======================================================================================
// Liberación de memoria
// ======================================================================================
void
Buffer::destroy() {
    // Macro segura para liberar la interfaz COM de DirectX
    SAFE_RELEASE(m_buffer);
}

// ======================================================================================
// Función Helper Interna: Llama a la API de creación nativa de DirectX
// ======================================================================================
HRESULT
Buffer::createBuffer(Device& device,
    D3D11_BUFFER_DESC& desc,
    D3D11_SUBRESOURCE_DATA* initData) {

    if (!device.m_device) {
        ERROR("Buffer", "createBuffer", "Device is nullptr");
        return E_POINTER;
    }

    // Llamada nativa de DirectX 11 para reservar la memoria en la tarjeta gráfica
    HRESULT hr = device.CreateBuffer(&desc, initData, &m_buffer);
    if (FAILED(hr)) {
        ERROR("Buffer", "createBuffer", "Failed to create buffer");
        return hr;
    }
    return S_OK;
}