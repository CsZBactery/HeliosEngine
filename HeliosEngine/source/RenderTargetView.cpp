#include "RenderTargetView.h"
#include "Device.h"
#include "Texture.h"
#include "DeviceContext.h"
#include "DepthStencilView.h"

//
// La primera funci�n `init` inicializa el Render Target View para un back buffer,
// com�nmente una textura multimuestreo (MSAA), que es la textura de la ventana principal.
//
HRESULT
RenderTargetView::init(Device& device,
	Texture& backBuffer,
	DXGI_FORMAT Format) {
	// Se verifica que el dispositivo y la textura no sean nulos y que el formato sea v�lido.
	if (!device.m_device) {
		ERROR("RenderTargetView", "init", "Device is nullptr.");
		return E_POINTER;
	}
	if (!backBuffer.m_texture) {
		ERROR("RenderTargetView", "init", "Texture is nullptr.");
		return E_POINTER;
	}
	if (Format == DXGI_FORMAT_UNKNOWN) {
		ERROR("RenderTargetView", "init", "Format is DXGI_FORMAT_UNKNOWN.");
		return E_INVALIDARG;
	}

	// Se configura la descripci�n para una vista de destino de renderizado.
	// Se limpia la estructura y se asigna el formato y la dimensi�n.
	// D3D11_RTV_DIMENSION_TEXTURE2DMS se usa para texturas multimuestreo.
	D3D11_RENDER_TARGET_VIEW_DESC desc;
	memset(&desc, 0, sizeof(desc));
	desc.Format = Format;
	desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;

	// Se crea la vista de destino de renderizado llamando a la funci�n del dispositivo de Direct3D.
	HRESULT hr = device.m_device->CreateRenderTargetView(backBuffer.m_texture,
		&desc,
		&m_renderTargetView);
	// Se verifica si la creaci�n fue exitosa.
	if (FAILED(hr)) {
		ERROR("RenderTargetView", "init",
			("Failed to create render target view. HRESULT: " + std::to_string(hr)).c_str());
		return hr;
	}
	return S_OK;
}

//
// La segunda funci�n `init` es una versi�n m�s general que inicializa el Render Target View
// con una dimensi�n y un formato dados, no solo para back buffers.
//
HRESULT
RenderTargetView::init(Device& device,
	Texture& inTex,
	D3D11_RTV_DIMENSION ViewDimension,
	DXGI_FORMAT Format) {
	// Se realizan las mismas verificaciones de entrada.
	if (!device.m_device) {
		ERROR("RenderTargetView", "init", "Device is nullptr.");
		return E_POINTER;
	}
	if (!inTex.m_texture) {
		ERROR("RenderTargetView", "init", "Texture is nullptr.");
		return E_POINTER;
	}
	if (Format == DXGI_FORMAT_UNKNOWN) {
		ERROR("RenderTargetView", "init", "Format is DXGI_FORMAT_UNKNOWN.");
		return E_INVALIDARG;
	}

	// Se configura la descripci�n usando la dimensi�n y el formato proporcionados.
	D3D11_RENDER_TARGET_VIEW_DESC desc;
	memset(&desc, 0, sizeof(desc));
	desc.Format = Format;
	desc.ViewDimension = ViewDimension;

	// Se crea la vista de destino de renderizado.
	HRESULT hr = device.m_device->CreateRenderTargetView(inTex.m_texture,
		&desc,
		&m_renderTargetView);

	// Se verifica si la creaci�n fue exitosa.
	if (FAILED(hr)) {
		ERROR("RenderTargetView", "init",
			("Failed to create render target view. HRESULT: " + std::to_string(hr)).c_str());
		return hr;
	}

	return S_OK;
}

//
// La primera funci�n `render` limpia la vista de destino de renderizado y la asigna
// junto con un buffer de profundidad/plantilla.
//
void
RenderTargetView::render(DeviceContext& deviceContext,
	DepthStencilView& depthStencilView,
	unsigned int numViews,
	const float ClearColor[4]) {
	// Se verifica que el contexto y la vista no sean nulos.
	if (!deviceContext.m_deviceContext) {
		ERROR("RenderTargetView", "render", "DeviceContext is nullptr.");
		return;
	}
	if (!m_renderTargetView) {
		ERROR("RenderTargetView", "render", "RenderTargetView is nullptr.");
		return;
	}

	// Se limpia el color de la vista de destino de renderizado con el color especificado.
	deviceContext.m_deviceContext->ClearRenderTargetView(m_renderTargetView, ClearColor);

	// Se configura la vista de destino de renderizado y el buffer de profundidad/plantilla
	// para que la GPU sepa d�nde dibujar.
	deviceContext.m_deviceContext->OMSetRenderTargets(numViews,
		&m_renderTargetView,
		depthStencilView.m_depthStencilView);
}

//
// La segunda funci�n `render` es una versi�n simplificada que solo asigna la vista de destino de renderizado,
// sin un buffer de profundidad/plantilla.
//
void
RenderTargetView::render(DeviceContext& deviceContext, unsigned int numViews) {
	// Se verifica que el contexto y la vista no sean nulos.
	if (!deviceContext.m_deviceContext) {
		ERROR("RenderTargetView", "render", "DeviceContext is nullptr.");
		return;
	}
	if (!m_renderTargetView) {
		ERROR("RenderTargetView", "render", "RenderTargetView is nullptr.");
		return;
	}
	// Se configura la vista de destino de renderizado, pasando nullptr para el buffer de profundidad.
	deviceContext.m_deviceContext->OMSetRenderTargets(numViews,
		&m_renderTargetView,
		nullptr);
}

//
// La funci�n `destroy` libera la memoria de la vista de destino de renderizado.
//
void
RenderTargetView::destroy() {
	SAFE_RELEASE(m_renderTargetView);
}