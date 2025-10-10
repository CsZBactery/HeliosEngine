#include "../include/InputLayout.h"
#include "../include/Device.h"
#include "../include/DeviceContext.h"
                                      
HRESULT InputLayout::init(
  Device& device,
  std::vector<D3D11_INPUT_ELEMENT_DESC>& layout,
  ID3DBlob* vertexShaderData)
{
  if (layout.empty()) return E_INVALIDARG;
  if (!vertexShaderData) return E_POINTER;

  HRESULT hr = device.CreateInputLayout(
    layout.data(),
    static_cast<UINT>(layout.size()),
    vertexShaderData->GetBufferPointer(),
    vertexShaderData->GetBufferSize(),
    &m_inputLayout);

  return hr;
}

void InputLayout::render(DeviceContext& deviceContext)
{
  if (!m_inputLayout) return;
  deviceContext.IASetInputLayout(m_inputLayout);
}

void InputLayout::destroy()
{
  if (m_inputLayout) { m_inputLayout->Release(); m_inputLayout = nullptr; }
}
