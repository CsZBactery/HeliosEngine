#include "../include/Device.h"

// -----------------------------------------------------------------------------
// INICIALIZACION DEL HARDWARE VIRTUAL
// -----------------------------------------------------------------------------
void Device::init() {
    UINT createDeviceFlags = 0;

    // Si estamos en modo Debug (Visual Studio), activamos la capa de depuracion de DirectX.
    // Esto es vital: nos avisa en la consola si cometemos errores en los shaders o leaks de memoria.
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    // Lista de drivers en orden de preferencia:
    // 1. HARDWARE: Usar la tarjeta grafica real (NVIDIA/AMD). Es lo que queremos.
    // 2. WARP: Renderizado por CPU optimizado (rpido, pero no tanto como GPU). Usado si no hay GPU dedicada.
    // 3. REFERENCE: Implementacion de referencia (muy lento, solo para pruebas de exactitud).
    D3D_DRIVER_TYPE driverTypes[] = {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
    };
    UINT numDriverTypes = ARRAYSIZE(driverTypes);

    // Niveles de DirectX que soportamos (del mas nuevo al mas viejo)
    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };
    UINT numFeatureLevels = ARRAYSIZE(featureLevels);

    D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
    HRESULT hr = E_FAIL;

    // Bucle principal de creacion: Probamos los drivers uno por uno hasta que uno funcione.
    for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++) {
        D3D_DRIVER_TYPE driverType = driverTypes[driverTypeIndex];

        // Variable temporal para el contexto. 
        // Nota: Aunque D3D11CreateDevice nos crea un contexto, lo soltamos aqui mismo
        // porque la clase DeviceContext se encargara de recuperarlo y gestionarlo despues.
        ID3D11DeviceContext* pContext = nullptr;

        hr = D3D11CreateDevice(
            nullptr,                    // Adapter (nullptr usa el monitor principal)
            driverType,                 // Tipo de driver (Hardware, Warp, etc.)
            nullptr,                    // Software rasterizer (no usado)
            createDeviceFlags,          // Flags (Debug, etc.)
            featureLevels,              // Versiones soportadas
            numFeatureLevels,           // Cantidad de versiones
            D3D11_SDK_VERSION,          // Version del SDK
            &m_device,                  // [OUT] El puntero al Device creado
            &featureLevel,              // [OUT] La version de DX que se logro activar
            &pContext                   // [OUT] Contexto temporal
        );

        if (SUCCEEDED(hr)) {
            // Exito: soltamos el contexto temporal (BaseApp lo pedira limpiamente luego)
            if (pContext) pContext->Release();
            break;
        }
    }

    if (FAILED(hr)) {
        ERROR("Device", "init", "Fallo critico: No se pudo crear el dispositivo D3D11.");
    }
}

// Stubs para mantener la interfaz de componentes, aunque el Device es estatico
void Device::update() {}
void Device::render() {}

void Device::destroy() {
    // Liberar la interfaz fisica de la GPU
    if (m_device) m_device->Release();
    m_device = nullptr;
}

// -----------------------------------------------------------------------------
// WRAPPERS DE CREACION DE RECURSOS
// Estas funciones encapsulan las llamadas nativas de DX11 y agregan chequeos de seguridad.
// -----------------------------------------------------------------------------

HRESULT Device::CreateRenderTargetView(ID3D11Resource* pResource, const D3D11_RENDER_TARGET_VIEW_DESC* pDesc, ID3D11RenderTargetView** ppRTView) {
    if (!m_device) return E_FAIL;
    if (!pResource) return E_INVALIDARG;
    if (!ppRTView) return E_POINTER;

    // Crear la vista para que el output merger sepa donde dibujar
    HRESULT hr = m_device->CreateRenderTargetView(pResource, pDesc, ppRTView);
    if (FAILED(hr)) {
        ERROR("Device", "CreateRenderTargetView", ("Fallo al crear RTV. HR: " + std::to_string(hr)).c_str());
    }
    return hr;
}

