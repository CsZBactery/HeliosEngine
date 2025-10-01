#pragma once
#include <d3d11.h>

class DepthStencilView {
public:
  DepthStencilView() = default;
  ~DepthStencilView() { destroy(); }
  DepthStencilView(const DepthStencilView&) = delete;
  DepthStencilView& operator=(const DepthStencilView&) = delete;

  void destroy();

  // Crea la textura de profundidad + DSV (sin MSAA)
  HRESULT create(ID3D11Device* dev, UINT width, UINT height,
    DXGI_FORMAT fmt = DXGI_FORMAT_D24_UNORM_S8_UINT);

  // Estado de depth-stencil (opcional, cache interno)
  HRESULT ensureDepthState(ID3D11Device* dev, bool write = true);

  // Bind a OM junto con un RTV externo
  void bind(ID3D11DeviceContext* ctx, ID3D11RenderTargetView* rtv);

  ID3D11DepthStencilView* dsv()  const { return m_dsv; }
  ID3D11Texture2D* tex()  const { return m_tex; }
  ID3D11DepthStencilState* state()const { return m_state; }

private:
  ID3D11Texture2D* m_tex = nullptr;
  ID3D11DepthStencilView* m_dsv = nullptr;
  ID3D11DepthStencilState* m_state = nullptr;
};
