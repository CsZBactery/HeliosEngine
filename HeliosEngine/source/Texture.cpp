#include "../include/Texture.h"
#include "../include/Device.h"
#include "../include/DeviceContext.h"

#include <Windows.h>
#include <d3dx11.h>
#include <string>

// --- Helper local: string (UTF-8/ANSI) -> wstring (UTF-16) ---
static std::wstring ToW(const std::string& s) {
    if (s.empty()) return std::wstring();
    int len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
    std::wstring ws(len ? len - 1 : 0, L'\0');
    if (len > 1) MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, &ws[0], len);
    return ws;
}

// ------------------------------------------------------------------
// Carga desde archivo (DDS/PNG/JPG)
// ------------------------------------------------------------------
HRESULT Texture::init(Device& device,
    const std::string& textureName,
    ExtensionType extensionType) {
    if (!device.m_device) {
        ERROR("Texture", "init", "Device is null.");
        return E_POINTER;
    }
    if (textureName.empty()) {
        ERROR("Texture", "init", "Texture name cannot be empty.");
        return E_INVALIDARG;
    }

    // Armar nombre de archivo según extensión pedida
    switch (extensionType) {
    case ExtensionType::DDS: m_textureName = textureName + ".dds"; break;
    case ExtensionType::PNG: m_textureName = textureName + ".png"; break;
    case ExtensionType::JPG: m_textureName = textureName + ".jpg"; break;
    default:
        ERROR("Texture", "init", "Unsupported extension type");
        return E_INVALIDARG;
    }

    // Cargar (Unicode)
    std::wstring wpath = ToW(m_textureName);
    HRESULT hr = D3DX11CreateShaderResourceViewFromFileW(
        device.m_device,
        wpath.c_str(),
        nullptr,
        nullptr,
        &m_textureFromImg,
        nullptr
    );

    if (FAILED(hr)) {
        std::string msg = "Failed to load texture. Verify filepath: " + m_textureName;
        ERROR("Texture", "init", msg.c_str());
        return hr;
    }

    return S_OK;
}

// ------------------------------------------------------------------
// Crear textura vacía (RT/Depth/etc.)
// ------------------------------------------------------------------
HRESULT Texture::init(Device& device,
    unsigned int width,
    unsigned int height,
    DXGI_FORMAT Format,
    unsigned int BindFlags,
    unsigned int sampleCount,
    unsigned int qualityLevels) {
    if (!device.m_device) {
        ERROR("Texture", "init", "Device is null");
        return E_POINTER;
    }
    if (width == 0 || height == 0) {
        ERROR("Texture", "init", "Width and height must be greater than 0");
        return E_INVALIDARG;
    }

    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = Format;
    desc.SampleDesc.Count = sampleCount;
    desc.SampleDesc.Quality = qualityLevels;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = BindFlags;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;

    HRESULT hr = device.CreateTexture2D(&desc, nullptr, &m_texture);
    if (FAILED(hr)) {
        std::string msg = "Failed to create texture. HRESULT: " + std::to_string(hr);
        ERROR("Texture", "init", msg.c_str());
        return hr;
    }
    return S_OK;
}

// ------------------------------------------------------------------
// Crear SRV desde textura existente
// ------------------------------------------------------------------
HRESULT Texture::init(Device& device, Texture& textureRef, DXGI_FORMAT format) {
    if (!device.m_device) {
        ERROR("Texture", "init", "Device is null.");
        return E_POINTER;
    }
    if (!textureRef.m_texture) {
        ERROR("Texture", "init", "Texture is null");
        return E_POINTER;
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;
    srvDesc.Texture2D.MostDetailedMip = 0;

    HRESULT hr = device.m_device->CreateShaderResourceView(
        textureRef.m_texture, &srvDesc, &m_textureFromImg);

    if (FAILED(hr)) {
        std::string msg = "Failed to create SRV. HRESULT: " + std::to_string(hr);
        ERROR("Texture", "init", msg.c_str());
        return hr;
    }
    return S_OK;
}

void Texture::update() {}

void Texture::render(DeviceContext& deviceContext,
    unsigned int StartSlot,
    unsigned int NumViews) {
    if (!deviceContext.m_deviceContext) {
        ERROR("Texture", "render", "Device Context is null.");
        return;
    }
    if (m_textureFromImg) {
        deviceContext.PSSetShaderResources(StartSlot, NumViews, &m_textureFromImg);
    }
}

void Texture::destroy() {
    if (m_texture) { SAFE_RELEASE(m_texture); }
    if (m_textureFromImg) { SAFE_RELEASE(m_textureFromImg); }
}
