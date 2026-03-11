// ======================================================================================
// Archivo: ShaderProgram.cpp
// Implementación de la compilación, creación y vinculación de Vertex y Pixel Shaders.
// ======================================================================================

#include "ShaderProgram.h"
#include "Device.h"
#include "DeviceContext.h"
#include "EngineUtilities/Utilities/LayoutBuilder.h"

// Inicializa el programa compilando el Vertex y Pixel shader desde el mismo archivo
HRESULT
ShaderProgram::init(Device& device,
    const std::string& fileName,
    LayoutBuilder layoutBuilder) {
    if (!device.m_device) {
        ERROR("ShaderProgram", "init", "Device is null.");
        return E_POINTER;
    }
    if (fileName.empty()) {
        ERROR("ShaderProgram", "init", "File name is empty.");
        return E_INVALIDARG;
    }

    m_shaderFileName = fileName;

    // 1. Creación del Vertex Shader
    HRESULT hr = CreateShader(device, ShaderType::VERTEX_SHADER);
    if (FAILED(hr)) {
        ERROR("ShaderProgram", "init", "Failed to create vertex shader.");
        return hr;
    }

    // 2. Creación del Input Layout (Usando el builder y la firma del VS compilado)
    hr = CreateInputLayout(device, layoutBuilder);
    if (FAILED(hr)) {
        ERROR("ShaderProgram", "init", "Failed to create input layout.");
        return hr;
    }

    // 3. Creación del Pixel Shader
    hr = CreateShader(device, ShaderType::PIXEL_SHADER);
    if (FAILED(hr)) {
        ERROR("ShaderProgram", "init", "Failed to create pixel shader.");
        return hr;
    }

    return hr;
}

// Crea el "puente" de datos entre los vértices en C++ y las variables de entrada en HLSL
HRESULT
ShaderProgram::CreateInputLayout(Device& device, LayoutBuilder layoutBuilder) {
    if (!m_vertexShaderData) {
        ERROR("ShaderProgram", "CreateInputLayout", "Vertex shader data is null. Compile VS first.");
        return E_POINTER;
    }
    if (!device.m_device) {
        ERROR("ShaderProgram", "CreateInputLayout", "Device is null.");
        return E_POINTER;
    }

    // Obtenemos el vector subyacente del Builder
    auto& layout = layoutBuilder.Get();

    HRESULT hr = m_inputLayout.init(device, layout.data(), (UINT)layout.size(), m_vertexShaderData);

    // El blob del shader ya no se necesita para validar layouts, liberamos memoria
    SAFE_RELEASE(m_vertexShaderData);

    if (FAILED(hr)) {
        ERROR("ShaderProgram", "CreateInputLayout", "Failed to create input layout.");
        return hr;
    }

    return hr;
}

