#pragma once
#include "../include/Prerequisites.h"

class Device;
class Texture;
class DeviceContext;

class DepthStencilView {
public:
    DepthStencilView() = default;
    ~DepthStencilView() { destroy(); }

    DepthStencilView(const DepthStencilView&) = delete;
    DepthStencilView& operator=(const DepthStencilView&) = delete;

    // Crea un DSV a partir de una textura 2D de depth (tu wrapper Texture la provee)
    HRESULT init(Device& device, Texture& depthTexture, DXGI_FORMAT format);

    // Limpia el depth-stencil (con valores por defecto)
    void render(DeviceContext& deviceContext,
        float depth = 1.0f,
        UINT8 stencil = 0,
        UINT clearFlags = D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL);

    void destroy();

    // Acceso seguro (para otros wrappers que quieran bindearlo)
    ID3D11DepthStencilView* get() const { return m_dsv; }

private:
    ID3D11DepthStencilView* m_dsv = nullptr;
};
