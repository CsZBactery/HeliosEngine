#include "../include/Prerequisites.h"
#include "../include/DeviceContext.h"

DeviceContext::DeviceContext(DeviceContext&& other) noexcept
  : m_deviceContext(other.m_deviceContext) {
  other.m_deviceContext = nullptr;
}

DeviceContext& DeviceContext::operator=(DeviceContext&& other) noexcept {
  if (this != &other) {
    destroy();
    m_deviceContext = other.m_deviceContext;
    other.m_deviceContext = nullptr;
  }
  return *this;
}

HRESULT DeviceContext::initFromDevice(ID3D11Device* device) {
  if (!device) {
    ERROR("DeviceContext", "initFromDevice", "device is nullptr");
    return E_POINTER;
  }
  destroy();
  device->GetImmediateContext(&m_deviceContext);
  if (!m_deviceContext) {
    ERROR("DeviceContext", "initFromDevice", "GetImmediateContext returned nullptr");
    return E_FAIL;
  }
  MESSAGE("DeviceContext", "initFromDevice", "Immediate context acquired");
  return S_OK;
}

HRESULT DeviceContext::attach(ID3D11DeviceContext* ctx) {
  destroy();
  if (!ctx) {
    ERROR("DeviceContext", "attach", "ctx is nullptr");
    return E_POINTER;
  }
  m_deviceContext = ctx;
  m_deviceContext->AddRef();
  MESSAGE("DeviceContext", "attach", "Context attached (AddRef)");
  return S_OK;
}

void DeviceContext::clearState() {
  if (m_deviceContext) {
    m_deviceContext->ClearState();
  }
}

void DeviceContext::destroy() {
  SAFE_RELEASE(m_deviceContext);
}

void DeviceContext::OMSetRenderTargets(
  UINT NumViews,
  ID3D11RenderTargetView* const* ppRenderTargetViews,
  ID3D11DepthStencilView* pDepthStencilView) {
  if (!m_deviceContext) {
    ERROR("DeviceContext", "OMSetRenderTargets", "m_deviceContext is nullptr");
    return;
  }
  m_deviceContext->OMSetRenderTargets(NumViews, ppRenderTargetViews, pDepthStencilView);
}
