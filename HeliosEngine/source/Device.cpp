#include "../include/Prerequisites.h"
#include "../include/Device.h"

void Device::destroy() {
  SAFE_RELEASE(m_device);
}

HRESULT Device::CreateRenderTargetView(ID3D11Resource* pResource,
  const D3D11_RENDER_TARGET_VIEW_DESC* pDesc,
  ID3D11RenderTargetView** ppRTView)
{
  if (!m_device) { ERROR(Device, CreateRenderTargetView, L"m_device is nullptr"); return E_FAIL; }
  if (!pResource) { ERROR(Device, CreateRenderTargetView, L"pResource is nullptr"); return E_INVALIDARG; }
  if (!ppRTView) { ERROR(Device, CreateRenderTargetView, L"ppRTView is nullptr"); return E_POINTER; }

  HRESULT hr = m_device->CreateRenderTargetView(pResource, pDesc, ppRTView);
  if (SUCCEEDED(hr)) {
    MESSAGE(Device, CreateRenderTargetView, L"Render Target View created successfully!");
  }
  else {
    std::wostringstream os; os << L"Failed to create RTV. hr=0x" << std::hex << hr;
    ERROR(Device, CreateRenderTargetView, os.str().c_str());
  }
  return hr;
}