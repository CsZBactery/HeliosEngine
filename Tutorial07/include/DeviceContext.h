#pragma once
#include "Prerequisites.h"

// Wrapper mínimo del ID3D11DeviceContext inmediato (no-owing)
class DeviceContext {
public:
  DeviceContext() = default;
  ~DeviceContext() = default;

  // Move-only (evita copias accidentales)
  DeviceContext(DeviceContext&& other) noexcept;
  DeviceContext& operator=(DeviceContext&& other) noexcept;
  DeviceContext(const DeviceContext&) = delete;
  DeviceContext& operator=(const DeviceContext&) = delete;

  // Inicializa obteniendo el contexto inmediato desde un ID3D11Device
  HRESULT initFromDevice(ID3D11Device* device);

  // Adjunta un contexto ya creado (hace AddRef)
  HRESULT attach(ID3D11DeviceContext* ctx);

  // Limpia el estado del pipeline
  void clearState();

  // Libera el contexto (Release)
  void destroy();

  // Enlaza RTV/DSV (OM)
  void OMSetRenderTargets(UINT NumViews,
    ID3D11RenderTargetView* const* ppRenderTargetViews,
    ID3D11DepthStencilView* pDepthStencilView);

  // Acceso crudo si lo necesitas
  ID3D11DeviceContext* get() const { return m_deviceContext; }

private:
  ID3D11DeviceContext* m_deviceContext = nullptr; // no-owing
};
