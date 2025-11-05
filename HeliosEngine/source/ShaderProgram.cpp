#include "../include/ShaderProgram.h"
#include "../include/Device.h"
#include "../include/DeviceContext.h"

#include <Windows.h>
#include <d3dcompiler.h>
#include <string>
#include <vector>

#pragma comment(lib, "d3dcompiler.lib")

// Helper: UTF-8 -> UTF-16
static std::wstring ToW(const std::string& s) {
    if (s.empty()) return std::wstring();
    int len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
    std::wstring ws(len ? len - 1 : 0, L'\0');
    if (len > 1) MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, &ws[0], len);
    return ws;
}

// Leer archivo binario a memoria (wide path)
static bool ReadFileToBufferW(const wchar_t* path, std::vector<char>& out) {
    HANDLE h = CreateFileW(path, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) return false;

    LARGE_INTEGER size{};
    if (!GetFileSizeEx(h, &size) || size.QuadPart <= 0) {
        CloseHandle(h);
        return false;
    }
    out.resize(static_cast<size_t>(size.QuadPart));
    DWORD read = 0;
    BOOL ok = ReadFile(h, out.data(), static_cast<DWORD>(out.size()), &read, nullptr);
    CloseHandle(h);
    return ok && read == out.size();
}

HRESULT ShaderProgram::init(Device& device,
    const std::string& fileName,
    std::vector<D3D11_INPUT_ELEMENT_DESC> Layout)
{
    if (!device.m_device) return E_POINTER;
    if (fileName.empty())  return E_INVALIDARG;
    if (Layout.empty())    return E_INVALIDARG;

    m_shaderFileName = fileName;

    // VS
    HRESULT hr = CreateShader(device, ShaderType::VERTEX_SHADER);
    if (FAILED(hr)) return hr;

    // Input layout usando el blob del VS
    hr = CreateInputLayout(device, Layout);
    if (FAILED(hr)) return hr;

    // PS
    hr = CreateShader(device, ShaderType::PIXEL_SHADER);
    return hr;
}

HRESULT ShaderProgram::initFromSource(Device& device,
    const std::string& hlslSource,
    const std::vector<D3D11_INPUT_ELEMENT_DESC>& Layout)
{
    if (!device.m_device)   return E_POINTER;
    if (hlslSource.empty()) return E_INVALIDARG;
    if (Layout.empty())     return E_INVALIDARG;

    // Compilar VS desde memoria
    ID3DBlob* vsBlob = nullptr;
    HRESULT hr = CompileShaderFromMemory(hlslSource.c_str(),
        hlslSource.size(),
        "VS", "vs_4_0", &vsBlob);
    if (FAILED(hr)) return hr;

    // Crear VS (firma de 4 args nativa)
    hr = device.m_device->CreateVertexShader(vsBlob->GetBufferPointer(),
        vsBlob->GetBufferSize(),
        nullptr,
        &m_VertexShader);
    if (FAILED(hr)) { vsBlob->Release(); return hr; }

    // Input layout (InputLayout::init espera vector no-const)
    {
        std::vector<D3D11_INPUT_ELEMENT_DESC> copy = Layout;
        hr = m_inputLayout.init(device, copy, vsBlob);
        if (FAILED(hr)) { vsBlob->Release(); return hr; }
    }

    SAFE_RELEASE(m_vertexShaderData);
    m_vertexShaderData = vsBlob; // opcional conservar

    // Compilar PS desde memoria
    ID3DBlob* psBlob = nullptr;
    hr = CompileShaderFromMemory(hlslSource.c_str(),
        hlslSource.size(),
        "PS", "ps_4_0", &psBlob);
    if (FAILED(hr)) return hr;

    // Crear PS
    hr = device.m_device->CreatePixelShader(psBlob->GetBufferPointer(),
        psBlob->GetBufferSize(),
        nullptr,
        &m_PixelShader);
    if (FAILED(hr)) { psBlob->Release(); return hr; }

    SAFE_RELEASE(m_pixelShaderData);
    m_pixelShaderData = psBlob;

    return S_OK;
}

HRESULT ShaderProgram::CreateInputLayout(Device& device,
    std::vector<D3D11_INPUT_ELEMENT_DESC> Layout)
{
    if (!m_vertexShaderData) return E_POINTER;
    if (!device.m_device)    return E_POINTER;
    if (Layout.empty())      return E_INVALIDARG;

    return m_inputLayout.init(device, Layout, m_vertexShaderData);
}

