#include "../include/DepthStencilView.h"
#include "../include/Device.h"
#include "../include/DeviceContext.h"
#include "../include/Texture.h"

HRESULT
DepthStencilView::init(Device& device, Texture& depthStencil, DXGI_FORMAT format) {

    // Validaciones de seguridad básicas
    if (!device.m_device) {
        ERROR("DepthStencilView", "init", "Device is null.");
        return E_POINTER;
    }
    if (!depthStencil.m_texture) {
        ERROR("DepthStencilView", "init", "Texture is null.");
        return E_POINTER;
    }
    if (format == DXGI_FORMAT_UNKNOWN) {
        ERROR("DepthStencilView", "init", "Format is DXGI_FORMAT_UNKNOWN.");
        return E_INVALIDARG;
    }

    // Consultamos la descripcion de la textura para saber si tiene MSAA (Antialiasing) activado.
    // Esto es importante porque la "Dimension" de la vista cambia si es multisampleada o no.
    D3D11_TEXTURE2D_DESC texDesc;
    depthStencil.m_texture->GetDesc(&texDesc);

    D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
    ZeroMemory(&descDSV, sizeof(descDSV));
    descDSV.Format = format;

    // Logica de seleccion de dimension:
    if (texDesc.SampleDesc.Count > 1) {
        // Si tiene mas de 1 muestra, es una textura Multisampling (MS)
        descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
    }
    else {
        // Si es una textura normal 2D
        descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        descDSV.Texture2D.MipSlice = 0;
    }

    // Creamos la vista en la GPU
    HRESULT hr = device.m_device->CreateDepthStencilView(
        depthStencil.m_texture,
        &descDSV,
        &m_depthStencilView
    );

    if (FAILED(hr)) {
        ERROR("DepthStencilView", "init",
            ("Failed to create depth stencil view. HRESULT: " + std::to_string(hr)).c_str());
        return hr;
    }

    return S_OK;
}

void
DepthStencilView::render(DeviceContext& deviceContext) {
    // Aunque la funcion se llame "render", su trabajo real es LIMPIAR el buffer.
    // Esto se debe llamar al inicio de cada frame para borrar la profundidad vieja (1.0f).

    if (!deviceContext.m_deviceContext) {
        ERROR("DepthStencilView", "render", "Device context is null.");
        return;
    }

    if (!m_depthStencilView) {
        ERROR("DepthStencilView", "render", "DepthStencilView is null.");
        return;
    }

    // Limpiamos Depth a 1.0 (lo mas lejano) y Stencil a 0
    deviceContext.m_deviceContext->ClearDepthStencilView(
        m_depthStencilView,
        D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
        1.0f,
        0
    );
}

void
DepthStencilView::destroy() {
    SAFE_RELEASE(m_depthStencilView);
}