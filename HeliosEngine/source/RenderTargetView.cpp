#include "../include/RenderTargetView.h"
#include "../include/Device.h"
#include "../include/Texture.h"
#include "../include/DeviceContext.h"
#include "../include/DepthStencilView.h"

HRESULT RenderTargetView::init(Device& device, Texture& backBuffer, DXGI_FORMAT format) {
    if (!device.m_device) { ERROR("RenderTargetView", "init", "Device is nullptr.");  return E_POINTER; }
    if (!backBuffer.m_texture) { ERROR("RenderTargetView", "init", "Texture is nullptr."); return E_POINTER; }
    if (format == DXGI_FORMAT_UNKNOWN) { ERROR("RenderTargetView", "init", "Format is unknown.");  return E_INVALIDARG; }

    // Detectar si el back-buffer es MSAA
    D3D11_TEXTURE2D_DESC texDesc{};
    backBuffer.m_texture->GetDesc(&texDesc);

    D3D11_RENDER_TARGET_VIEW_DESC desc{};
    desc.Format = format;
    if (texDesc.SampleDesc.Count > 1) {
        desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
    }
    else {
        desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
        desc.Texture2D.MipSlice = 0;
    }

    HRESULT hr = device.m_device->CreateRenderTargetView(backBuffer.m_texture, &desc, &m_renderTargetView);
    if (FAILED(hr)) {
        ERROR("RenderTargetView", "init", ("Failed to create RTV. HRESULT: " + std::to_string(hr)).c_str());
        return hr;
    }
    return S_OK;
}

HRESULT RenderTargetView::init(Device& device, Texture& inTex, D3D11_RTV_DIMENSION /*viewDimension*/, DXGI_FORMAT format) {
    if (!device.m_device) { ERROR("RenderTargetView", "init", "Device is nullptr.");  return E_POINTER; }
    if (!inTex.m_texture) { ERROR("RenderTargetView", "init", "Texture is nullptr."); return E_POINTER; }
    if (format == DXGI_FORMAT_UNKNOWN) { ERROR("RenderTargetView", "init", "Format is unknown.");  return E_INVALIDARG; }

    // Igual que arriba: autodetectar MSAA
    D3D11_TEXTURE2D_DESC texDesc{};
    inTex.m_texture->GetDesc(&texDesc);

    D3D11_RENDER_TARGET_VIEW_DESC desc{};
    desc.Format = format;
    if (texDesc.SampleDesc.Count > 1) {
        desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
    }
    else {
        desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
        desc.Texture2D.MipSlice = 0;
    }

    HRESULT hr = device.m_device->CreateRenderTargetView(inTex.m_texture, &desc, &m_renderTargetView);
    if (FAILED(hr)) {
        ERROR("RenderTargetView", "init", ("Failed to create RTV. HRESULT: " + std::to_string(hr)).c_str());
        return hr;
    }
    return S_OK;
}

void RenderTargetView::update() {}

void RenderTargetView::render(DeviceContext& deviceContext, DepthStencilView& depthStencilView, unsigned int /*numViews*/, const float clearColor[4]) {
    if (!deviceContext.m_deviceContext) { ERROR("RenderTargetView", "render", "DeviceContext is nullptr."); return; }
    if (!m_renderTargetView) { ERROR("RenderTargetView", "render", "RenderTargetView is nullptr."); return; }

    deviceContext.m_deviceContext->ClearRenderTargetView(m_renderTargetView, clearColor);
    deviceContext.m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, depthStencilView.m_depthStencilView);
}

void RenderTargetView::render(DeviceContext& deviceContext, unsigned int /*numViews*/) {
    if (!deviceContext.m_deviceContext) { ERROR("RenderTargetView", "render", "DeviceContext is nullptr."); return; }
    if (!m_renderTargetView) { ERROR("RenderTargetView", "render", "RenderTargetView is nullptr."); return; }

    deviceContext.m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, nullptr);
}

void RenderTargetView::destroy() {
    SAFE_RELEASE(m_renderTargetView);
}
