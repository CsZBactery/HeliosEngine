#include "../include/RenderTargetView.h"

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(x) do{ if(x){ (x)->Release(); (x)=nullptr; } }while(0)
#endif

void RenderTargetView::destroy() {
  SAFE_RELEASE(m_rtv);
}

HRESULT RenderTargetView::createFromBackbuffer(IDXGISwapChain* swap, ID3D11Device* dev)
{
  if (!swap || !dev) return E_POINTER;
  destroy();

  ID3D11Texture2D* back = nullptr;
  HRESULT hr = swap->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&back);
  if (FAILED(hr)) return hr;

  hr = dev->CreateRenderTargetView(back, nullptr, &m_rtv);
  SAFE_RELEASE(back);
  return hr;
}

HRESULT RenderTargetView::createFromTexture(ID3D11Device* dev, ID3D11Texture2D* tex,
  const D3D11_RENDER_TARGET_VIEW_DESC* desc)
{
  if (!dev || !tex) return E_POINTER;
  destroy();
  return dev->CreateRenderTargetView(tex, desc, &m_rtv);
}