HRESULT ShaderProgram::CreateShader(Device& device, ShaderType type)
{
    if (!device.m_device)         return E_POINTER;
    if (m_shaderFileName.empty()) return E_INVALIDARG;

    const char* entry = (type == ShaderType::PIXEL_SHADER) ? "PS" : "VS";
    const char* target = (type == ShaderType::PIXEL_SHADER) ? "ps_4_0" : "vs_4_0";

    // Compilar desde archivo (.fx/.hlsl) usando D3DCompile con lectura manual
    ID3DBlob* blob = nullptr;
    std::wstring wpath = ToW(m_shaderFileName);
    HRESULT hr = CompileShaderFromFile(wpath.c_str(), entry, target, &blob);
    if (FAILED(hr)) return hr;

    if (type == ShaderType::PIXEL_SHADER) {
        hr = device.m_device->CreatePixelShader(blob->GetBufferPointer(),
            blob->GetBufferSize(),
            nullptr,
            &m_PixelShader);
    }
    else {
        hr = device.m_device->CreateVertexShader(blob->GetBufferPointer(),
            blob->GetBufferSize(),
            nullptr,
            &m_VertexShader);
    }
    if (FAILED(hr)) { blob->Release(); return hr; }

    if (type == ShaderType::PIXEL_SHADER) {
        SAFE_RELEASE(m_pixelShaderData);
        m_pixelShaderData = blob;
    }
    else {
        SAFE_RELEASE(m_vertexShaderData);
        m_vertexShaderData = blob;
    }
    return S_OK;
}

HRESULT ShaderProgram::CreateShader(Device& device,
    ShaderType type,
    const std::string& fileName)
{
    if (!device.m_device)  return E_POINTER;
    if (fileName.empty())  return E_INVALIDARG;
    m_shaderFileName = fileName;
    return CreateShader(device, type);
}

// --------- Compilar leyendo archivo + D3DCompile (sin D3DCompileFromFile) ---------
HRESULT ShaderProgram::CompileShaderFromFile(const wchar_t* szFileName,
    const char* szEntryPoint,
    const char* szShaderModel,
    ID3DBlob** ppBlobOut)
{
    if (!szFileName || !szEntryPoint || !szShaderModel || !ppBlobOut) return E_INVALIDARG;

    std::vector<char> src;
    if (!ReadFileToBufferW(szFileName, src)) {
        OutputDebugStringW(L"[CompileShaderFromFile] No se pudo leer el archivo.\n");
        return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    }

    UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
    flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    ID3DBlob* pBlob = nullptr;
    ID3DBlob* pErr = nullptr;

    // Nombre simbólico para mensajes (opcional)
    // (no es necesario, puedes pasar nullptr)
    HRESULT hr = D3DCompile(
        src.data(),
        src.size(),
        nullptr,        // pSourceName
        nullptr,        // macros
        nullptr,        // include (si usas #include, luego te paso un ID3DInclude)
        szEntryPoint,
        szShaderModel,
        flags, 0,
        &pBlob, &pErr);

    if (FAILED(hr)) {
        if (pErr) {
            OutputDebugStringA((const char*)pErr->GetBufferPointer());
            pErr->Release();
        }
        return hr;
    }
    if (pErr) pErr->Release();

    *ppBlobOut = pBlob;
    return S_OK;
}

// --------- Compilar desde memoria ---------
HRESULT ShaderProgram::CompileShaderFromMemory(const char* source, size_t length,
    const char* szEntryPoint,
    const char* szShaderModel,
    ID3DBlob** ppBlobOut)
{
    if (!ppBlobOut || !source || length == 0) return E_INVALIDARG;

    UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
    flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    ID3DBlob* pBlob = nullptr;
    ID3DBlob* pErr = nullptr;

    HRESULT hr = D3DCompile(source, length, nullptr, nullptr, nullptr,
        szEntryPoint, szShaderModel, flags, 0,
        &pBlob, &pErr);
    if (FAILED(hr)) {
        if (pErr) { OutputDebugStringA((const char*)pErr->GetBufferPointer()); pErr->Release(); }
        return hr;
    }
    if (pErr) pErr->Release();

    *ppBlobOut = pBlob;
    return S_OK;
}

void ShaderProgram::render(DeviceContext& deviceContext)
{
    if (!m_VertexShader || !m_PixelShader || !m_inputLayout.m_inputLayout) return;
    m_inputLayout.render(deviceContext);
    deviceContext.m_deviceContext->VSSetShader(m_VertexShader, nullptr, 0);
    deviceContext.m_deviceContext->PSSetShader(m_PixelShader, nullptr, 0);
}

void ShaderProgram::render(DeviceContext& deviceContext, ShaderType type)
{
    if (!deviceContext.m_deviceContext) return;
    switch (type) {
    case VERTEX_SHADER: deviceContext.m_deviceContext->VSSetShader(m_VertexShader, nullptr, 0); break;
    case PIXEL_SHADER:  deviceContext.m_deviceContext->PSSetShader(m_PixelShader, nullptr, 0);  break;
    default: break;
    }
}

void ShaderProgram::destroy()
{
    if (m_VertexShader || m_PixelShader) {
        // por si acaso limpias estado antes
    }
    SAFE_RELEASE(m_VertexShader);
    m_inputLayout.destroy();
    SAFE_RELEASE(m_PixelShader);
    SAFE_RELEASE(m_vertexShaderData);
    SAFE_RELEASE(m_pixelShaderData);
}
