#include "../include/ShaderProgram.h"
#include "../include/Device.h"
#include "../include/DeviceContext.h"

#include <Windows.h>
#include <d3dcompiler.h>  // DXSDK: define ID3D10Blob
#include <d3dx11.h>
#include <string>
#include <vector>

// Helper local: string (UTF-8/ANSI) -> wstring (UTF-16)
static std::wstring ToW(const std::string& s) {
    if (s.empty()) return std::wstring();
    int len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
    std::wstring ws(len ? len - 1 : 0, L'\0');
    if (len > 1) MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, &ws[0], len);
    return ws;
}

HRESULT ShaderProgram::init(Device& device,
    const std::string& fileName,
    std::vector<D3D11_INPUT_ELEMENT_DESC> Layout)
{
    if (!device.m_device) { ERROR("ShaderProgram", "init", "Device is null."); return E_POINTER; }
    if (fileName.empty()) { ERROR("ShaderProgram", "init", "File name is empty."); return E_INVALIDARG; }
    if (Layout.empty()) { ERROR("ShaderProgram", "init", "InputLayout is empty."); return E_INVALIDARG; }

    m_shaderFileName = fileName;

    // VS
    HRESULT hr = CreateShader(device, ShaderType::VERTEX_SHADER);
    if (FAILED(hr)) { ERROR("ShaderProgram", "init", "Failed to create vertex shader."); return hr; }

    // InputLayout (usa blob del VS)
    hr = CreateInputLayout(device, Layout);
    if (FAILED(hr)) { ERROR("ShaderProgram", "init", "Failed to create input layout."); return hr; }

    // PS
    hr = CreateShader(device, ShaderType::PIXEL_SHADER);
    if (FAILED(hr)) { ERROR("ShaderProgram", "init", "Failed to create pixel shader."); return hr; }

    return S_OK;
}

HRESULT ShaderProgram::CreateInputLayout(Device& device,
    std::vector<D3D11_INPUT_ELEMENT_DESC> Layout)
{
    if (!m_vertexShaderData) { ERROR("ShaderProgram", "CreateInputLayout", "VertexShader data is null."); return E_POINTER; }
    if (!device.m_device) { ERROR("ShaderProgram", "CreateInputLayout", "Device is null."); return E_POINTER; }
    if (Layout.empty()) { ERROR("ShaderProgram", "CreateInputLayout", "Input layout is empty."); return E_INVALIDARG; }

    // Asegúrate de que InputLayout::init reciba ID3D10Blob*
    HRESULT hr = m_inputLayout.init(device, Layout, m_vertexShaderData);
    SAFE_RELEASE(m_vertexShaderData);
    if (FAILED(hr)) { ERROR("ShaderProgram", "CreateInputLayout", "Failed to create input layout."); return hr; }
    return S_OK;
}

HRESULT ShaderProgram::CreateShader(Device& device, ShaderType type)
{
    if (!device.m_device) { ERROR("ShaderProgram", "CreateShader", "Device is null"); return E_POINTER; }
    if (m_shaderFileName.empty()) { ERROR("ShaderProgram", "CreateShader", "Shader file name is empty"); return E_INVALIDARG; }

    const char* entry = (type == ShaderType::PIXEL_SHADER) ? "PS" : "VS";
    const char* target = (type == ShaderType::PIXEL_SHADER) ? "ps_4_0" : "vs_4_0";

    ID3D10Blob* shaderData = nullptr;
    std::wstring wpath = ToW(m_shaderFileName);

    HRESULT hr = CompileShaderFromFile(wpath.c_str(), entry, target, &shaderData);
    if (FAILED(hr)) { ERROR("ShaderProgram", "CreateShader", "Failed to compile shader from file."); return hr; }

    // Tu Device wrapper expone la firma nativa (4 args) -> pasa nullptr como linkage
    if (type == ShaderType::PIXEL_SHADER) {
        hr = device.CreatePixelShader(shaderData->GetBufferPointer(),
            shaderData->GetBufferSize(),
            nullptr,               // ID3D11ClassLinkage*
            &m_PixelShader);
    }
    else {
        hr = device.CreateVertexShader(shaderData->GetBufferPointer(),
            shaderData->GetBufferSize(),
            nullptr,               // ID3D11ClassLinkage*
            &m_VertexShader);
    }

    if (FAILED(hr)) {
        ERROR("ShaderProgram", "CreateShader", "Failed to create shader object from compiled data.");
        shaderData->Release();
        return hr;
    }

    if (type == ShaderType::PIXEL_SHADER) {
        SAFE_RELEASE(m_pixelShaderData);
        m_pixelShaderData = shaderData;
    }
    else {
        SAFE_RELEASE(m_vertexShaderData);
        m_vertexShaderData = shaderData;
    }
    return S_OK;
}

