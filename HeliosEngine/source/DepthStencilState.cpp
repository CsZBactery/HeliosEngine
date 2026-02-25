// ======================================================================================
// Archivo: DepthStencilState.cpp
// Implementación de las reglas de Profundidad (Z-Buffer) y Máscaras (Stencil).
// ======================================================================================

#include "DepthStencilState.h"
#include "Device.h"
#include "DeviceContext.h"

// Inicializa las reglas lógicas en la memoria de la tarjeta gráfica
HRESULT
DepthStencilState::init(Device& device, bool enableDepth, bool enableStencil) {
	// Verificamos que la tarjeta de video esté conectada y lista
	if (!device.m_device) {
		ERROR("ShaderProgram", "init", "Device is null.");
		return E_POINTER;
	}

	// Estructura que le dirá a DirectX cómo queremos que se comporten los píxeles
	D3D11_DEPTH_STENCIL_DESC desc = {};

	// --- CONFIGURACIÓN DE PROFUNDIDAD (Z-BUFFER) ---
	// Activa o desactiva la prueba de distancia (si un objeto debe tapar a otro)
	desc.DepthEnable = enableDepth;

	// Siempre permitimos que se escriba la nueva información de profundidad en el buffer
	desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;

	// Regla matemática: Dibuja el píxel solo si su distancia es "MENOR" (está más cerca de la cámara)
	desc.DepthFunc = D3D11_COMPARISON_LESS;

	// --- CONFIGURACIÓN DE ESTARCIDO (STENCIL) ---
	// El Stencil se usa para crear máscaras de recorte, siluetas o sombras complejas
	desc.StencilEnable = enableStencil;
	desc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	desc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;

	// Operaciones para los polígonos que miran hacia la cámara (Front Face)
	desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	// Si falla la prueba de profundidad (hay un objeto delante), incrementamos el valor del stencil.
	// (Esta es una técnica muy común para el algoritmo de Sombras Volumétricas).
	desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Operaciones para los polígonos que miran en dirección contraria (Back Face)
	desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	// Si falla la prueba de profundidad por detrás, decrementamos el valor.
	desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	desc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Finalmente, enviamos esta estructura a la GPU para que cree el estado lógico
	HRESULT hr = device.m_device->CreateDepthStencilState(&desc, &m_depthStencilState);
	if (FAILED(hr)) {
		ERROR("DepthStencilState", "init", "Failed to create DepthStencilState");
	}

	return S_OK;
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