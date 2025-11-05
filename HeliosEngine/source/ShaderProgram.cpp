#include "../include/ShaderProgram.h"
#include "../include/Device.h"
#include "../include/DeviceContext.h"

#include <Windows.h>
#include <d3dcompiler.h>  // ID3DBlob, D3DCompile
#include <d3dx11.h>       // D3DX11CompileFromFileW
#include <string>
#include <vector>

// Helper: UTF-8 -> UTF-16
static std::wstring ToW(const std::string& s) {
    if (s.empty()) return std::wstring();
    int len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
    std::wstring ws(len ? len - 1 : 0, L'\0');
    if (len > 1) MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, &ws[0], len);
    return ws;
}

// =====================
// init() desde archivo
// =====================
HRESULT ShaderProgram::init(Device& device,
    const std::string& fileName,
    const std::vector<D3D11_INPUT_ELEMENT_DESC>& Layout) {
    if (!device.m_device) { ERROR("ShaderProgram", "init", "Device is null.");          return E_POINTER; }
    if (fileName.empty()) { ERROR("ShaderProgram", "init", "File name is empty.");      return E_INVALIDARG; }
    if (Layout.empty()) { ERROR("ShaderProgram", "init", "InputLayout is empty.");    return E_INVALIDARG; }

    m_shaderFileName = fileName;

    // VS
    HRESULT hr = CreateShader(device, ShaderType::VERTEX_SHADER);
    if (FAILED(hr)) { ERROR("ShaderProgram", "init", "Failed to create vertex shader.");  return hr; }

    // InputLayout usando el blob del VS
    hr = CreateInputLayout(device, Layout);
    if (FAILED(hr)) { ERROR("ShaderProgram", "init", "Failed to create input layout.");   return hr; }

    // PS
    hr = CreateShader(device, ShaderType::PIXEL_SHADER);
    if (FAILED(hr)) { ERROR("ShaderProgram", "init", "Failed to create pixel shader.");   return hr; }

    return S_OK;
}

// =========================
// initFromSource() en RAM
// =========================
HRESULT ShaderProgram::initFromSource(Device& device,
    const std::string& hlslSource,
    const std::vector<D3D11_INPUT_ELEMENT_DESC>& Layout) {
    if (!device.m_device) { ERROR("ShaderProgram", "initFromSource", "Device is null.");          return E_POINTER; }
    if (hlslSource.empty()) { ERROR("ShaderProgram", "initFromSource", "HLSL source is empty.");    return E_INVALIDARG; }
    if (Layout.empty()) { ERROR("ShaderProgram", "initFromSource", "InputLayout is empty.");    return E_INVALIDARG; }

    // Compilar VS
    ID3DBlob* vsBlob = nullptr;
    HRESULT hr = CompileShaderFromMemory(hlslSource.c_str(),
        hlslSource.size(),
        "VS", "vs_4_0",
        &vsBlob);
    if (FAILED(hr)) { ERROR("ShaderProgram", "initFromSource", "VS compile failed."); return hr; }

    // Crear VS (firma nativa)
    hr = device.m_device->CreateVertexShader(vsBlob->GetBufferPointer(),
        vsBlob->GetBufferSize(),
        nullptr,
        &m_VertexShader);
    if (FAILED(hr)) {
        vsBlob->Release();
        ERROR("ShaderProgram", "initFromSource", "CreateVertexShader failed.");
        return hr;
    }

    // InputLayout usando vsBlob (copia porque tu InputLayout::init pide vector no-const)
    {
        std::vector<D3D11_INPUT_ELEMENT_DESC> layoutCopy = Layout;
        HRESULT ilhr = m_inputLayout.init(device, layoutCopy, vsBlob);
        if (FAILED(ilhr)) {
            vsBlob->Release();
            ERROR("ShaderProgram", "initFromSource", "InputLayout init failed.");
            return ilhr;
        }
    }

    SAFE_RELEASE(m_vertexShaderData);
    m_vertexShaderData = vsBlob; // opcional conservar para debug; si no, libera

    // Compilar PS
    ID3DBlob* psBlob = nullptr;
    hr = CompileShaderFromMemory(hlslSource.c_str(),
        hlslSource.size(),
        "PS", "ps_4_0",
        &psBlob);
    if (FAILED(hr)) { ERROR("ShaderProgram", "initFromSource", "PS compile failed."); return hr; }

    // Crear PS (firma nativa)
    hr = device.m_device->CreatePixelShader(psBlob->GetBufferPointer(),
        psBlob->GetBufferSize(),
        nullptr,
        &m_PixelShader);
    if (FAILED(hr)) {
        psBlob->Release();
        ERROR("ShaderProgram", "initFromSource", "CreatePixelShader failed.");
        return hr;
    }

    SAFE_RELEASE(m_pixelShaderData);
    m_pixelShaderData = psBlob; // opcional conservar para debug

    return S_OK;
}

// =======================
// CreateInputLayout()
// =======================
HRESULT ShaderProgram::CreateInputLayout(Device& device,
    const std::vector<D3D11_INPUT_ELEMENT_DESC>& Layout) {
    if (!m_vertexShaderData) { ERROR("ShaderProgram", "CreateInputLayout", "VertexShader data is null."); return E_POINTER; }
    if (!device.m_device) { ERROR("ShaderProgram", "CreateInputLayout", "Device is null.");            return E_POINTER; }
    if (Layout.empty()) { ERROR("ShaderProgram", "CreateInputLayout", "Input layout is empty.");     return E_INVALIDARG; }

    std::vector<D3D11_INPUT_ELEMENT_DESC> layoutCopy = Layout;
    HRESULT hr = m_inputLayout.init(device, layoutCopy, m_vertexShaderData);
    if (FAILED(hr)) { ERROR("ShaderProgram", "CreateInputLayout", "Failed to create input layout."); return hr; }
    return S_OK;
}

