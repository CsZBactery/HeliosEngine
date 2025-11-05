#pragma once
#include "Prerequisites.h"
#include "InputLayout.h"

#include <string>
#include <vector>
#include <d3d11.h>
#include <d3dcompiler.h> // Para ID3DBlob y D3DCompile

class Device;
class DeviceContext;

class ShaderProgram {
public:
    ShaderProgram() = default;
    ~ShaderProgram() { destroy(); }

    // Compila desde archivo (.fx/.hlsl)
    HRESULT init(Device& device,
        const std::string& fileName,
        const std::vector<D3D11_INPUT_ELEMENT_DESC>& Layout);

    // Compila desde código HLSL en memoria (opcional, útil si no quieres archivos)
    HRESULT initFromSource(Device& device,
        const std::string& hlslSource,
        const std::vector<D3D11_INPUT_ELEMENT_DESC>& Layout);

    void    update() {}
    void    render(DeviceContext& deviceContext);
    void    render(DeviceContext& deviceContext, ShaderType type);
    void    destroy();

    HRESULT CreateInputLayout(Device& device,
        const std::vector<D3D11_INPUT_ELEMENT_DESC>& Layout);

    HRESULT CreateShader(Device& device, ShaderType type);
    HRESULT CreateShader(Device& device, ShaderType type, const std::string& fileName);

    // Compiladores
    HRESULT CompileShaderFromFile(LPCWSTR szFileName,
        LPCSTR  szEntryPoint,
        LPCSTR  szShaderModel,
        ID3DBlob** ppBlobOut);

    HRESULT CompileShaderFromMemory(LPCSTR  source,
        SIZE_T  length,
        LPCSTR  szEntryPoint,
        LPCSTR  szShaderModel,
        ID3DBlob** ppBlobOut);

public:
    ID3D11VertexShader* m_VertexShader = nullptr;
    ID3D11PixelShader* m_PixelShader = nullptr;
    InputLayout         m_inputLayout;

private:
    std::string m_shaderFileName;

    // Blobs compilados para crear input layout / depurar
    ID3DBlob* m_vertexShaderData = nullptr;
    ID3DBlob* m_pixelShaderData = nullptr;
};
