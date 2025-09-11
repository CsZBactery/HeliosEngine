#include "../include/Prerequisites.h"
#include "../include/Device.h"

HRESULT Device::CreateRenderTargetView(ID3D11Resource* pResource,
  const D3D11_RENDER_TARGET_VIEW_DESC* pDesc,
  ID3D11RenderTargetView** ppRTView)
{
  // Validaciones
  if (!m_device) {
    ERROR("Device", "CreateRenderTargetView", "m_device is nullptr");
    return E_FAIL;
  }
  if (!pResource) {
    ERROR("Device", "CreateRenderTargetView", "pResource is nullptr");
    return E_INVALIDARG;
  }
  if (!ppRTView) {
    ERROR("Device", "CreateRenderTargetView", "ppRTView is nullptr");
    return E_POINTER;
  }

  // Crear el RTV
  HRESULT hr = m_device->CreateRenderTargetView(pResource, pDesc, ppRTView);
  if (SUCCEEDED(hr)) {
    MESSAGE("Device", "CreateRenderTargetView", "Render Target View created successfully!");
  }
  else {
    std::string msg = std::string("Failed to create Render Target View. HRESULT=") + std::to_string(hr);
    ERROR("Device", "CreateRenderTargetView", msg.c_str());
  }
  return hr;
}
