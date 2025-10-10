#pragma once
#include "Prerequisites.h"
#include <vector>

class Device;
class DeviceContext;

class InputLayout {
public:
  InputLayout() = default;
  ~InputLayout() = default;

  HRESULT init(
    Device& device,
    std::vector<D3D11_INPUT_ELEMENT_DESC>& layout,
    ID3DBlob* vertexShaderData);

  void update() {}
  void render(DeviceContext& deviceContext);
  void destroy();

public:
  ID3D11InputLayout* m_inputLayout = nullptr;
};
