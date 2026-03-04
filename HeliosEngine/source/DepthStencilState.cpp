// ======================================================================================
// Archivo: DepthStencilState.cpp
// Implementación de las reglas de Profundidad (Z-Buffer) y Máscaras (Stencil).
// ======================================================================================

#include "DepthStencilState.h"
#include "Device.h"
#include "DeviceContext.h"

// Inicializa las reglas lógicas en la memoria de la tarjeta gráfica
HRESULT
DepthStencilState::init(Device& device,
	bool depthEnable,
	D3D11_DEPTH_WRITE_MASK writeMask,
	D3D11_COMPARISON_FUNC depthFunc) {

	// Verificamos que la tarjeta de video esté conectada y lista
	if (!device.m_device) {
		ERROR("ShaderProgram", "init", "Device is null.");
		return E_POINTER;
	}

	// Estructura que le dirá a DirectX cómo queremos que se comporten los píxeles.
	// Al usar {} se inicializa toda la memoria en ceros automáticamente.
	D3D11_DEPTH_STENCIL_DESC desc{};

	// --- CONFIGURACIÓN DE PROFUNDIDAD (Z-BUFFER) ---
	// Asignamos las variables que recibimos por parámetro (ej. para el Skybox)
	desc.DepthEnable = depthEnable;
	desc.DepthWriteMask = writeMask;
	desc.DepthFunc = depthFunc;

	// --- CONFIGURACIÓN DE ESTARCIDO (STENCIL) ---
	// Desactivado por ahora según la arquitectura del motor base
	desc.StencilEnable = false;

	// Finalmente, enviamos esta estructura a la GPU para que cree el estado lógico
	HRESULT hr = device.m_device->CreateDepthStencilState(&desc, &m_depthStencilState);
	if (FAILED(hr)) {
		ERROR("DepthStencilState", "init", "Failed to create DepthStencilState");
		return hr;
	}

	return hr;
}

// Espacio reservado para futuras actualizaciones dinámicas
void
DepthStencilState::update() {
	// Actualmente vacío, el estado suele ser estático por frame
}

// Inyecta este estado de reglas en el pipeline gráfico activo (Output-Merger)
void
DepthStencilState::render(DeviceContext& deviceContext,
	unsigned int stencilRef,
	bool reset) {

	// Validaciones de seguridad antes de hablar con la GPU
	if (!deviceContext.m_deviceContext) {
		ERROR("RenderTargetView", "render", "DeviceContext is nullptr.");
		return;
	}
	if (!m_depthStencilState) {
		ERROR("DepthStencilState", "render", "DepthStencilState is nullptr");
		return;
	}

	// Si pedimos no resetear, aplicamos nuestras reglas creadas en el init()
	if (!reset) {
		deviceContext.m_deviceContext->OMSetDepthStencilState(m_depthStencilState, stencilRef);
	}
	// Si reset es true, limpiamos las reglas (volvemos al comportamiento por defecto de DirectX)
	else {
		deviceContext.m_deviceContext->OMSetDepthStencilState(nullptr, stencilRef);
	}
}

// Limpia la memoria de la tarjeta gráfica de forma segura al cerrar el motor
void
DepthStencilState::destroy() {
	// Utilizamos nuestra macro para hacer un Release() y poner el puntero en nullptr
	SAFE_RELEASE(m_depthStencilState);
}