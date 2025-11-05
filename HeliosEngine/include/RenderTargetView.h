#pragma once
#include "Prerequisites.h"

class Device;
class DeviceContext;
class Texture;
class DepthStencilView;

/** Encapsula un ID3D11RenderTargetView */
class RenderTargetView {
public:
    RenderTargetView() = default;
    ~RenderTargetView() = default;

    HRESULT init(Device& device, Texture& backBuffer, DXGI_FORMAT format);
    HRESULT init(Device& device, Texture& inTex, D3D11_RTV_DIMENSION viewDimension, DXGI_FORMAT format);

    void    update();
    void    render(DeviceContext& deviceContext, DepthStencilView& depthStencilView, unsigned int numViews, const float clearColor[4]);
    void    render(DeviceContext& deviceContext, unsigned int numViews);
    void    destroy();

private:
    ID3D11RenderTargetView* m_renderTargetView = nullptr;
};
