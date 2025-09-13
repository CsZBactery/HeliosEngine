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
    ERROR(L"DeviceContext", L"initFromDevice", L"device is nullptr");
    return E_POINTER;
  }
  destroy();
  device->GetImmediateContext(&m_deviceContext);
  if (!m_deviceContext) {
    ERROR(L"DeviceContext", L"initFromDevice", L"GetImmediateContext returned nullptr");
    return E_FAIL;
  }
  MESSAGE(L"DeviceContext", L"initFromDevice", L"Immediate context acquired");
  return S_OK;
}

HRESULT DeviceContext::attach(ID3D11DeviceContext* ctx) {
  destroy();
  if (!ctx) {
    ERROR(L"DeviceContext", L"attach", L"ctx is nullptr");
    return E_POINTER;
  }
  m_deviceContext = ctx;
  m_deviceContext->AddRef();
  MESSAGE(L"DeviceContext", L"attach", L"Context attached (AddRef)");
  return S_OK;
}

void DeviceContext::clearState() {
  if (!m_deviceContext) {
    ERROR(L"DeviceContext", L"clearState", L"m_deviceContext is nullptr");
    return;
  }
  m_deviceContext->ClearState();
}

void DeviceContext::destroy() {
  SAFE_RELEASE(m_deviceContext);
}

void DeviceContext::OMSetRenderTargets(
  UINT NumViews,
  ID3D11RenderTargetView* const* ppRenderTargetViews,
  ID3D11DepthStencilView* pDepthStencilView)
{
  if (!m_deviceContext) {
    ERROR(L"DeviceContext", L"OMSetRenderTargets", L"m_deviceContext is nullptr");
    return;
  }
  if (NumViews > 0 && !ppRenderTargetViews) {
    ERROR(L"DeviceContext", L"OMSetRenderTargets", L"ppRenderTargetViews is nullptr while NumViews > 0");
    return;
  }
  m_deviceContext->OMSetRenderTargets(NumViews, ppRenderTargetViews, pDepthStencilView);
}

void DeviceContext::RSSetViewports(UINT NumViewports, const D3D11_VIEWPORT* pViewports) {
  if (!m_deviceContext) {
    ERROR(L"DeviceContext", L"RSSetViewports", L"m_deviceContext is nullptr");
    return;
  }
  if (NumViewports == 0 || !pViewports) {
    ERROR(L"DeviceContext", L"RSSetViewports", L"invalid params (NumViewports==0 or pViewports==nullptr)");
    return;
  }
  m_deviceContext->RSSetViewports(NumViewports, pViewports);
}

void DeviceContext::RSSetState(ID3D11RasterizerState* pRasterizerState) {
  if (!m_deviceContext) {
    ERROR(L"DeviceContext", L"RSSetState", L"m_deviceContext is nullptr");
    return;
  }
  // pRasterizerState puede ser nullptr para desbind
  m_deviceContext->RSSetState(pRasterizerState);
}

void DeviceContext::IASetInputLayout(ID3D11InputLayout* pInputLayout) {
  if (!m_deviceContext) {
    ERROR(L"DeviceContext", L"IASetInputLayout", L"m_deviceContext is nullptr");
    return;
  }
  if (!pInputLayout) {
    ERROR(L"DeviceContext", L"IASetInputLayout", L"pInputLayout is nullptr");
    return;
  }
  m_deviceContext->IASetInputLayout(pInputLayout);
}

void DeviceContext::IASetVertexBuffers(UINT StartSlot, UINT NumBuffers,
  ID3D11Buffer* const* ppVertexBuffers,
  const UINT* pStrides, const UINT* pOffsets)
{
  if (!m_deviceContext) {
    ERROR(L"DeviceContext", L"IASetVertexBuffers", L"m_deviceContext is nullptr");
    return;
  }
  if (NumBuffers == 0) {
    ERROR(L"DeviceContext", L"IASetVertexBuffers", L"NumBuffers == 0");
    return;
  }
  if (!ppVertexBuffers || !pStrides || !pOffsets) {
    ERROR(L"DeviceContext", L"IASetVertexBuffers", L"null array(s): ppVertexBuffers/pStrides/pOffsets");
    return;
  }
  m_deviceContext->IASetVertexBuffers(StartSlot, NumBuffers, ppVertexBuffers, pStrides, pOffsets);
}

