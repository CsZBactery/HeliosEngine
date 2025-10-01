#include "../include/DepthStencilView.h"

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(x) do{ if(x){ (x)->Release(); (x)=nullptr; } }while(0)
#endif

void DepthStencilView::destroy()
{
  SAFE_RELEASE(m_state);
  SAFE_RELEASE(m_dsv);
  SAFE_RELEASE(m_tex);
}

HRESULT DepthStencilView::create(ID3D11Device* dev, UINT w, UINT h, DXGI_FORMAT fmt)
{
  if (!dev || w == 0 || h == 0) return E_INVALIDARG;
  destroy();

  D3D11_TEXTURE2D_DESC td{};
  td.Width = w; td.Height = h;
  td.MipLevels = 1; td.ArraySize = 1;
  td.Format = fmt;
  td.SampleDesc.Count = 1;
  td.Usage = D3D11_USAGE_DEFAULT;
  td.BindFlags = D3D11_BIND_DEPTH_STENCIL;

  HRESULT hr = dev->CreateTexture2D(&td, nullptr, &m_tex);
  if (FAILED(hr)) return hr;

  hr = dev->CreateDepthStencilView(m_tex, nullptr, &m_dsv);
  return hr;
}

HRESULT DepthStencilView::ensureDepthState(ID3D11Device* dev, bool write)
{
  if (!dev) return E_POINTER;
  if (m_state) return S_OK;

  D3D11_DEPTH_STENCIL_DESC ds{};
  ds.DepthEnable = TRUE;
  ds.DepthWriteMask = write ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
  ds.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
  ds.StencilEnable = FALSE;

  return dev->CreateDepthStencilState(&ds, &m_state);
}

void DepthStencilView::bind(ID3D11DeviceContext* ctx, ID3D11RenderTargetView* rtv)
{
  if (!ctx || !m_dsv) return;
  ID3D11RenderTargetView* rt = rtv;
  ctx->OMSetRenderTargets(1, &rt, m_dsv);
  if (m_state) ctx->OMSetDepthStencilState(m_state, 0);
}
