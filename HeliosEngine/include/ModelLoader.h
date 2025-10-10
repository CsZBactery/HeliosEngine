#pragma once
#include "Prerequisites.h"
#include <string>
#include <vector>
#include <unordered_map>

class Device;
class DeviceContext;

struct VertexPNCT {
  DirectX::XMFLOAT3 pos;
  DirectX::XMFLOAT3 nrm;
  DirectX::XMFLOAT2 uv;
};

class Model {
public:
  bool isValid() const { return m_vb && m_ib && m_indexCount > 0; }
  void bind(DeviceContext& ctx, UINT slot = 0, UINT stride = sizeof(VertexPNCT), UINT offset = 0);
  void draw(DeviceContext& ctx);

  void destroy();

private:
  friend class ModelLoader;
  ID3D11Buffer* m_vb = nullptr;
  ID3D11Buffer* m_ib = nullptr;
  UINT m_indexCount = 0;
};

class ModelLoader {
public:
  // Carga un OBJ básico (v / vt / vn) y crea VB/IB
  static HRESULT LoadOBJ(
    Device& device,
    const std::string& path,
    Model& outModel);
};
