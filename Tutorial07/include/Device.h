#pragma once
#include "Prerequisites.h"

// Wrapper mínimo de ID3D11Device
class Device {
public:
  Device() = default;
  ~Device() = default;

  // No copiable
  Device(const Device&) = delete;
  Device& operator=(const Device&) = delete;

  // Libera el dispositivo
  void destroy();

  // Thin wrapper a CreateRenderTargetView
  HRESULT CreateRenderTargetView(ID3D11Resource* pResource,
    const D3D11_RENDER_TARGET_VIEW_DESC* pDesc,
    ID3D11RenderTargetView** ppRTView);

public: // <- público para que puedas usar g_device.m_device
  ID3D11Device* m_device = nullptr;
};
