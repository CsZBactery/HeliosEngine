// DeviceContext.cpp
#include "../include/DeviceContext.h"

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(x) if(x){ (x)->Release(); (x)=nullptr; }
#endif

// --------------------------
// Move semantics
// --------------------------
DeviceContext::DeviceContext(DeviceContext&& other) noexcept {
  m_deviceContext = other.m_deviceContext;
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

// --------------------------
// Gestión / adquisición
// --------------------------
HRESULT DeviceContext::initFromDevice(ID3D11Device* device) {
  if (!device) return E_POINTER;
  destroy();
  device->GetImmediateContext(&m_deviceContext); // AddRef implícito
  return (m_deviceContext) ? S_OK : E_FAIL;
}

HRESULT DeviceContext::attach(ID3D11DeviceContext* ctx) {
  if (!ctx) return E_POINTER;
  destroy();
  m_deviceContext = ctx;
  m_deviceContext->AddRef();
  return S_OK;
}

void DeviceContext::clearState() {
  if (m_deviceContext) m_deviceContext->ClearState();
}

void DeviceContext::destroy() {
  SAFE_RELEASE(m_deviceContext);
}

// --------------------------
// OUTPUT-MERGER (OM)
// --------------------------
void DeviceContext::OMSetRenderTargets(UINT NumViews,
  ID3D11RenderTargetView* const* ppRenderTargetViews,
  ID3D11DepthStencilView* pDepthStencilView) {
  if (m_deviceContext)
    m_deviceContext->OMSetRenderTargets(NumViews, ppRenderTargetViews, pDepthStencilView);
}

// --------------------------
// RASTERIZER (RS)
// --------------------------
void DeviceContext::RSSetViewports(UINT NumViewports, const D3D11_VIEWPORT* pViewports) {
  if (m_deviceContext) m_deviceContext->RSSetViewports(NumViewports, pViewports);
}
void DeviceContext::RSSetState(ID3D11RasterizerState* pRasterizerState) {
  if (m_deviceContext) m_deviceContext->RSSetState(pRasterizerState);
}

// --------------------------
// INPUT ASSEMBLER (IA)
// --------------------------
void DeviceContext::IASetInputLayout(ID3D11InputLayout* pInputLayout) {
  if (m_deviceContext) m_deviceContext->IASetInputLayout(pInputLayout);
}
void DeviceContext::IASetVertexBuffers(UINT StartSlot, UINT NumBuffers,
  ID3D11Buffer* const* ppVertexBuffers, const UINT* pStrides, const UINT* pOffsets) {
  if (m_deviceContext)
    m_deviceContext->IASetVertexBuffers(StartSlot, NumBuffers, ppVertexBuffers, pStrides, pOffsets);
}
void DeviceContext::IASetIndexBuffer(ID3D11Buffer* pIndexBuffer,
  DXGI_FORMAT Format, UINT Offset) {
  if (m_deviceContext) m_deviceContext->IASetIndexBuffer(pIndexBuffer, Format, Offset);
}
void DeviceContext::IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY Topology) {
  if (m_deviceContext) m_deviceContext->IASetPrimitiveTopology(Topology);
}

// --------------------------
// VERTEX SHADER (VS)
// --------------------------
void DeviceContext::VSSetShader(ID3D11VertexShader* pVS,
  ID3D11ClassInstance* const* ppClassInstances, UINT NumClassInstances) {
  if (m_deviceContext) m_deviceContext->VSSetShader(pVS, ppClassInstances, NumClassInstances);
}
void DeviceContext::VSSetConstantBuffers(UINT StartSlot, UINT NumBuffers,
  ID3D11Buffer* const* ppConstantBuffers) {
  if (m_deviceContext) m_deviceContext->VSSetConstantBuffers(StartSlot, NumBuffers, ppConstantBuffers);
}

// --------------------------
// PIXEL SHADER (PS)
// --------------------------
void DeviceContext::PSSetShader(ID3D11PixelShader* pPS,
  ID3D11ClassInstance* const* ppClassInstances, UINT NumClassInstances) {
  if (m_deviceContext) m_deviceContext->PSSetShader(pPS, ppClassInstances, NumClassInstances);
}
void DeviceContext::PSSetConstantBuffers(UINT StartSlot, UINT NumBuffers,
  ID3D11Buffer* const* ppConstantBuffers) {
  if (m_deviceContext) m_deviceContext->PSSetConstantBuffers(StartSlot, NumBuffers, ppConstantBuffers);
}
void DeviceContext::PSSetShaderResources(UINT StartSlot, UINT NumViews,
  ID3D11ShaderResourceView* const* ppShaderResourceViews) {
  if (m_deviceContext) m_deviceContext->PSSetShaderResources(StartSlot, NumViews, ppShaderResourceViews);
}
void DeviceContext::PSSetSamplers(UINT StartSlot, UINT NumSamplers,
  ID3D11SamplerState* const* ppSamplers) {
  if (m_deviceContext) m_deviceContext->PSSetSamplers(StartSlot, NumSamplers, ppSamplers);
}
