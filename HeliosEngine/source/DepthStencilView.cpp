#include "../include/DepthStencilView.h"
#include "../include/Device.h"
#include "../include/DeviceContext.h"
#include "../include/Texture.h"

// Inicializa la DSV (Depth/Stencil View) a partir de una textura de profundidad.
HRESULT
DepthStencilView::init(Device& device, Texture& depthStencil, DXGI_FORMAT format) {
    // Validaciones básicas
    if (!device.m_device) {
        ERROR("DepthStencilView", "init", "Device is null.");
        return E_POINTER;
    }
    if (!depthStencil.m_texture) {
        ERROR("DepthStencilView", "init", "texture is null.");
        return E_POINTER;
    }
    if (format == DXGI_FORMAT_UNKNOWN) {
        ERROR("DepthStencilView", "init", "Format is DXGI_FORMAT_UNKNOWN.");
        return E_INVALIDARG;
    }

    // Consultar si la textura es MSAA para elegir la vista adecuada
    D3D11_TEXTURE2D_DESC texDesc;
    depthStencil.m_texture->GetDesc(&texDesc);

    // Descripción base de la DSV
    D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
    ZeroMemory(&descDSV, sizeof(descDSV));
    descDSV.Format = format;

    // Selección de dimensión según MSAA
    if (texDesc.SampleDesc.Count > 1) {
        descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
    }
    else {
        descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        descDSV.Texture2D.MipSlice = 0;
    }

    // Crear la DSV
    HRESULT hr = device.m_device->
        CreateDepthStencilView(depthStencil.m_texture,
            &descDSV,
            &m_depthStencilView);

    if (FAILED(hr)) {
        ERROR("DepthStencilView", "init",
            ("Failed to create depth stencil view. HRESULT: " + std::to_string(hr)).c_str());
        return hr;
    }

    // Éxito
    MESSAGE("DepthStencilView", "init", "Depth stencil view created successfully.");
    return S_OK;
}

// Limpia el buffer de profundidad y stencil al inicio de cada frame.
void
DepthStencilView::render(DeviceContext& deviceContext) {
    // Validaciones
    if (!deviceContext.m_deviceContext) {
        ERROR("DepthStencilView", "render", "Device context is null.");
        return;
    }
    if (!m_depthStencilView) {
        ERROR("DepthStencilView", "render", "DepthStencilView is null.");
        return;
    }

    // Clear de profundidad (1.0f) y stencil (0)
    deviceContext.m_deviceContext->ClearDepthStencilView(m_depthStencilView,
        D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
        1.0f,
        0);
}

// Libera la DSV.
void
DepthStencilView::destroy() {
    SAFE_RELEASE(m_depthStencilView);
}
