// ======================================================================================
// Archivo: SwapChain.cpp
// Gestiona la cadena de intercambio DXGI y el Doble Buffering para presentar en pantalla.
// ======================================================================================

#include "SwapChain.h"
#include "Device.h"
#include "DeviceContext.h"
#include "Texture.h"
#include "Window.h"

// Inicializa el Swap Chain permitiendo que la imagen en el back buffer pase a la pantalla.
HRESULT
SwapChain::init(Device& device, DeviceContext& deviceContext, Texture& backBuffer, Window window) {
    // Se verifica que el handle de la ventana sea válido antes de continuar.
    if (!window.m_hWnd) {
        ERROR("SwapChain", "init", "Invalid window handle. (m_hWnd is nullptr)");
        return E_POINTER;
    }

    HRESULT hr = S_OK;

    // Se configuran los flags para la creación del dispositivo. En modo de depuración,
    // se agrega el flag para permitir mensajes de error detallados de Direct3D.
    unsigned int createDeviceFlags = 0;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    // Se definen los tipos de controladores que se intentarán: Hardware real, luego emulado.
    D3D_DRIVER_TYPE driverTypes[] = {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
    };
    unsigned int numDriverTypes = ARRAYSIZE(driverTypes);

    // Se intenta con DirectX 11.0, y se retrocede a versiones de DX10 si la GPU es muy antigua.
    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };
    unsigned int numFeatureLevels = ARRAYSIZE(featureLevels);

    // Se intenta crear el dispositivo y el contexto de Direct3D.
    for (unsigned int driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++) {
        D3D_DRIVER_TYPE driverType = driverTypes[driverTypeIndex];
        hr = D3D11CreateDevice(
            nullptr,
            driverType,
            nullptr,
            createDeviceFlags,
            featureLevels,
            numFeatureLevels,
            D3D11_SDK_VERSION,
            &device.m_device,
            &m_featureLevel,
            &deviceContext.m_deviceContext
        );

        if (SUCCEEDED(hr)) {
            MESSAGE("SwapChain", "init", "Device created successfully.");
            break;
        }
    }

    if (FAILED(hr)) {
        ERROR("SwapChain", "init", ("Failed to create D3D11 device. HRESULT: " + std::to_string(hr)).c_str());
        return hr;
    }

    // Configuración de MSAA (Suavizado de bordes). Validamos qué tanta calidad soporta la tarjeta.
    m_sampleCount = 4;
    hr = device.m_device->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, m_sampleCount, &m_qualityLevels);

    if (FAILED(hr) || m_qualityLevels == 0) {
        ERROR("SwapChain", "init", ("MSAA not supported or invalid quality level. HRESULT: " + std::to_string(hr)).c_str());
        return hr;
    }

    // Configuración de la estructura de la Cadena de Intercambio
    DXGI_SWAP_CHAIN_DESC sd;
    memset(&sd, 0, sizeof(sd));
    sd.BufferCount = 1; // 1 Backbuffer (+1 Frontbuffer implícito) = Double Buffering
    sd.BufferDesc.Width = window.m_width;
    sd.BufferDesc.Height = window.m_height;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = window.m_hWnd;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD; // Destruye el frame viejo tras mostrarlo para mayor velocidad

    // Asignación de MSAA al SwapChain principal
    sd.SampleDesc.Count = m_sampleCount;
    sd.SampleDesc.Quality = m_qualityLevels - 1;

    // Obtención de las interfaces de fábrica de la placa de video (Jerarquía DXGI)
    hr = device.m_device->QueryInterface(__uuidof(IDXGIDevice), (void**)&m_dxgiDevice);
    if (FAILED(hr)) return hr;

    hr = m_dxgiDevice->GetAdapter(&m_dxgiAdapter);
    if (FAILED(hr)) return hr;

    hr = m_dxgiAdapter->GetParent(__uuidof(IDXGIFactory), reinterpret_cast<void**>(&m_dxgiFactory));
    if (FAILED(hr)) return hr;

    // Creación oficial del Swap Chain usando la configuración previa
    hr = m_dxgiFactory->CreateSwapChain(device.m_device, &sd, &m_swapChain);
    if (FAILED(hr)) {
        ERROR("SwapChain", "init", ("Failed to create swap chain. HRESULT: " + std::to_string(hr)).c_str());
        return hr;
    }

    // Extracción de la textura 2D física (Lienzo) que la GPU acaba de crear
    hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer.m_texture));
    if (FAILED(hr)) {
        ERROR("SwapChain", "init", ("Failed to get back buffer. HRESULT: " + std::to_string(hr)).c_str());
        return hr;
    }

    return S_OK;
}

// Libera los recursos de Direct3D relacionados en el orden correcto
void SwapChain::destroy() {
    SAFE_RELEASE(m_swapChain);
    SAFE_RELEASE(m_dxgiDevice);
    SAFE_RELEASE(m_dxgiAdapter);
    SAFE_RELEASE(m_dxgiFactory);
}

// Intercambia el back buffer con la pantalla visible.
void SwapChain::present() {
    if (m_swapChain) {
        // Enviar imagen a pantalla (Flip)
        HRESULT hr = m_swapChain->Present(0, 0);
        if (FAILED(hr)) {
            ERROR("SwapChain", "present", ("Failed to present swap chain. HRESULT: " + std::to_string(hr)).c_str());
        }
    }
    else {
        ERROR("SwapChain", "present", "Swap chain is not initialized.");
    }
}

// ======================================================================================
// NUEVOS MÉTODOS DEL PROFESOR: Vitales para soportar redimensionado de ventana.
// ======================================================================================

// Cambia dinámicamente la resolución interna de los buffers para coincidir con la UI.
HRESULT
SwapChain::resizeBuffers(UINT width, UINT height) {
    if (!m_swapChain) {
        ERROR("SwapChain", "resizeBuffers", "Swap chain is not initialized.");
        return E_POINTER;
    }

    // Pasar 0 en formato y cantidad de buffers indica a DXGI que mantenga la config actual,
    // pero actualizando a los nuevos 'width' y 'height'.
    HRESULT hr = m_swapChain->ResizeBuffers(
        0,
        width,
        height,
        DXGI_FORMAT_UNKNOWN,
        0
    );

    if (FAILED(hr)) {
        ERROR("SwapChain", "resizeBuffers", ("ResizeBuffers failed. HRESULT: " + std::to_string(hr)).c_str());
        return hr;
    }

    return S_OK;
}

// Devuelve el BackBuffer actual tras un redimensionado.
HRESULT
SwapChain::getBackBuffer(Texture& backBuffer) {
    if (!m_swapChain) {
        ERROR("SwapChain", "getBackBuffer", "Swap chain is not initialized.");
        return E_POINTER;
    }

    // IMPORTANTE: Se inyecta la memoria directamente en la variable miembro m_texture
    HRESULT hr = m_swapChain->GetBuffer(
        0, __uuidof(ID3D11Texture2D),
        reinterpret_cast<void**>(&backBuffer.m_texture)
    );

    if (FAILED(hr)) {
        ERROR("SwapChain", "getBackBuffer", ("Failed to get back buffer. HRESULT: " + std::to_string(hr)).c_str());
        return hr;
    }

    return S_OK;
}