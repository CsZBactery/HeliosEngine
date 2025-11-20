// Texture.cpp — versión unificada (DDS con D3DX11 + PNG/JPG con stb_image)
#include "../include/Texture.h"
#include "../include/Device.h"
#include "../include/DeviceContext.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#include <d3d11.h>
#include <d3dx11.h> 
#include <string>

#define STB_IMAGE_IMPLEMENTATION
#include "../include/stb_image.h"


static std::wstring ToW(const std::string& s) {
    if (s.empty()) return std::wstring();
    int len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
    std::wstring ws(len ? len - 1 : 0, L'\0');
    if (len > 1) MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, &ws[0], len);
    return ws;
}

HRESULT Texture::init(Device& device,
    const std::string& textureName,
    ExtensionType extensionType)
{
    if (!device.m_device) {
        ERROR(L"Texture", L"init", L"Device is null.");
        return E_POINTER;
    }
    if (textureName.empty()) {
        ERROR(L"Texture", L"init", L"Texture name cannot be empty.");
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;

    switch (extensionType)
    {
    case ExtensionType::DDS:
    {
    
        m_textureName = textureName + ".dds";
        std::wstring wpath = ToW(m_textureName);

        hr = D3DX11CreateShaderResourceViewFromFileW(
            device.m_device,
            wpath.c_str(),
            nullptr,
            nullptr,
            &m_textureFromImg,
            nullptr
        );

        if (FAILED(hr)) {
            std::wstring wmsg = L"Failed to load DDS texture. Verify filepath: " + ToW(m_textureName);
            ERROR(L"Texture", L"init", wmsg.c_str());
            return hr;
        }
        break;
    }

    case ExtensionType::PNG:
    case ExtensionType::JPG:
    {
        
        m_textureName = textureName + (extensionType == ExtensionType::PNG ? ".png" : ".jpg");

        int width = 0, height = 0, channels = 0;
        unsigned char* data = stbi_load(m_textureName.c_str(), &width, &height, &channels, 4);
        if (!data) {
            std::string r = stbi_failure_reason() ? stbi_failure_reason() : "unknown";
            std::wstring wmsg = L"Failed to load image with stb_image: " + ToW(r) +
                L" Path: " + ToW(m_textureName);
            ERROR(L"Texture", L"init", wmsg.c_str());
            return E_FAIL;
        }

        
        D3D11_TEXTURE2D_DESC desc = {};
        desc.Width = static_cast<UINT>(width);
        desc.Height = static_cast<UINT>(height);
        desc.MipLevels = 1;     
        desc.ArraySize = 1;
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        desc.CPUAccessFlags = 0;
        desc.MiscFlags = 0;

        D3D11_SUBRESOURCE_DATA initData = {};
        initData.pSysMem = data;
        initData.SysMemPitch = static_cast<UINT>(width * 4);

        ID3D11Texture2D* tex = nullptr;
        hr = device.CreateTexture2D(&desc, &initData, &tex);

        stbi_image_free(data);

        if (FAILED(hr) || !tex) {
            ERROR(L"Texture", L"init", L"Failed to create D3D texture from image data.");
            SAFE_RELEASE(tex);
            return FAILED(hr) ? hr : E_FAIL;
        }

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = desc.Format;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.MipLevels = 1;

        hr = device.m_device->CreateShaderResourceView(tex, &srvDesc, &m_textureFromImg);
        SAFE_RELEASE(tex);

        if (FAILED(hr)) {
            ERROR(L"Texture", L"init", L"Failed to create SRV for image texture.");
            return hr;
        }
        break;
    }

    default:
        ERROR(L"Texture", L"init", L"Unsupported extension type.");
        return E_INVALIDARG;
    }

    return S_OK;
}

// ------------------------------------------------------------------
// Crear textura vacía
// ------------------------------------------------------------------
HRESULT Texture::init(Device& device,
    unsigned int width,
    unsigned int height,
    DXGI_FORMAT Format,
    unsigned int BindFlags,
    unsigned int sampleCount,
    unsigned int qualityLevels)
{
    if (!device.m_device) {
        ERROR(L"Texture", L"init", L"Device is null");
        return E_POINTER;
    }
    if (width == 0 || height == 0) {
        ERROR(L"Texture", L"init", L"Width and height must be greater than 0");
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
        std::wstring wmsg = L"Failed to create texture. HRESULT: " + ToW(std::to_string(hr));
        ERROR(L"Texture", L"init", wmsg.c_str());
        return hr;
    }
    return S_OK;
}

// ------------------------------------------------------------------
// Crear SRV desde textura existente
// ------------------------------------------------------------------
HRESULT Texture::init(Device& device, Texture& textureRef, DXGI_FORMAT format)
{
    if (!device.m_device) {
        ERROR(L"Texture", L"init", L"Device is null.");
        return E_POINTER;
    }
    if (!textureRef.m_texture) {
        ERROR(L"Texture", L"init", L"Texture is null");
        return E_POINTER;
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = 1;

    HRESULT hr = device.m_device->CreateShaderResourceView(
        textureRef.m_texture, &srvDesc, &m_textureFromImg);

    if (FAILED(hr)) {
        std::wstring wmsg = L"Failed to create SRV. HRESULT: " + ToW(std::to_string(hr));
        ERROR(L"Texture", L"init", wmsg.c_str());
        return hr;
    }
    return S_OK;
}

void Texture::update() {}

// Enlaza la SRV al Pixel Shader
void Texture::render(DeviceContext& deviceContext,
    unsigned int StartSlot,
    unsigned int NumViews)
{
    if (!deviceContext.m_deviceContext) {
        ERROR(L"Texture", L"render", L"Device Context is null.");
        return;
    }
    if (m_textureFromImg) {
        deviceContext.PSSetShaderResources(StartSlot, NumViews, &m_textureFromImg);
    }
}

void Texture::destroy()
{
    if (m_texture) { SAFE_RELEASE(m_texture); }
    if (m_textureFromImg) { SAFE_RELEASE(m_textureFromImg); }
}
