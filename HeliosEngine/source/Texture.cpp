#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Texture.h"
#include "Device.h"
#include "DeviceContext.h"

// --- IMPLEMENTACIÓN DE LA REGLA DE LOS TRES (CRASH FIX) ---

// 1. Destructor
Texture::~Texture() {
    destroy();
}

// 2. Constructor de Copia (Cuando haces push_back en vector)
Texture::Texture(const Texture& other) {
    m_texture = other.m_texture;
    m_textureFromImg = other.m_textureFromImg;
    m_samplerState = other.m_samplerState;
    m_textureName = other.m_textureName;

    // Aumentamos referencias para que DirectX sepa que hay un dueño más
    if (m_texture) m_texture->AddRef();
    if (m_textureFromImg) m_textureFromImg->AddRef();
    if (m_samplerState) m_samplerState->AddRef();
}

// 3. Operador de Asignación (Cuando haces a = b)
Texture& Texture::operator=(const Texture& other) {
    if (this != &other) {
        destroy(); // Liberamos lo que teníamos antes

        m_texture = other.m_texture;
        m_textureFromImg = other.m_textureFromImg;
        m_samplerState = other.m_samplerState;
        m_textureName = other.m_textureName;

        // Aumentamos referencias
        if (m_texture) m_texture->AddRef();
        if (m_textureFromImg) m_textureFromImg->AddRef();
        if (m_samplerState) m_samplerState->AddRef();
    }
    return *this;
}
// -----------------------------------------------------------

void Texture::destroy() {
    // Usamos SAFE_RELEASE o comprobación directa
    if (m_texture) { m_texture->Release(); m_texture = nullptr; }
    if (m_textureFromImg) { m_textureFromImg->Release(); m_textureFromImg = nullptr; }
    if (m_samplerState) { m_samplerState->Release(); m_samplerState = nullptr; }
}

HRESULT Texture::init(Device& device, const std::string& textureName, ExtensionType extensionType) {
    if (!device.m_device) return E_POINTER;

    std::string ext;
    switch (extensionType) {
    case PNG: ext = ".png"; break;
    case JPG: ext = ".jpg"; break;
    case TGA: ext = ".tga"; break;
    default: ext = ".png"; break;
    }
    m_textureName = textureName + ext;

    int width, height, channels;
    // Forzamos 4 canales (RGBA)
    unsigned char* data = stbi_load(m_textureName.c_str(), &width, &height, &channels, 4);

    if (!data) {
        std::string altPath = "Assets/Textures/" + m_textureName;
        data = stbi_load(altPath.c_str(), &width, &height, &channels, 4);
        if (!data) return E_FAIL;
    }

    D3D11_TEXTURE2D_DESC textureDesc = {};
    textureDesc.Width = width;
    textureDesc.Height = height;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;
    textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = data;
    initData.SysMemPitch = width * 4;

    HRESULT hr = device.m_device->CreateTexture2D(&textureDesc, &initData, &m_texture);
    stbi_image_free(data);

    if (FAILED(hr)) return hr;

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = textureDesc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;

    return device.m_device->CreateShaderResourceView(m_texture, &srvDesc, &m_textureFromImg);
}

HRESULT Texture::init(Device& device, unsigned int width, unsigned int height, DXGI_FORMAT Format, unsigned int BindFlags, unsigned int sampleCount, unsigned int qualityLevels) {
    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = Format;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = BindFlags;

    return device.m_device->CreateTexture2D(&desc, nullptr, &m_texture);
}

HRESULT Texture::init(Device& device, Texture& textureRef, DXGI_FORMAT format) {
    // Implementación básica de copia de vista
    if (!textureRef.m_texture) return E_FAIL;

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;

    return device.m_device->CreateShaderResourceView(textureRef.m_texture, &srvDesc, &m_textureFromImg);
}

void Texture::update() {}

void Texture::render(DeviceContext& deviceContext, unsigned int StartSlot, unsigned int NumViews) {
    if (m_textureFromImg) {
        deviceContext.m_deviceContext->PSSetShaderResources(StartSlot, NumViews, &m_textureFromImg);
    }
}

HRESULT Texture::CreateCubemap(Device& device, DeviceContext& deviceContext, const std::array<std::string, 6>& facePaths, bool generateMips) {
    destroy();
    stbi_set_flip_vertically_on_load(false);

    int width = 0, height = 0, c = 0;
    std::array<unsigned char*, 6> facePixels{};

    for (int i = 0; i < 6; ++i) {
        facePixels[i] = stbi_load(facePaths[i].c_str(), &width, &height, &c, 4);
        if (!facePixels[i]) return E_FAIL;
    }

    D3D11_TEXTURE2D_DESC texDesc = {};
    texDesc.Width = width;
    texDesc.Height = height;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 6;
    texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    texDesc.SampleDesc.Count = 1;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    texDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

    std::array<D3D11_SUBRESOURCE_DATA, 6> data = {};
    for (int i = 0; i < 6; ++i) {
        data[i].pSysMem = facePixels[i];
        data[i].SysMemPitch = width * 4;
        data[i].SysMemSlicePitch = 0;
    }

    HRESULT hr = device.m_device->CreateTexture2D(&texDesc, data.data(), &m_texture);

    if (SUCCEEDED(hr)) {
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = texDesc.Format;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
        srvDesc.TextureCube.MostDetailedMip = 0;
        srvDesc.TextureCube.MipLevels = 1;
        hr = device.m_device->CreateShaderResourceView(m_texture, &srvDesc, &m_textureFromImg);
    }

    for (auto* p : facePixels) stbi_image_free(p);
    return hr;
}

ID3D11ShaderResourceView* Texture::CreateCubemapFaceSRV(ID3D11Device* device, ID3D11Texture2D* cubemapTex, DXGI_FORMAT format, UINT faceIndex, UINT mipLevels) {
    if (!cubemapTex) return nullptr;
    D3D11_SHADER_RESOURCE_VIEW_DESC d = {};
    d.Format = format;
    d.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
    d.Texture2DArray.MostDetailedMip = 0;
    d.Texture2DArray.MipLevels = 1;
    d.Texture2DArray.FirstArraySlice = faceIndex;
    d.Texture2DArray.ArraySize = 1;

    ID3D11ShaderResourceView* srv = nullptr;
    device->CreateShaderResourceView(cubemapTex, &d, &srv);
    return srv;
}