HRESULT ShaderProgram::CreateShader(Device& device,
    ShaderType type,
    const std::string& fileName)
{
    if (!device.m_device) { ERROR("ShaderProgram", "init", "Device is null"); return E_POINTER; }
    if (fileName.empty()) { ERROR("ShaderProgram", "init", "File name is empty."); return E_INVALIDARG; }
    m_shaderFileName = fileName;
    HRESULT hr = CreateShader(device, type);
    if (FAILED(hr)) { ERROR("ShaderProgram", "CreateShader", "Failed to create shader from file."); return hr; }
    return S_OK;
}

// FIRMA WIDE + DXSDK (ID3D10Blob**)
HRESULT ShaderProgram::CompileShaderFromFile(LPCWSTR szFileName,
    LPCSTR  szEntryPoint,
    LPCSTR  szShaderModel,
    ID3D10Blob** ppBlobOut)
{
    if (!ppBlobOut) return E_POINTER;

    UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(_DEBUG)
    flags |= D3DCOMPILE_DEBUG;
#endif

    ID3D10Blob* pErrorBlob = nullptr;
    ID3D10Blob* pBlob = nullptr;

    HRESULT hr = D3DX11CompileFromFileW(
        szFileName,
        nullptr, nullptr,
        szEntryPoint,
        szShaderModel,
        flags, 0,
        nullptr,      // ID3DX11ThreadPump*
        &pBlob,
        &pErrorBlob,
        nullptr);

    if (FAILED(hr)) {
        if (pErrorBlob) {
            OutputDebugStringA(static_cast<const char*>(pErrorBlob->GetBufferPointer()));
        }
        SAFE_RELEASE(pErrorBlob);
        return hr;
    }

    *ppBlobOut = pBlob;
    SAFE_RELEASE(pErrorBlob);
    return S_OK;
}

void ShaderProgram::render(DeviceContext& deviceContext)
{
    if (!m_VertexShader || !m_PixelShader || !m_inputLayout.m_inputLayout) {
        ERROR("ShaderPRogram", "render", "Shader or InputLayout not initialized");
        return;
    }
    m_inputLayout.render(deviceContext);
    deviceContext.m_deviceContext->VSSetShader(m_VertexShader, nullptr, 0);
    deviceContext.m_deviceContext->PSSetShader(m_PixelShader, nullptr, 0);
}

void ShaderProgram::render(DeviceContext& deviceContext, ShaderType type)
{
    if (!deviceContext.m_deviceContext) { ERROR("RenderTargetView", "render", "DeviceContext is nullptr"); return; }
    switch (type) {
    case VERTEX_SHADER: deviceContext.m_deviceContext->VSSetShader(m_VertexShader, nullptr, 0); break;
    case PIXEL_SHADER:  deviceContext.m_deviceContext->PSSetShader(m_PixelShader, nullptr, 0);  break;
    default: break;
    }
}

void ShaderProgram::destroy()
{
    SAFE_RELEASE(m_VertexShader);
    m_inputLayout.destroy();
    SAFE_RELEASE(m_PixelShader);
    SAFE_RELEASE(m_vertexShaderData);
    SAFE_RELEASE(m_pixelShaderData);
}

void ShaderProgram::update() {}
