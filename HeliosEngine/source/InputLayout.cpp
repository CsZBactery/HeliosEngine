// ======================================================================================
// Archivo: InputLayout.cpp
// Implementación de la definición de formato de vértices para el Input Assembler.
// ======================================================================================

#include "InputLayout.h"
#include "Device.h"
#include "DeviceContext.h"

// ======================================================================================
// Sobrecarga del Profesor: Utiliza punteros crudos y un contador (Estilo nativo DirectX)
// ======================================================================================
HRESULT
InputLayout::init(Device& device,
    const D3D11_INPUT_ELEMENT_DESC* layoutDesc,
    UINT layoutCount,
    ID3DBlob* vertexShaderData) {

    // Validamos que nos pasen datos reales. 
    if (!layoutDesc || layoutCount == 0) {
        ERROR("InputLayout", "init", "Layout descriptor is empty.");
        return E_INVALIDARG;
    }

    // Sin el blob (bytecode) del Vertex Shader no podemos validar si el Layout es correcto.
    if (!vertexShaderData) {
        ERROR("InputLayout", "init", "VertexShaderData nulo (necesario para validar la firma)");
        return E_POINTER;
    }

    // Aquí ocurre la validación cruzada: DirectX compara el array de descripciones de C++ 
    // con el "Input Signature" compilado dentro del Vertex Shader. Si los tipos o 
    // semánticas (ej. POSITION, TEXCOORD) no coinciden, esta función fallará.
    HRESULT hr = device.m_device->CreateInputLayout(
        layoutDesc,
        layoutCount,
        vertexShaderData->GetBufferPointer(), // Puntero al inicio del bytecode compilado
        vertexShaderData->GetBufferSize(),    // Longitud del bytecode
        &m_inputLayout
    );

    if (FAILED(hr)) {
        ERROR("InputLayout", "init", ("Failed to create InputLayout. HRESULT: " + std::to_string(hr)).c_str());
        return hr;
    }

    return S_OK;
}

// ======================================================================================
// Sobrecarga Tuya: Utiliza std::vector (Más seguro y fácil de usar en el motor)
// ======================================================================================
HRESULT
InputLayout::init(Device& device,
    std::vector<D3D11_INPUT_ELEMENT_DESC>& Layout,
    ID3DBlob* VertexShaderData) {

    if (Layout.empty()) {
        ERROR("InputLayout", "init", "Vector de Layout vacío");
        return E_INVALIDARG;
    }

    // Reutilizamos la lógica de la función anterior para no duplicar código, 
    // extrayendo el puntero crudo (.data()) y el tamaño (.size()) del vector.
    return init(device, Layout.data(), static_cast<UINT>(Layout.size()), VertexShaderData);
}

// ======================================================================================
// Actualización dinámica
// ======================================================================================
void
InputLayout::update() {
    // Por ahora no necesitamos actualizar el layout en tiempo real.
    // Esto se usaría si cambiáramos dinámicamente el formato de vértice 
    // (ej: activar/desactivar blend weights para animaciones esqueléticas).
}

// ======================================================================================
// Vinculación al pipeline gráfico
// ======================================================================================
void
InputLayout::render(DeviceContext& deviceContext) {
    if (!m_inputLayout) {
        ERROR("InputLayout", "render", "InputLayout nulo, no se puede bindear");
        return;
    }

    // Le decimos a la etapa IA (Input Assembler) de la GPU: 
    // "Interpreta los bytes del Vertex Buffer según este plano maestro"
    deviceContext.m_deviceContext->IASetInputLayout(m_inputLayout);
}

// ======================================================================================
// Liberación de memoria
// ======================================================================================
void
InputLayout::destroy() {
    SAFE_RELEASE(m_inputLayout);
}