HRESULT Device::CreateTexture2D(const D3D11_TEXTURE2D_DESC* pDesc, const D3D11_SUBRESOURCE_DATA* pInitialData, ID3D11Texture2D** ppTexture2D) {
    if (!m_device) return E_FAIL;
    if (!pDesc) return E_INVALIDARG;

    // Reservar memoria en VRAM para la textura
    HRESULT hr = m_device->CreateTexture2D(pDesc, pInitialData, ppTexture2D);
    if (FAILED(hr)) {
        ERROR("Device", "CreateTexture2D", ("Fallo al crear Textura2D. HR: " + std::to_string(hr)).c_str());
    }
    return hr;
}

HRESULT Device::CreateDepthStencilView(ID3D11Resource* pResource, const D3D11_DEPTH_STENCIL_VIEW_DESC* pDesc, ID3D11DepthStencilView** ppDepthStencilView) {
    // Wrapper para la vista del Z-Buffer
    if (!m_device) return E_FAIL;
    if (!pResource) return E_INVALIDARG;

    HRESULT hr = m_device->CreateDepthStencilView(pResource, pDesc, ppDepthStencilView);
    if (FAILED(hr)) {
        ERROR("Device", "CreateDepthStencilView", ("Fallo al crear DSV. HR: " + std::to_string(hr)).c_str());
    }
    return hr;
}

HRESULT Device::CreateVertexShader(const void* pShaderBytecode, unsigned int BytecodeLength, ID3D11ClassLinkage* pClassLinkage, ID3D11VertexShader** ppVertexShader) {
    // Compilar el Vertex Shader en la GPU
    if (!m_device) return E_FAIL;
    if (!pShaderBytecode) return E_INVALIDARG;

    HRESULT hr = m_device->CreateVertexShader(pShaderBytecode, BytecodeLength, pClassLinkage, ppVertexShader);
    if (FAILED(hr)) {
        ERROR("Device", "CreateVertexShader", ("Fallo al crear VS. HR: " + std::to_string(hr)).c_str());
    }
    return hr;
}

HRESULT Device::CreateInputLayout(const D3D11_INPUT_ELEMENT_DESC* pInputElementDescs, UINT NumElements, const void* pShaderBytecodeWithInputSignature, unsigned int BytecodeLength, ID3D11InputLayout** ppInputLayout) {
    // Validar que el layout de C++ coincida con el input del Shader
    if (!m_device) return E_FAIL;

    HRESULT hr = m_device->CreateInputLayout(pInputElementDescs, NumElements, pShaderBytecodeWithInputSignature, BytecodeLength, ppInputLayout);
    if (FAILED(hr)) {
        ERROR("Device", "CreateInputLayout", ("Fallo al crear InputLayout. HR: " + std::to_string(hr)).c_str());
    }
    return hr;
}

HRESULT Device::CreatePixelShader(const void* pShaderBytecode, unsigned int BytecodeLength, ID3D11ClassLinkage* pClassLinkage, ID3D11PixelShader** ppPixelShader) {
    if (!m_device) return E_FAIL;

    HRESULT hr = m_device->CreatePixelShader(pShaderBytecode, BytecodeLength, pClassLinkage, ppPixelShader);
    if (FAILED(hr)) {
        ERROR("Device", "CreatePixelShader", ("Fallo al crear PS. HR: " + std::to_string(hr)).c_str());
    }
    return hr;
}

HRESULT Device::CreateSamplerState(const D3D11_SAMPLER_DESC* pSamplerDesc, ID3D11SamplerState** ppSamplerState) {
    // Crear estado de muestreo (filtros, clamping, wrapping)
    if (!m_device) return E_FAIL;

    HRESULT hr = m_device->CreateSamplerState(pSamplerDesc, ppSamplerState);
    if (FAILED(hr)) {
        ERROR("Device", "CreateSamplerState", ("Fallo al crear Sampler. HR: " + std::to_string(hr)).c_str());
    }
    return hr;
}

HRESULT Device::CreateBuffer(const D3D11_BUFFER_DESC* pDesc, const D3D11_SUBRESOURCE_DATA* pInitialData, ID3D11Buffer** ppBuffer) {
    // Wrapper generico para Vertex, Index y Constant Buffers
    if (!m_device) return E_FAIL;
    if (!pDesc) return E_INVALIDARG;

    HRESULT hr = m_device->CreateBuffer(pDesc, pInitialData, ppBuffer);
    if (FAILED(hr)) {
        ERROR("Device", "CreateBuffer", ("Fallo al crear Buffer. HR: " + std::to_string(hr)).c_str());
    }
    return hr;
}