#include "../include/Prerequisites.h"
#include "../include/DepthStencilView.h"
#include "../include/Device.h"
#include "../include/DeviceContext.h"
#include "../include/Texture.h"

HRESULT DepthStencilView::init(Device& device, Texture& depthTexture, DXGI_FORMAT format) {
    if (!device.m_device) { ERROR(DepthStencilView, init, L"Device es nullptr"); return E_POINTER; }

    // Obtén la textura 2D del wrapper Texture:
    // Opción A) si tienes público "m_texture":
    ID3D11Texture2D* tex2D = depthTexture.m_texture;
    // Opción B) si tu Texture tiene get(): tex2D = depthTexture.get();

    if (!tex2D) { ERROR(DepthStencilView, init, L"Depth texture es nullptr"); return E_FAIL; }

    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
    dsvDesc.Format = format;                          // DXGI_FORMAT_D24_UNORM_S8_UINT o el que pases
    dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Texture2D.MipSlice = 0;

    SAFE_RELEASE(m_dsv);
    HRESULT hr = device.m_device->CreateDepthStencilView(tex2D, &dsvDesc, &m_dsv);
    if (FAILED(hr)) { ERROR(DepthStencilView, init, L"CreateDepthStencilView falló"); return hr; }
    return S_OK;
}

void DepthStencilView::render(DeviceContext& deviceContext,
    float depth, UINT8 stencil, UINT clearFlags) {
    auto* ctx = deviceContext.get();
    if (!ctx || !m_dsv) { ERROR(DepthStencilView, render, L"ctx o m_dsv es nullptr"); return; }
    ctx->ClearDepthStencilView(m_dsv, clearFlags, depth, stencil);
}

void DepthStencilView::destroy() {
    SAFE_RELEASE(m_dsv);
}
