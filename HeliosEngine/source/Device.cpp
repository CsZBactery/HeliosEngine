#include "../include/Device.h"

// -----------------------------------------------------------------------------
// ESTA ES LA FUNCIÓN QUE TE FALTABA (init)
// Es obligatoria para crear la tarjeta gráfica virtual.
// -----------------------------------------------------------------------------
void Device::init() {
    UINT createDeviceFlags = 0;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_DRIVER_TYPE driverTypes[] = {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
    };
    UINT numDriverTypes = ARRAYSIZE(driverTypes);

    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };
    UINT numFeatureLevels = ARRAYSIZE(featureLevels);

    D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
    HRESULT hr = E_FAIL;

    for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++) {
        D3D_DRIVER_TYPE driverType = driverTypes[driverTypeIndex];

        // Usamos un contexto temporal
        ID3D11DeviceContext* pContext = nullptr;

        hr = D3D11CreateDevice(
            nullptr,                    // Adapter
            driverType,                 // Driver Type
            nullptr,                    // Software
            createDeviceFlags,          // Flags
            featureLevels,              // Feature Levels
            numFeatureLevels,           // Num Feature Levels
            D3D11_SDK_VERSION,          // SDK Version
            &m_device,                  // Device (Output)
            &featureLevel,              // Feature Level (Output)
            &pContext                   // Context (Output temporal)
        );

        if (SUCCEEDED(hr)) {
            // Liberamos el contexto temporal (BaseApp lo recuperará después)
            if (pContext) pContext->Release();
            break;
        }
    }

    if (FAILED(hr)) {
        ERROR("Device", "init", "Failed to create D3D11 Device.");
    }
}

// Funciones vacías requeridas por el Linker para evitar errores
void Device::update() {}
void Device::render() {}

// -----------------------------------------------------------------------------
// TUS FUNCIONES ORIGINALES (Mantén todo esto igual)
// -----------------------------------------------------------------------------

void Device::destroy() {
    if (m_device) m_device->Release();
    m_device = nullptr;
}

HRESULT Device::CreateRenderTargetView(ID3D11Resource* pResource, const D3D11_RENDER_TARGET_VIEW_DESC* pDesc, ID3D11RenderTargetView** ppRTView) {
    if (!m_device) return E_FAIL; // Protección extra
    if (!pResource) return E_INVALIDARG;
    if (!ppRTView) return E_POINTER;

    HRESULT hr = m_device->CreateRenderTargetView(pResource, pDesc, ppRTView);
    if (FAILED(hr)) {
        ERROR("Device", "CreateRenderTargetView", ("Failed. HRESULT: " + std::to_string(hr)).c_str());
    }
    return hr;
}

HRESULT Device::CreateTexture2D(const D3D11_TEXTURE2D_DESC* pDesc, const D3D11_SUBRESOURCE_DATA* pInitialData, ID3D11Texture2D** ppTexture2D) {
    if (!m_device) return E_FAIL;
    if (!pDesc) return E_INVALIDARG;
    if (!ppTexture2D) return E_POINTER;

    HRESULT hr = m_device->CreateTexture2D(pDesc, pInitialData, ppTexture2D);
    if (FAILED(hr)) {
        ERROR("Device", "CreateTexture2D", ("Failed. HRESULT: " + std::to_string(hr)).c_str());
    }
    return hr;
}

HRESULT Device::CreateDepthStencilView(ID3D11Resource* pResource, const D3D11_DEPTH_STENCIL_VIEW_DESC* pDesc, ID3D11DepthStencilView** ppDepthStencilView) {
    if (!m_device) return E_FAIL;
    if (!pResource) return E_INVALIDARG;
    if (!ppDepthStencilView) return E_POINTER;

    HRESULT hr = m_device->CreateDepthStencilView(pResource, pDesc, ppDepthStencilView);
    if (FAILED(hr)) {
        ERROR("Device", "CreateDepthStencilView", ("Failed. HRESULT: " + std::to_string(hr)).c_str());
    }
    return hr;
}

HRESULT Device::CreateVertexShader(const void* pShaderBytecode, unsigned int BytecodeLength, ID3D11ClassLinkage* pClassLinkage, ID3D11VertexShader** ppVertexShader) {
    if (!m_device) return E_FAIL;
    if (!pShaderBytecode) return E_INVALIDARG;
    if (!ppVertexShader) return E_POINTER;

    HRESULT hr = m_device->CreateVertexShader(pShaderBytecode, BytecodeLength, pClassLinkage, ppVertexShader);
    if (FAILED(hr)) {
        ERROR("Device", "CreateVertexShader", ("Failed. HRESULT: " + std::to_string(hr)).c_str());
    }
    return hr;
}

HRESULT Device::CreateInputLayout(const D3D11_INPUT_ELEMENT_DESC* pInputElementDescs, UINT NumElements, const void* pShaderBytecodeWithInputSignature, unsigned int BytecodeLength, ID3D11InputLayout** ppInputLayout) {
    if (!m_device) return E_FAIL;
    if (!pInputElementDescs) return E_INVALIDARG;
    if (!ppInputLayout) return E_POINTER;

    HRESULT hr = m_device->CreateInputLayout(pInputElementDescs, NumElements, pShaderBytecodeWithInputSignature, BytecodeLength, ppInputLayout);
    if (FAILED(hr)) {
        ERROR("Device", "CreateInputLayout", ("Failed. HRESULT: " + std::to_string(hr)).c_str());
    }
    return hr;
}

HRESULT Device::CreatePixelShader(const void* pShaderBytecode, unsigned int BytecodeLength, ID3D11ClassLinkage* pClassLinkage, ID3D11PixelShader** ppPixelShader) {
    if (!m_device) return E_FAIL;
    if (!pShaderBytecode) return E_INVALIDARG;
    if (!ppPixelShader) return E_POINTER;

    HRESULT hr = m_device->CreatePixelShader(pShaderBytecode, BytecodeLength, pClassLinkage, ppPixelShader);
    if (FAILED(hr)) {
        ERROR("Device", "CreatePixelShader", ("Failed. HRESULT: " + std::to_string(hr)).c_str());
    }
    return hr;
}

HRESULT Device::CreateSamplerState(const D3D11_SAMPLER_DESC* pSamplerDesc, ID3D11SamplerState** ppSamplerState) {
    if (!m_device) return E_FAIL;
    if (!pSamplerDesc) return E_INVALIDARG;
    if (!ppSamplerState) return E_POINTER;

    HRESULT hr = m_device->CreateSamplerState(pSamplerDesc, ppSamplerState);
    if (FAILED(hr)) {
        ERROR("Device", "CreateSamplerState", ("Failed. HRESULT: " + std::to_string(hr)).c_str());
    }
    return hr;
}

HRESULT Device::CreateBuffer(const D3D11_BUFFER_DESC* pDesc, const D3D11_SUBRESOURCE_DATA* pInitialData, ID3D11Buffer** ppBuffer) {
    if (!m_device) return E_FAIL;
    if (!pDesc) return E_INVALIDARG;
    if (!ppBuffer) return E_POINTER;

    HRESULT hr = m_device->CreateBuffer(pDesc, pInitialData, ppBuffer);
    if (FAILED(hr)) {
        ERROR("Device", "CreateBuffer", ("Failed. HRESULT: " + std::to_string(hr)).c_str());
    }
    return hr;
}