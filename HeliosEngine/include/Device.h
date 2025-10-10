#pragma once
#include "Prerequisites.h"  

class Device {
public:
  Device() = default;
  ~Device() = default;

  Device(const Device&) = delete;
  Device& operator=(const Device&) = delete;

  void destroy();

  // ----- Proxys a ID3D11Device -----
  HRESULT CreateRenderTargetView(
    ID3D11Resource* pResource,
    const D3D11_RENDER_TARGET_VIEW_DESC* pDesc,
    ID3D11RenderTargetView** ppRTView);

  HRESULT CreateVertexShader(
    const void* pShaderBytecode,
    SIZE_T BytecodeLength,
    ID3D11ClassLinkage* pClassLinkage,
    ID3D11VertexShader** ppVertexShader)
  {
    if (!m_device || !pShaderBytecode || !ppVertexShader) return E_POINTER;
    *ppVertexShader = nullptr;
    return m_device->CreateVertexShader(pShaderBytecode, BytecodeLength, pClassLinkage, ppVertexShader);
  }

  HRESULT CreatePixelShader(
    const void* pShaderBytecode,
    SIZE_T BytecodeLength,
    ID3D11ClassLinkage* pClassLinkage,
    ID3D11PixelShader** ppPixelShader)
  {
    if (!m_device || !pShaderBytecode || !ppPixelShader) return E_POINTER;
    *ppPixelShader = nullptr;
    return m_device->CreatePixelShader(pShaderBytecode, BytecodeLength, pClassLinkage, ppPixelShader);
  }

  HRESULT CreateInputLayout(
    const D3D11_INPUT_ELEMENT_DESC* pInputElementDescs,
    UINT NumElements,
    const void* pShaderBytecodeWithInputSignature,
    SIZE_T BytecodeLength,
    ID3D11InputLayout** ppInputLayout)
  {
    if (!m_device || !pInputElementDescs || !ppInputLayout || !pShaderBytecodeWithInputSignature) return E_POINTER;
    *ppInputLayout = nullptr;
    return m_device->CreateInputLayout(pInputElementDescs, NumElements,
      pShaderBytecodeWithInputSignature, BytecodeLength, ppInputLayout);
  }

  HRESULT CreateBuffer(
    const D3D11_BUFFER_DESC* pDesc,
    const D3D11_SUBRESOURCE_DATA* pInitialData,
    ID3D11Buffer** ppBuffer)
  {
    if (!m_device || !pDesc || !ppBuffer) return E_POINTER;
    *ppBuffer = nullptr;
    return m_device->CreateBuffer(pDesc, pInitialData, ppBuffer);
  }

public:
  ID3D11Device* m_device = nullptr; 
};
