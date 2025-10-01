#pragma once
#include <d3d11.h>
#include <dxgi.h>

class RenderTargetView {
public:
  RenderTargetView() = default;
  ~RenderTargetView() { destroy(); }
  RenderTargetView(const RenderTargetView&) = delete;
  RenderTargetView& operator=(const RenderTargetView&) = delete;

  void destroy();

  // RTV desde backbuffer de una swap chain
  HRESULT createFromBackbuffer(IDXGISwapChain* swap, ID3D11Device* dev);

  // RTV desde una textura 2D existente (desc opcional)
  HRESULT createFromTexture(ID3D11Device* dev, ID3D11Texture2D* tex,
    const D3D11_RENDER_TARGET_VIEW_DESC* desc = nullptr);

  ID3D11RenderTargetView* get() const { return m_rtv; }

private:
  ID3D11RenderTargetView* m_rtv = nullptr;
};
