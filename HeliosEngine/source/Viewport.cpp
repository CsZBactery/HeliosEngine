#include "../include/Viewport.h"
#include "../include/Window.h"
#include "../include/DeviceContext.h"

HRESULT
Viewport::init(const Window& window) {
	// Validacion de seguridad: necesitamos una ventana valida
	if (!window.m_hWnd) {
		ERROR("Viewport", "init", "Handle de ventana (m_hWnd) nulo");
		return E_POINTER;
	}
	if (window.m_width == 0 || window.m_height == 0) {
		ERROR("Viewport", "init", "Dimensiones de ventana invalidas (0)");
		return E_INVALIDARG;
	}

	// Configuracion estandar para cubrir toda la ventana.
	// TopLeft (0,0) es la esquina superior izquierda.
	m_viewport.Width = static_cast<float>(window.m_width);
	m_viewport.Height = static_cast<float>(window.m_height);

	// Rango de profundidad estandar de DirectX (0.0 a 1.0).
	// Si quisieras efectos raros de profundidad invertida, aqui lo cambiarias.
	m_viewport.MinDepth = 0.0f;
	m_viewport.MaxDepth = 1.0f;
	m_viewport.TopLeftX = 0;
	m_viewport.TopLeftY = 0;

	return S_OK;
}

HRESULT
Viewport::init(unsigned int width, unsigned int height) {
	// Sobrecarga util para cuando queremos renderizar en una textura (Off-screen)
	// o para split-screen (pantalla dividida), donde el viewport no ocupa toda la ventana.
	if (width == 0 || height == 0) {
		ERROR("Viewport", "init", "Dimensiones cero");
		return E_INVALIDARG;
	}

	m_viewport.Width = static_cast<float>(width);
	m_viewport.Height = static_cast<float>(height);
	m_viewport.MinDepth = 0.0f;
	m_viewport.MaxDepth = 1.0f;
	m_viewport.TopLeftX = 0;
	m_viewport.TopLeftY = 0;

	return S_OK;
}

void
Viewport::render(DeviceContext& deviceContext) {
	if (!deviceContext.m_deviceContext) {
		ERROR("Viewport", "init", "Contexto nulo");
		return;
	}
	// Comando al Rasterizador: "Todo lo que dibujes, ajustalo a este rectangulo"
	deviceContext.RSSetViewports(1, &m_viewport);
}