#pragma once
#include "Prerequisites.h"

class
  Device {
public:
  Device() = default;
  ~Device() = default;

  void
    init();

  void
    uptade();

  void
    render();

  void
    destroy();

  HRESULT
    CreateRenderTargetView(ID3D11Resource* pResource,
                           const D3D11_RENDER_TARGET_VIEW_DESC* pDesc,
                           ID3D11RenderTargetView** ppRTView);

public:
/**
* @brief Puntero al dispositivo Direct3D 11.
* Details Creado en init(), liberado en destroy().
*/
  ID3D11Device* m_device = nullptr;
 };

