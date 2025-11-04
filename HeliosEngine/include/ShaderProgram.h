#pragma once
#include "Prerequisites.h"
#include "InputLayout.h"
#include <string>
#include <vector>

// DXSDK usa ID3D10Blob (no ID3DBlob)
struct ID3D10Blob;

class Device;
class DeviceContext;

class ShaderProgram {
public:
    ShaderProgram() = default;
    ~ShaderProgram() = default;

    HRESULT init(Device& device,
        const std::string& fileName,
        std::vector<D3D11_INPUT_ELEMENT_DESC> Layout);

    void update();
    void render(DeviceContext& deviceContext);
    void render(DeviceContext& deviceContext, ShaderType type);
    void destroy();

    HRESULT CreateInputLayout(Device& device,
        std::vector<D3D11_INPUT_ELEMENT_DESC> Layout);

    HRESULT CreateShader(Device& device, ShaderType type);
    HRESULT CreateShader(Device& device, ShaderType type, const std::string& fileName);

    // Compilación wide (Unicode) + DXSDK (ID3D10Blob**)
    HRESULT CompileShaderFromFile(LPCWSTR szFileName,
        LPCSTR  szEntryPoint,
        LPCSTR  szShaderModel,
        ID3D10Blob** ppBlobOut);

public:
    ID3D11VertexShader* m_VertexShader = nullptr;
    ID3D11PixelShader* m_PixelShader = nullptr;
    InputLayout         m_inputLayout;

private:
    std::string m_shaderFileName;

    // Blobs compilados (DXSDK)
    ID3D10Blob* m_vertexShaderData = nullptr;
    ID3D10Blob* m_pixelShaderData = nullptr;
};
