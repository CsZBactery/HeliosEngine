#pragma once
#include "Prerequisites.h"
#include "InputLayout.h"
#include <string>
#include <vector>

// Forward correcto (NO uses ID3D10Blob)

class Device;
class DeviceContext;

class ShaderProgram {
public:
    ShaderProgram() = default;
    ~ShaderProgram() = default;

    // Carga desde archivo .fx/.hlsl
    HRESULT init(Device& device,
        const std::string& fileName,
        std::vector<D3D11_INPUT_ELEMENT_DESC> Layout);

    // Alternativa: compilar desde string HLSL (sin archivos)
    HRESULT initFromSource(Device& device,
        const std::string& hlslSource,
        const std::vector<D3D11_INPUT_ELEMENT_DESC>& Layout);

    void    render(DeviceContext& deviceContext);
    void    render(DeviceContext& deviceContext, ShaderType type);
    void    destroy();

    HRESULT CreateInputLayout(Device& device,
        std::vector<D3D11_INPUT_ELEMENT_DESC> Layout);

    HRESULT CreateShader(Device& device, ShaderType type);
    HRESULT CreateShader(Device& device, ShaderType type, const std::string& fileName);

    // Compiladores (Windows SDK, sin D3DX)
    HRESULT CompileShaderFromFile(const wchar_t* szFileName,
        const char* szEntryPoint,
        const char* szShaderModel,
        ID3DBlob** ppBlobOut);

    HRESULT CompileShaderFromMemory(const char* source, size_t length,
        const char* szEntryPoint,
        const char* szShaderModel,
        ID3DBlob** ppBlobOut);

public:
    ID3D11VertexShader* m_VertexShader = nullptr;
    ID3D11PixelShader* m_PixelShader = nullptr;
    InputLayout         m_inputLayout;

private:
    std::string m_shaderFileName;
    ID3DBlob* m_vertexShaderData = nullptr; // blob VS (para input layout)
    ID3DBlob* m_pixelShaderData = nullptr; // blob PS (opcional)
};
