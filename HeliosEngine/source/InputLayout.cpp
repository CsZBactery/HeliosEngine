#include "../include/InputLayout.h"
#include "../include/Device.h"
#include "../include/DeviceContext.h"

HRESULT
InputLayout::init(Device& device,
	std::vector<D3D11_INPUT_ELEMENT_DESC>& Layout,
	ID3DBlob* VertexShaderData) {

	// Validamos que nos pasen datos reales. 
	// Sin el blob (bytecode) del Vertex Shader no podemos validar si el Layout es correcto.
	if (Layout.empty()) {
		ERROR("InputLayout", "init", "Vector de Layout vacio");
		return E_INVALIDARG;
	}
	if (!VertexShaderData) {
		ERROR("InputLayout", "init", "VertexShaderData nulo (necesario para validar la firma)");
		return E_POINTER;
	}

	// Aqui ocurre la validacion cruzada: DirectX compara tu array de descripciones de C++ (Layout) 
	// con el "Input Signature" compilado dentro del Vertex Shader. Si los tipos o semanticas no coinciden, falla.
	HRESULT hr = device.CreateInputLayout(Layout.data(),
		static_cast<unsigned int>(Layout.size()),
		VertexShaderData->GetBufferPointer(), // Puntero al inicio del bytecode compilado
		VertexShaderData->GetBufferSize(),    // Longitud del bytecode
		&m_inputLayout);

	if (FAILED(hr)) {
		ERROR("InputLayout", "init",
			("Fallo al crear InputLayout. HRESULT: " + std::to_string(hr)).c_str());
		return hr;
	}

	return S_OK;
}

void
InputLayout::update() {
	// Por ahora no necesitamos actualizar el layout en tiempo real.
	// Esto se usaria si cambiaramos dinamicamente el formato de vertice (ej: activar/desactivar blend weights).
}

void
InputLayout::render(DeviceContext& deviceContext) {
	if (!m_inputLayout) {
		ERROR("InputLayout", "render", "InputLayout nulo, no se puede bindear");
		return;
	}

	// Le decimos a la IA (Input Assembler) de la GPU: "Interpreta los bytes del Vertex Buffer segun este plano"
	deviceContext.m_deviceContext->IASetInputLayout(m_inputLayout);
}

void
InputLayout::destroy() {
	SAFE_RELEASE(m_inputLayout);
}