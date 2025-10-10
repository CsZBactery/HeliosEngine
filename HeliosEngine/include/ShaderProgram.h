#pragma once
#include "Prerequisites.h"
#include <string>
#include <vector>

class Device;
class DeviceContext;
class InputLayout;

enum class ShaderType { Vertex, Pixel };

class ShaderProgram {
public:
  ShaderProgram() = default;
  ~ShaderProgram() = default;

  // Carga y compila VS/PS del mismo archivo (entry VSMain/PSMain por defecto)
  HRESULT init(
    Device& device,
    const std::string& fileName,
    std::vector<D3D11_INPUT_ELEMENT_DESC> layout // se usa para crear IA
  );

  void update() {}
  void render(DeviceContext& deviceContext);
  void render(DeviceContext& deviceContext, ShaderType type);
  void destroy();

  // Acceso a blobs por si necesitas reflección o IA custom
  ID3DBlob* getVSBlob() const { return m_vertexShaderData; }

public:
  ID3D11VertexShader* m_VertexShader = nullptr;
  ID3D11PixelShader* m_PixelShader = nullptr;

  InputLayout* m_inputLayoutPtr = nullptr; // no-ownership

private:
  std::string m_shaderFileName;
  ID3DBlob* m_vertexShaderData = nullptr;
  ID3DBlob* m_pixelShaderData = nullptr;

private:
  HRESULT compileShaderFromFile(
    const wchar_t* szFile,
    LPCSTR entryPoint,
    LPCSTR shaderModel,
    ID3DBlob** ppBlobOut);
};
