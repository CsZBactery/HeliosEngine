#include "../include/Viewport.h"
#include "../include/Window.h"
#include "../include/DeviceContext.h"
// ../include/ //
HRESULT Viewport::init(const Window& window)
{
  const UINT w = static_cast<UINT>(window.width());  
  const UINT h = static_cast<UINT>(window.height());
  if (w == 0 || h == 0) return E_INVALIDARG;

  m_viewport.TopLeftX = 0.0f;
  m_viewport.TopLeftY = 0.0f;
  m_viewport.Width = static_cast<FLOAT>(w);
  m_viewport.Height = static_cast<FLOAT>(h);
  m_viewport.MinDepth = 0.0f;
  m_viewport.MaxDepth = 1.0f;
  return S_OK;
}

HRESULT Viewport::init(unsigned int width, unsigned int height)
{
  if (width == 0 || height == 0) return E_INVALIDARG;

  m_viewport.TopLeftX = 0.0f;
  m_viewport.TopLeftY = 0.0f;
  m_viewport.Width = static_cast<FLOAT>(width);
  m_viewport.Height = static_cast<FLOAT>(height);
  m_viewport.MinDepth = 0.0f;
  m_viewport.MaxDepth = 1.0f;
  return S_OK;
}

void Viewport::render(DeviceContext& deviceContext)
{
  deviceContext.RSSetViewports(1, &m_viewport);
}
