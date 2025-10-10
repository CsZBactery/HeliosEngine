#include "../include/ShaderProgram.h"
#include "../include/Device.h"
#include "../include/DeviceContext.h"
#include "../include/InputLayout.h"
                                    //  ../include/ //
#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")

static std::wstring ToW(const std::string& s) {
  return std::wstring(s.begin(), s.end());
}

HRESULT ShaderProgram::compileShaderFromFile(
  const wchar_t* szFile, LPCSTR entry, LPCSTR model, ID3DBlob** out)
{
  UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(_DEBUG)
  flags |= D3DCOMPILE_DEBUG;
#endif
  ID3DBlob* errors = nullptr;
  HRESULT hr = D3DCompileFromFile(
    szFile, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
    entry, model, flags, 0, out, &errors);

  if (errors) errors->Release();
  return hr;
}

HRESULT ShaderProgram::init(
  Device& device,
  const std::string& fileName,
  std::vector<D3D11_INPUT_ELEMENT_DESC> layout)
{
  m_shaderFileName = fileName;
  const std::wstring wfile = ToW(fileName);

  // VS
  HRESULT hr = compileShaderFromFile(wfile.c_str(), "VSMain", "vs_5_0", &m_vertexShaderData);
  if (FAILED(hr)) return hr;
  hr = device.CreateVertexShader(
    m_vertexShaderData->GetBufferPointer(),
    m_vertexShaderData->GetBufferSize(),
    nullptr, &m_VertexShader);
  if (FAILED(hr)) return hr;

  // PS
  hr = compileShaderFromFile(wfile.c_str(), "PSMain", "ps_5_0", &m_pixelShaderData);
  if (FAILED(hr)) return hr;
  hr = device.CreatePixelShader(
    m_pixelShaderData->GetBufferPointer(),
    m_pixelShaderData->GetBufferSize(),
    nullptr, &m_PixelShader);
  if (FAILED(hr)) return hr;

  // Si te pasan un InputLayout externo, créalo allí; aquí solo guardamos el blob.
  // (El IA lo setearás a través de InputLayout::init + render)

  return S_OK;
}

void ShaderProgram::render(DeviceContext& deviceContext)
{
  if (m_VertexShader) deviceContext.VSSetShader(m_VertexShader, nullptr, 0);
  if (m_PixelShader)  deviceContext.PSSetShader(m_PixelShader, nullptr, 0);

  if (m_inputLayoutPtr) m_inputLayoutPtr->render(deviceContext);
}

void ShaderProgram::render(DeviceContext& deviceContext, ShaderType type)
{
  if (type == ShaderType::Vertex && m_VertexShader)
    deviceContext.VSSetShader(m_VertexShader, nullptr, 0);
  else if (type == ShaderType::Pixel && m_PixelShader)
    deviceContext.PSSetShader(m_PixelShader, nullptr, 0);
}

void ShaderProgram::destroy()
{
  if (m_VertexShader) { m_VertexShader->Release();    m_VertexShader = nullptr; }
  if (m_PixelShader) { m_PixelShader->Release();     m_PixelShader = nullptr; }
  if (m_vertexShaderData) { m_vertexShaderData->Release(); m_vertexShaderData = nullptr; }
  if (m_pixelShaderData) { m_pixelShaderData->Release();  m_pixelShaderData = nullptr; }
}
