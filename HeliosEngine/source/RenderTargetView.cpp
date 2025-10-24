#include "../include/Prerequisites.h"
#include "../include/RenderTargetView.h"
#include "../include/Device.h"
#include "../include/DeviceContext.h"
#include "../include/Texture.h"
#include "../include/DepthStencilView.h"

// NOTA: asumo que Texture expone ID3D11Texture2D* m_texture (como los demás wrappers del profe).
//       Si tu Texture tiene get(), cámbialo por: auto* tex = backBuffer.get();

void RenderTargetView::destroy() {
    SAFE_RELEASE(m_rtv);
}

HRESULT RenderTargetView::createFromBackbuffer(IDXGISwapChain* swap, ID3D11Device* dev) {
    if (!swap || !dev) return E_POINTER;

    SAFE_RELEASE(m_rtv);

    ID3D11Texture2D* backTex = nullptr;
    HRESULT hr = swap->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backTex);
    if (FAILED(hr)) return hr;

    hr = dev->CreateRenderTargetView(backTex, nullptr, &m_rtv);
    SAFE_RELEASE(backTex);
    return hr;
}

HRESULT RenderTargetView::createFromTexture(ID3D11Device* dev, ID3D11Texture2D* tex,
    const D3D11_RENDER_TARGET_VIEW_DESC* desc) {
    if (!dev || !tex) return E_POINTER;
    SAFE_RELEASE(m_rtv);
    return dev->CreateRenderTargetView(tex, desc, &m_rtv);
}

HRESULT RenderTargetView::init(Device& device, Texture& backBuffer, DXGI_FORMAT format) {
    if (!device.m_device) { ERROR(RenderTargetView, init, L"Device es nullptr"); return E_POINTER; }

    // Obtener la textura 2D desde el wrapper Texture
    ID3D11Texture2D* tex2D = nullptr;

    // Opción A) si Texture expone m_texture público:
    extern ID3D11Texture2D* __force_unused; // solo para que VS no se queje si no usas A
    tex2D = backBuffer.m_texture;

    // Opción B) si tu Texture tiene get():
    // tex2D = backBuffer.get();

    if (!tex2D) {
        ERROR(RenderTargetView, init, L"BackBuffer no contiene textura válida");
        return E_FAIL;
    }

    // Si piden formato, podemos construir un RTV desc opcional.
    D3D11_RENDER_TARGET_VIEW_DESC rtvDesc{};
    rtvDesc.Format = format;
    rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    rtvDesc.Texture2D.MipSlice = 0;

    SAFE_RELEASE(m_rtv);
    HRESULT hr = device.m_device->CreateRenderTargetView(tex2D, &rtvDesc, &m_rtv);
    if (FAILED(hr)) {
        ERROR(RenderTargetView, init, L"CreateRenderTargetView falló");
        return hr;
    }
    return S_OK;
}

void RenderTargetView::render(DeviceContext& deviceContext,
    DepthStencilView& dsv,
    unsigned int /*NumViews*/,
    const float ClearColor[4]) {
    if (!m_rtv) {
        ERROR(RenderTargetView, render, L"RTV es nullptr");
        return;
    }

    ID3D11RenderTargetView* views[1] = { m_rtv };
    ID3D11DepthStencilView* dsvPtr = dsv.get();

    if (auto* ctx = deviceContext.get()) {
        ctx->OMSetRenderTargets(1, views, dsvPtr);
        ctx->ClearRenderTargetView(m_rtv, ClearColor);
    }
    else {
        ERROR(RenderTargetView, render, L"DeviceContext::get() es nullptr");
    }
}
