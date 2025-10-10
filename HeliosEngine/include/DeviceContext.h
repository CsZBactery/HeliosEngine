#pragma once
/**
 * @file DeviceContext.h
 * @brief Wrapper mínimo (move-only) del contexto inmediato de D3D11.
 *
 * Depende de: Prerequisites.h -> <d3d11.h>
 */

#include "Prerequisites.h"

class DeviceContext {
public:
  DeviceContext() = default;
  ~DeviceContext() = default;

  // Move-only
  DeviceContext(DeviceContext&& other) noexcept;
  DeviceContext& operator=(DeviceContext&& other) noexcept;
  DeviceContext(const DeviceContext&) = delete;
  DeviceContext& operator=(const DeviceContext&) = delete;

  // Adquisición / gestión
  HRESULT initFromDevice(ID3D11Device* device);           // obtiene el inmediato (AddRef interno)
  HRESULT attach(ID3D11DeviceContext* ctx);               // adjunta un contexto existente (AddRef)
  void    clearState();                                   // ClearState()
  void    destroy();                                      // Release()

  // ---------- OUTPUT-MERGER ----------
  void OMSetRenderTargets(UINT NumViews,
    ID3D11RenderTargetView* const* ppRenderTargetViews,
    ID3D11DepthStencilView* pDepthStencilView);

  // ---------- RASTERIZER ----------
  void RSSetViewports(UINT NumViewports, const D3D11_VIEWPORT* pViewports);
  void RSSetState(ID3D11RasterizerState* pRasterizerState);

  // ---------- INPUT ASSEMBLER ----------
  void IASetInputLayout(ID3D11InputLayout* pInputLayout);
  void IASetVertexBuffers(UINT StartSlot, UINT NumBuffers,
    ID3D11Buffer* const* ppVertexBuffers,
    const UINT* pStrides, const UINT* pOffsets);
  void IASetIndexBuffer(ID3D11Buffer* pIndexBuffer,
    DXGI_FORMAT Format, UINT Offset);
  void IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY Topology);

  // ---------- VERTEX SHADER ----------
  void VSSetShader(ID3D11VertexShader* pVS,
    ID3D11ClassInstance* const* ppClassInstances,
    UINT NumClassInstances);
  void VSSetConstantBuffers(UINT StartSlot, UINT NumBuffers,
    ID3D11Buffer* const* ppConstantBuffers);

  // ---------- PIXEL SHADER ----------
  void PSSetShader(ID3D11PixelShader* pPS,
    ID3D11ClassInstance* const* ppClassInstances,
    UINT NumClassInstances);
  void PSSetConstantBuffers(UINT StartSlot, UINT NumBuffers,
    ID3D11Buffer* const* ppConstantBuffers);
  void PSSetShaderResources(UINT StartSlot, UINT NumViews,
    ID3D11ShaderResourceView* const* ppShaderResourceViews);
  void PSSetSamplers(UINT StartSlot, UINT NumSamplers,
    ID3D11SamplerState* const* ppSamplers);

  // ---------- Actualizaciones / Draw ----------
  void UpdateSubresource(ID3D11Resource* pDstResource, UINT DstSubresource,
    const D3D11_BOX* pDstBox, const void* pSrcData,
    UINT SrcRowPitch, UINT SrcDepthPitch);

  void DrawIndexed(UINT IndexCount, UINT StartIndexLocation, INT BaseVertexLocation);

  // Acceso crudo
  ID3D11DeviceContext* get() const { return m_deviceContext; }

private:
  ID3D11DeviceContext* m_deviceContext = nullptr; 
};
