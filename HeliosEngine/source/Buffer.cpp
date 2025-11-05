#include "../include/Buffer.h"
#include "../include/Device.h"
#include "../include/DeviceContext.h"


HRESULT
Buffer::init(Device& device, const MeshComponent& mesh, unsigned int bindFlag) {
	// Valida que el dispositivo exista
	if (!device.m_device) {
		ERROR("ShaderProgram", "init", "Device is null.");
		return E_POINTER;
	}
	// Si es VB, debe haber vértices
	if ((bindFlag & D3D11_BIND_VERTEX_BUFFER) && mesh.m_vertex.empty()) {
		ERROR("Buffer", "init", "Vertex buffer is empty");
		return E_INVALIDARG;
	}
	// Si es IB, debe haber índices
	if ((bindFlag & D3D11_BIND_INDEX_BUFFER) && mesh.m_index.empty()) {
		ERROR("Buffer", "init", "Index buffer is empty");
		return E_INVALIDARG;
	}

	// Descriptores base para crear el buffer
	D3D11_BUFFER_DESC desc = {};
	D3D11_SUBRESOURCE_DATA data = {};

	desc.Usage = D3D11_USAGE_DEFAULT;     // GPU read / GPU write por comandos
	desc.CPUAccessFlags = 0;               // CPU no escribe directamente
	m_bindFlag = bindFlag;                 // Guardar tipo de enlace (VB/IB/CB)

	// Configura tamaño/stride y datos iniciales según el tipo de buffer
	if (bindFlag & D3D11_BIND_VERTEX_BUFFER) {
		m_stride = sizeof(SimpleVertex);
		desc.ByteWidth = m_stride * static_cast<unsigned int>(mesh.m_vertex.size());
		desc.BindFlags = (D3D11_BIND_FLAG)bindFlag;
		data.pSysMem = mesh.m_vertex.data();  // datos de vértices
	}
	else if (bindFlag & D3D11_BIND_INDEX_BUFFER) {
		m_stride = sizeof(unsigned int);
		desc.ByteWidth = m_stride * static_cast<unsigned int>(mesh.m_index.size());
		desc.BindFlags = (D3D11_BIND_FLAG)bindFlag;
		data.pSysMem = mesh.m_index.data();   // datos de índices
	}

	// Crea el buffer en GPU
	return createBuffer(device, desc, &data);
}

HRESULT
Buffer::init(Device& device, unsigned int ByteWidth) {
	// Valida device
	if (!device.m_device) {
		ERROR("ShaderProgram", "init", "Device is null.");
		return E_POINTER;
	}
	// Evita tamaños inválidos
	if (ByteWidth == 0) {
		ERROR("Buffer", "init", "ByteWidth is zero");
		return E_INVALIDARG;
	}
	m_stride = ByteWidth; // Para CB no se usa como stride de vértice, sólo referencia

	// Descripción de Constant Buffer (sin datos iniciales)
	D3D11_BUFFER_DESC desc = {};
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.ByteWidth = ByteWidth;                 // Debe ser múltiplo de 16 bytes
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	m_bindFlag = desc.BindFlags;

	// Crear CB vacío (se actualizará con UpdateSubresource)
	return createBuffer(device, desc, nullptr);
}

void
Buffer::update(DeviceContext& deviceContext,
	ID3D11Resource* pDstResource,
	unsigned int DstSubresource,
	const D3D11_BOX* pDstBox,
	const void* pSrcData,
	unsigned int SrcRowPitch,
	unsigned int SrcDepthPitch) {
	// Verifica que el buffer exista
	if (!m_buffer) {
		ERROR("ShaderProgram", "update", "m_buffer is null.");
		return;
	}
	// Verifica datos de origen válidos
	if (!pSrcData) {
		ERROR("ShaderProgram", "update", "pSrcData is null.");
		return;
	}
	// Sube datos al recurso (usa el propio m_buffer como destino)
	deviceContext.m_deviceContext->UpdateSubresource(m_buffer,
		DstSubresource,
		pDstBox,
		pSrcData,
		SrcRowPitch,
		SrcDepthPitch);


}

void
Buffer::render(DeviceContext& deviceContext,
	unsigned int StartSlot,
	unsigned int NumBuffers,
	bool setPixelShader,
	DXGI_FORMAT format) {
	// Validaciones básicas
	if (!deviceContext.m_deviceContext) {
		ERROR("RenderTargetView", "render", "DeviceContext is nullptr.");
		return;
	}
	if (!m_buffer) {
		ERROR("Buffer", "render", "m_buffer is null.");
		return;
	}

	// Enlaza el buffer según su tipo (VB/IB/CB)
	switch (m_bindFlag) {
	case D3D11_BIND_VERTEX_BUFFER:
		// Asigna VB al IA (con stride y offset internos)
		deviceContext.m_deviceContext->IASetVertexBuffers(StartSlot, NumBuffers, &m_buffer, &m_stride, &m_offset);
		break;
	case D3D11_BIND_CONSTANT_BUFFER:
		// Enlaza CB al VS y opcionalmente al PS
		deviceContext.m_deviceContext->VSSetConstantBuffers(StartSlot, NumBuffers, &m_buffer);
		if (setPixelShader) {
			deviceContext.m_deviceContext->PSSetConstantBuffers(StartSlot, NumBuffers, &m_buffer);
		}
		break;
	case D3D11_BIND_INDEX_BUFFER:
		// Asigna IB al IA con formato (R16/R32) y offset
		deviceContext.m_deviceContext->IASetIndexBuffer(m_buffer, format, m_offset);
		break;
	default:
		// Tipo de bind no soportado por este método
		ERROR("Buffer", "render", "Unsupported BindFlag");
		break;
	}
}

void
Buffer::destroy() {
	// Libera recurso GPU y pone a nullptr
	SAFE_RELEASE(m_buffer);
}

HRESULT
Buffer::createBuffer(Device& device,
	D3D11_BUFFER_DESC& desc,
	D3D11_SUBRESOURCE_DATA* initData) {
	// Valida device antes de crear
	if (!device.m_device) {
		ERROR("Buffer", "createBuffer", "Device is nullptr");
		return E_POINTER;
	}

	// Crea el ID3D11Buffer con los parámetros dados
	HRESULT hr = device.CreateBuffer(&desc, initData, &m_buffer);
	if (FAILED(hr)) {
		ERROR("Buffer", "createBuffer", "Failed to create buffer");
		return hr;
	}
	return S_OK;
}