// Lógica principal para pedir a DirectX que convierta el HLSL en un objeto Shader
HRESULT
ShaderProgram::CreateShader(Device& device, ShaderType type) {
    if (!device.m_device) {
        ERROR("ShaderProgram", "CreateShader", "Device is null.");
        return E_POINTER;
    }
    if (m_shaderFileName.empty()) {
        ERROR("ShaderProgram", "CreateShader", "Shader file name is empty.");
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    ID3DBlob* shaderData = nullptr;

    // Define los puntos de entrada (main functions) según el tipo de shader
    const char* shaderEntryPoint = (type == ShaderType::PIXEL_SHADER) ? "PS" : "VS";
    // Usa Shader Model 5.0 (Requerido para PBR y texturas avanzadas en D3D11)
    const char* shaderModel = (type == ShaderType::PIXEL_SHADER) ? "ps_5_0" : "vs_5_0";

    // Compilación del archivo de texto (.hlsl o .fx) a binario
    hr = CompileShaderFromFile(const_cast<char*>(m_shaderFileName.data()),
        shaderEntryPoint,
        shaderModel,
        &shaderData);

    if (FAILED(hr)) {
        ERROR("ShaderProgram", "CreateShader", "Failed to compile shader from file: %s", m_shaderFileName.c_str());
        return hr;
    }

    // Creación del objeto COM en la GPU
    if (type == PIXEL_SHADER) {
        hr = device.CreatePixelShader(shaderData->GetBufferPointer(),
            shaderData->GetBufferSize(),
            nullptr,
            &m_PixelShader);
    }
    else {
        hr = device.CreateVertexShader(shaderData->GetBufferPointer(),
            shaderData->GetBufferSize(),
            nullptr,
            &m_VertexShader);
    }

    if (FAILED(hr)) {
        ERROR("ShaderProgram", "CreateShader", "Failed to create shader object from compiled data.");
        SAFE_RELEASE(shaderData);
        return hr;
    }

    // Guardar el binario (Blob) para usos futuros (como crear el Input Layout)
    if (type == PIXEL_SHADER) {
        SAFE_RELEASE(m_pixelShaderData);
        m_pixelShaderData = shaderData;
    }
    else {
        SAFE_RELEASE(m_vertexShaderData);
        m_vertexShaderData = shaderData;
    }

    return S_OK;
}

// Sobrecarga: Compila un solo tipo de shader si no se desea el pipeline completo
HRESULT
ShaderProgram::CreateShader(Device& device, ShaderType type, const std::string& fileName) {
    if (!device.m_device) {
        ERROR("ShaderProgram", "init", "Device is null.");
        return E_POINTER;
    }
    if (fileName.empty()) {
        ERROR("ShaderProgram", "init", "File name is empty.");
        return E_INVALIDARG;
    }

    m_shaderFileName = fileName;
    HRESULT hr = CreateShader(device, type);

    if (FAILED(hr)) {
        ERROR("ShaderProgram", "CreateShader", "Failed to Create shader from file: %s", m_shaderFileName.c_str());
        return hr;
    }

    return S_OK;
}

// Función de bajo nivel para invocar al compilador de DirectX (D3DCompile)
HRESULT
ShaderProgram::CompileShaderFromFile(char* szFileName,
    LPCSTR szEntryPoint,
    LPCSTR szShaderModel,
    ID3DBlob** ppBlobOut) {
    HRESULT hr = S_OK;

    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
    // Habilita el debug de shaders en Visual Studio Graphics Debugger
    dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

    ID3DBlob* pErrorBlob = nullptr;

    // Utiliza D3DX11CompileFromFile (Nota: En motores modernos se suele usar D3DCompileFromFile puro)
    hr = D3DX11CompileFromFile(szFileName,
        nullptr,
        nullptr,
        szEntryPoint,
        szShaderModel,
        dwShaderFlags,
        0,
        nullptr,
        ppBlobOut,
        &pErrorBlob,
        nullptr);

    if (FAILED(hr)) {
        if (pErrorBlob) {
            ERROR("ShaderProgram", "CompileShaderFromFile",
                "Failed to compile shader from file: %s. Error: %s",
                szFileName, static_cast<const char*>(pErrorBlob->GetBufferPointer()));

            SAFE_RELEASE(pErrorBlob);
        }
        else {
            ERROR("ShaderProgram", "CompileShaderFromFile",
                "Failed to compile shader from file: %s. No error message available.",
                szFileName);
        }
        return hr;
    }

    SAFE_RELEASE(pErrorBlob);
    return S_OK;
}

// Vincula el Pipeline completo (Layout + VS + PS) al contexto de renderizado
void ShaderProgram::render(DeviceContext& deviceContext) {
    if (!m_VertexShader || !m_PixelShader || !m_inputLayout.m_inputLayout) {
        ERROR("ShaderProgram", "render", "Shaders or InputLayout not initialized");
        return;
    }

    m_inputLayout.render(deviceContext);
    deviceContext.m_deviceContext->VSSetShader(m_VertexShader, nullptr, 0);
    deviceContext.m_deviceContext->PSSetShader(m_PixelShader, nullptr, 0);
}

// Vincula solo un shader específico (útil si hay pases donde el PS no cambia)
void ShaderProgram::render(DeviceContext& deviceContext, ShaderType type) {
    if (!deviceContext.m_deviceContext) {
        ERROR("ShaderProgram", "render", "DeviceContext is nullptr.");
        return;
    }

    switch (type) {
    case VERTEX_SHADER:
        deviceContext.m_deviceContext->VSSetShader(m_VertexShader, nullptr, 0);
        break;
    case PIXEL_SHADER:
        deviceContext.m_deviceContext->PSSetShader(m_PixelShader, nullptr, 0);
        break;
    default:
        break;
    }
}

// Libera los recursos compilados de la memoria de video
void ShaderProgram::destroy() {
    SAFE_RELEASE(m_VertexShader);
    SAFE_RELEASE(m_PixelShader);
    SAFE_RELEASE(m_vertexShaderData);
    SAFE_RELEASE(m_pixelShaderData);
    m_inputLayout.destroy();
}