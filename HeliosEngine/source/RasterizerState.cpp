// ======================================================================================
// Archivo: RasterizerState.cpp
// Implementación de las reglas para la conversión de geometría 3D a píxeles 2D.
// ======================================================================================

#include "RasterizerState.h"
#include "Device.h"
#include "DeviceContext.h"

// Inicializa el rasterizador con la configuración por defecto (Sólido y Culling trasero)
HRESULT
RasterizerState::init(Device device) {
	// Estructura que define cómo se "rellenan" los triángulos en la pantalla
	D3D11_RASTERIZER_DESC rasterizerDesc = {};

	// Dibuja el interior del polígono (si fuera D3D11_FILL_WIREFRAME, solo veríamos líneas)
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;

	// Optimización principal: No dibuja las caras traseras de los objetos.
	// (Ej. No dibuja el interior de una caja cerrada porque el jugador nunca lo verá).
	rasterizerDesc.CullMode = D3D11_CULL_BACK;

	// Define qué cara es el "Frente". En DirectX, el frente suele ser sentido horario (Clockwise).
	rasterizerDesc.FrontCounterClockwise = false;

	// Estos parámetros (Bias) se usan principalmente para evitar artefactos visuales
	// cuando se dibujan sombras (Shadow Acne). Por ahora los dejamos en cero.
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.SlopeScaledDepthBias = 0.0f;
	rasterizerDesc.DepthBiasClamp = 0.0f;

	// Permite recortar los píxeles que están más allá de la cámara (Far Plane)
	rasterizerDesc.DepthClipEnable = true;

	// Scissor permite renderizar solo dentro de un rectángulo específico de la pantalla. (Apagado por defecto)
	rasterizerDesc.ScissorEnable = false;

	// Apagamos el Multisampling y Anti-aliasing de líneas en esta configuración base
	rasterizerDesc.MultisampleEnable = false;
	rasterizerDesc.AntialiasedLineEnable = false;

	HRESULT hr = S_OK;

	// Enviamos nuestra configuración a la GPU para crear el estado lógico
	hr = device.m_device->CreateRasterizerState(&rasterizerDesc, &m_rasterizerState);

	if (FAILED(hr)) {
		ERROR("Rasterizer", "init", "CHECK FOR CreateRasterizerState()");
	}
	return hr;
}

// Inicializa el rasterizador pero permite personalizar el Modo de Relleno y el Descarte de Caras
HRESULT
RasterizerState::init(Device device, unsigned int FillMode, unsigned int CullMode) {
	D3D11_RASTERIZER_DESC rasterizerDesc = {};

	// Hacemos un casting (conversión) de los números enteros que pasamos por parámetro
	// a los tipos enumerados que DirectX espera.
	rasterizerDesc.FillMode = (D3D11_FILL_MODE)FillMode;
	rasterizerDesc.CullMode = (D3D11_CULL_MODE)CullMode;

	rasterizerDesc.FrontCounterClockwise = false;
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.SlopeScaledDepthBias = 0.0f;
	rasterizerDesc.DepthBiasClamp = 0.0f;
	rasterizerDesc.DepthClipEnable = true;
	rasterizerDesc.ScissorEnable = false;
	rasterizerDesc.MultisampleEnable = false;
	rasterizerDesc.AntialiasedLineEnable = false;

	HRESULT hr = S_OK;
	hr = device.m_device->CreateRasterizerState(&rasterizerDesc, &m_rasterizerState);

	if (FAILED(hr)) {
		ERROR("Rasterizer", "init", "CHECK FOR CreateRasterizerState()");
	}
	return hr;
}

// Espacio reservado por si necesitamos modificar dinámicamente este estado (ej. cambiar a Wireframe en tiempo real)
void
RasterizerState::update() {
}

// Vincula este estado de rasterización al pipeline para que la GPU comience a usar estas reglas
void
RasterizerState::render(DeviceContext& deviceContext) {
	// IMPORTANTE: Asegúrate de que tu clase DeviceContext tenga implementado el método RSSetState,
	// de lo contrario, tendrías que usar: deviceContext.m_deviceContext->RSSetState(m_rasterizerState);
	deviceContext.RSSetState(m_rasterizerState);
}

// Libera la memoria de la tarjeta de video de forma segura
void
RasterizerState::destroy() {
	SAFE_RELEASE(m_rasterizerState);
}