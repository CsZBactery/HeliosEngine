#pragma once
#include "Prerequisites.h"

class DeviceContext {
public:
  DeviceContext() = default;
  ~DeviceContext() { destroy(); }

  // No copiable (evita doble Release)
  DeviceContext(const DeviceContext&) = delete;
  DeviceContext& operator=(const DeviceContext&) = delete;

  // Movible
  DeviceContext(DeviceContext&& other) noexcept;
  DeviceContext& operator=(DeviceContext&& other) noexcept;

  // Inicializa tomando el contexto inmediato desde un ID3D11Device
  HRESULT initFromDevice(ID3D11Device* device);

  // O bien adjunta un contexto ya creado (AddRef interno)
  HRESULT attach(ID3D11DeviceContext* ctx);

  // Utilidades
  void    update() {}    // placeholder
  void    render() {}    // placeholder
  void    clearState();
  void    destroy();

  // Wrapper del método de D3D11
  void OMSetRenderTargets(
    UINT NumViews,
    ID3D11RenderTargetView* const* ppRenderTargetViews,
    ID3D11DepthStencilView* pDepthStencilView);

  // Acceso crudo si lo necesitas
  ID3D11DeviceContext* get() const { return m_deviceContext; }

private:
  ID3D11DeviceContext* m_deviceContext = nullptr;
};
