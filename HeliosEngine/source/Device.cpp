#include "../include/Prerequisites.h"
#include "../include/Device.h"

void Device::destroy()
{
  if (m_device) { m_device->Release(); m_device = nullptr; }
}

HRESULT Device::CreateRenderTargetView(
  ID3D11Resource* pResource,
  const D3D11_RENDER_TARGET_VIEW_DESC* pDesc,
  ID3D11RenderTargetView** ppRTView)
{
  if (!m_device || !pResource || !ppRTView) return E_POINTER;
  *ppRTView = nullptr;
  return m_device->CreateRenderTargetView(pResource, pDesc, ppRTView);
}
