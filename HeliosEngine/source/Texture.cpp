#include "../include/Texture.h"
#include "../include/Device.h"
#include "../include/DeviceContext.h"
#include "DDSTextureLoader.h"   // ../include/

// helper: std::string (UTF-8) -> std::wstring
static std::wstring ToWide(const std::string& s) {
    if (s.empty()) return {};
    int n = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), nullptr, 0);
    std::wstring w; w.resize(n);
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), &w[0], n);
    return w;
}

HRESULT Texture::init(Device& device,
    const std::string& textureName,
    ExtensionType extensionType)
{
    if (!device.m_device) {
        ERROR("Texture", "init", "Device is null.");
        return E_POINTER;
    }
    if (textureName.empty()) {
        ERROR("Texture", "init", "Texture name cannot be empty.");
        return E_INVALIDARG;
    }

    switch (extensionType) {
    case ExtensionType::DDS:
    {
        // Ruta wide; añade ".dds" si no lo pusiste
        std::wstring wpath = ToWide(textureName);
        if (wpath.size() < 4 || _wcsicmp(wpath.c_str() + (wpath.size() - 4), L".dds") != 0)
            wpath += L".dds";

        // Carga DDS -> crea SRV (m_textureFromImg)
        ID3D11Resource* texResource = nullptr; // opcional
        HRESULT hr = DirectX::CreateDDSTextureFromFile(
            device.m_device,
            wpath.c_str(),
            &texResource,            // puedes pasar nullptr si no necesitas el recurso base
            &m_textureFromImg
        );
        if (texResource) texResource->Release();

        if (FAILED(hr)) {
            ERROR("Texture", "init", "Failed to load DDS texture");
            return hr;
        }
        m_textureName = std::string(textureName); // metadato
        return S_OK;
    }

    case ExtensionType::PNG:
    case ExtensionType::JPG:
        // Si más adelante usas WIC o CreateWICTextureFromFile, implementa aquí.
        ERROR("Texture", "init", "WIC path not implemented in this project.");
        return E_NOTIMPL;

    default:
        ERROR("Texture", "init", "Unsupported extension type");
        return E_INVALIDARG;
    }
}

HRESULT Texture::init(Device& device,
    unsigned int width,
    unsigned int height,
    DXGI_FORMAT Format,
    unsigned int BindFlags,
    unsigned int sampleCount,
    unsigned int qualityLevels)
{
    if (!device.m_device) {
        ERROR("Texture", "init", "Device is null.");
        return E_POINTER;
    }
    if (width == 0 || height == 0) {
        ERROR("Texture", "init", "Width and height must be greater than 0");
        return E_INVALIDARG;
    }

    D3D11_TEXTURE2D_DESC desc{};
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

    // Usa directamente el device de D3D11
    HRESULT hr = device.m_device->CreateTexture2D(&desc, nullptr, &m_texture);
    if (FAILED(hr)) {
        ERROR("Texture", "init",
            ("Failed to create texture. HRESULT: " + std::to_string(hr)).c_str());
        return hr;
    }
    return S_OK;
}

HRESULT Texture::init(Device& device, Texture& textureRef, DXGI_FORMAT format)
{
    if (!device.m_device) {
        ERROR("Texture", "init", "Device is null.");
        return E_POINTER;
    }
    if (!textureRef.m_texture) {
        ERROR("Texture", "init", "TextureRef has no ID3D11Texture2D.");
        return E_POINTER;
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.Format = format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;
    srvDesc.Texture2D.MostDetailedMip = 0;

    HRESULT hr = device.m_device->CreateShaderResourceView(
        textureRef.m_texture, &srvDesc, &m_textureFromImg);
    if (FAILED(hr)) {
        ERROR("Texture", "init",
            ("Failed to create SRV. HRESULT: " + std::to_string(hr)).c_str());
        return hr;
    }
    return S_OK;
}

void Texture::update() {}

void Texture::render(DeviceContext& deviceContext, unsigned int StartSlot, unsigned int NumViews)
{
    // No accedemos a m_deviceContext (prob. es privado). Usa el wrapper.
    if (m_textureFromImg) {
        deviceContext.PSSetShaderResources(StartSlot, NumViews, &m_textureFromImg);
    }
}

void Texture::destroy()
{
    if (m_texture) { SAFE_RELEASE(m_texture); }
    if (m_textureFromImg) { SAFE_RELEASE(m_textureFromImg); }
}