void DeviceContext::IASetIndexBuffer(ID3D11Buffer* pIndexBuffer,
  DXGI_FORMAT Format, UINT Offset)
{
  if (!m_deviceContext) {
    ERROR(L"DeviceContext", L"IASetIndexBuffer", L"m_deviceContext is nullptr");
    return;
  }
  if (!pIndexBuffer) {
    ERROR(L"DeviceContext", L"IASetIndexBuffer", L"pIndexBuffer is nullptr");
    return;
  }
  if (Format != DXGI_FORMAT_R16_UINT && Format != DXGI_FORMAT_R32_UINT) {
    ERROR(L"DeviceContext", L"IASetIndexBuffer", L"Format must be R16_UINT or R32_UINT");
    return;
  }
  m_deviceContext->IASetIndexBuffer(pIndexBuffer, Format, Offset);
}

void DeviceContext::IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY Topology) {
  if (!m_deviceContext) {
    ERROR(L"DeviceContext", L"IASetPrimitiveTopology", L"m_deviceContext is nullptr");
    return;
  }
  m_deviceContext->IASetPrimitiveTopology(Topology);
}

void DeviceContext::VSSetShader(ID3D11VertexShader* pVS,
  ID3D11ClassInstance* const* ppClassInstances,
  UINT NumClassInstances)
{
  if (!m_deviceContext) {
    ERROR(L"DeviceContext", L"VSSetShader", L"m_deviceContext is nullptr");
    return;
  }
  // pVS puede ser nullptr para desbind; ppClassInstances puede ser nullptr si NumClassInstances=0
  m_deviceContext->VSSetShader(pVS, ppClassInstances, NumClassInstances);
}

void DeviceContext::VSSetConstantBuffers(UINT StartSlot, UINT NumBuffers,
  ID3D11Buffer* const* ppConstantBuffers)
{
  if (!m_deviceContext) {
    ERROR(L"DeviceContext", L"VSSetConstantBuffers", L"m_deviceContext is nullptr");
    return;
  }
  if (NumBuffers == 0 || !ppConstantBuffers) {
    ERROR(L"DeviceContext", L"VSSetConstantBuffers", L"invalid params (NumBuffers==0 or ppConstantBuffers==nullptr)");
    return;
  }
  m_deviceContext->VSSetConstantBuffers(StartSlot, NumBuffers, ppConstantBuffers);
}

void DeviceContext::PSSetShader(ID3D11PixelShader* pPS,
  ID3D11ClassInstance* const* ppClassInstances,
  UINT NumClassInstances)
{
  if (!m_deviceContext) {
    ERROR(L"DeviceContext", L"PSSetShader", L"m_deviceContext is nullptr");
    return;
  }
  m_deviceContext->PSSetShader(pPS, ppClassInstances, NumClassInstances);
}

void DeviceContext::PSSetConstantBuffers(UINT StartSlot, UINT NumBuffers,
  ID3D11Buffer* const* ppConstantBuffers)
{
  if (!m_deviceContext) {
    ERROR(L"DeviceContext", L"PSSetConstantBuffers", L"m_deviceContext is nullptr");
    return;
  }
  if (NumBuffers == 0 || !ppConstantBuffers) {
    ERROR(L"DeviceContext", L"PSSetConstantBuffers", L"invalid params (NumBuffers==0 or ppConstantBuffers==nullptr)");
    return;
  }
  m_deviceContext->PSSetConstantBuffers(StartSlot, NumBuffers, ppConstantBuffers);
}

void DeviceContext::PSSetShaderResources(UINT StartSlot, UINT NumViews,
  ID3D11ShaderResourceView* const* ppShaderResourceViews)
{
  if (!m_deviceContext) {
    ERROR(L"DeviceContext", L"PSSetShaderResources", L"m_deviceContext is nullptr");
    return;
  }
  if (NumViews == 0 || !ppShaderResourceViews) {
    ERROR(L"DeviceContext", L"PSSetShaderResources", L"invalid params (NumViews==0 or ppShaderResourceViews==nullptr)");
    return;
  }
  m_deviceContext->PSSetShaderResources(StartSlot, NumViews, ppShaderResourceViews);
}

void DeviceContext::PSSetSamplers(UINT StartSlot, UINT NumSamplers,
  ID3D11SamplerState* const* ppSamplers)
{
  if (!m_deviceContext) {
    ERROR(L"DeviceContext", L"PSSetSamplers", L"m_deviceContext is nullptr");
    return;
  }
  if (NumSamplers == 0 || !ppSamplers) {
    ERROR(L"DeviceContext", L"PSSetSamplers", L"invalid params (NumSamplers==0 or ppSamplers==nullptr)");
    return;
  }
  m_deviceContext->PSSetSamplers(StartSlot, NumSamplers, ppSamplers);
}