// =======================
// CreateShader() archivo
// =======================
HRESULT ShaderProgram::CreateShader(Device& device, ShaderType type) {
    if (!device.m_device) { ERROR("ShaderProgram", "CreateShader", "Device is null");             return E_POINTER; }
    if (m_shaderFileName.empty()) { ERROR("ShaderProgram", "CreateShader", "Shader file name is empty"); return E_INVALIDARG; }

    const char* entry = (type == ShaderType::PIXEL_SHADER) ? "PS" : "VS";
    const char* target = (type == ShaderType::PIXEL_SHADER) ? "ps_4_0" : "vs_4_0";

    ID3DBlob* shaderData = nullptr;
    std::wstring wpath = ToW(m_shaderFileName);

    HRESULT hr = CompileShaderFromFile(wpath.c_str(), entry, target, &shaderData);
    if (FAILED(hr)) {
        ERROR("ShaderProgram", "CreateShader", "Failed to compile shader from file.");
        return hr;
    }

    if (type == ShaderType::PIXEL_SHADER) {
        hr = device.m_device->CreatePixelShader(shaderData->GetBufferPointer(),
            shaderData->GetBufferSize(),
            nullptr,
            &m_PixelShader);
    }
    else {
        hr = device.m_device->CreateVertexShader(shaderData->GetBufferPointer(),
            shaderData->GetBufferSize(),
            nullptr,
            &m_VertexShader);
    }

    if (FAILED(hr)) {
        shaderData->Release();
        ERROR("ShaderProgram", "CreateShader", "Failed to create shader object.");
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
    const std::string& fileName) {
    if (!device.m_device) { ERROR("ShaderProgram", "CreateShader(file)", "Device is null");     return E_POINTER; }
    if (fileName.empty()) { ERROR("ShaderProgram", "CreateShader(file)", "File name is empty."); return E_INVALIDARG; }
    m_shaderFileName = fileName;
    return CreateShader(device, type);
}

// =======================
// Compilar desde archivo
// =======================
HRESULT ShaderProgram::CompileShaderFromFile(LPCWSTR szFileName,
    LPCSTR  szEntryPoint,
    LPCSTR  szShaderModel,
    ID3DBlob** ppBlobOut) {
    if (!ppBlobOut) return E_POINTER;

    UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
    flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    ID3DBlob* pBlob = nullptr;
    ID3DBlob* pErr = nullptr;

    HRESULT hr = D3DX11CompileFromFileW(
        szFileName,
        nullptr, nullptr,
        szEntryPoint,
        szShaderModel,
        flags, 0,
        nullptr,
        &pBlob,
        &pErr,
        nullptr);

    if (FAILED(hr)) {
        if (pErr) { OutputDebugStringA((const char*)pErr->GetBufferPointer()); pErr->Release(); }
        return hr;
    }
    if (pErr) pErr->Release();

    *ppBlobOut = pBlob;
    return S_OK;
}

// =======================
// Compilar desde memoria
// =======================
HRESULT ShaderProgram::CompileShaderFromMemory(LPCSTR  source,
    SIZE_T  length,
    LPCSTR  szEntryPoint,
    LPCSTR  szShaderModel,
    ID3DBlob** ppBlobOut) {
    if (!ppBlobOut || !source || length == 0) return E_INVALIDARG;

    UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
    flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    ID3DBlob* pBlob = nullptr;
    ID3DBlob* pErr = nullptr;

    HRESULT hr = D3DCompile(
        source, length,
        nullptr,      // optional source name
        nullptr,      // macros
        nullptr,      // include
        szEntryPoint,
        szShaderModel,
        flags, 0,
        &pBlob,
        &pErr);

    if (FAILED(hr)) {
        if (pErr) { OutputDebugStringA((const char*)pErr->GetBufferPointer()); pErr->Release(); }
        return hr;
    }
    if (pErr) pErr->Release();

    *ppBlobOut = pBlob;
    return S_OK;
}

// =======================
void ShaderProgram::render(DeviceContext& deviceContext) {
    if (!m_VertexShader || !m_PixelShader || !m_inputLayout.m_inputLayout) {
        ERROR("ShaderProgram", "render", "Shader or InputLayout not initialized");
        return;
    }
    m_inputLayout.render(deviceContext);
    deviceContext.m_deviceContext->VSSetShader(m_VertexShader, nullptr, 0);
    deviceContext.m_deviceContext->PSSetShader(m_PixelShader, nullptr, 0);
}

void ShaderProgram::render(DeviceContext& deviceContext, ShaderType type) {
    if (!deviceContext.m_deviceContext) { ERROR("RenderTargetView", "render", "DeviceContext is nullptr"); return; }
    switch (type) {
    case VERTEX_SHADER: deviceContext.m_deviceContext->VSSetShader(m_VertexShader, nullptr, 0); break;
    case PIXEL_SHADER:  deviceContext.m_deviceContext->PSSetShader(m_PixelShader, nullptr, 0); break;
    default: break;
    }
}

void ShaderProgram::destroy() {
    SAFE_RELEASE(m_VertexShader);
    m_inputLayout.destroy();
    SAFE_RELEASE(m_PixelShader);
    SAFE_RELEASE(m_vertexShaderData);
    SAFE_RELEASE(m_pixelShaderData);
}
