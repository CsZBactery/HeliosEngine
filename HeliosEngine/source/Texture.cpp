#include "Texture.h"
// Define STB_IMAGE_IMPLEMENTATION aquí si no está definido en otro lado
#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#endif
#include "stb_image.h"

// ----------------------------------------------------------------------------------
// CORRECCIÓN 1: EL DESTRUCTOR DEBE ESTAR VACÍO O COMENTADO
// Si llamas a destroy() aquí, los std::vector borrarán la textura antes de tiempo.
// ----------------------------------------------------------------------------------
Texture::~Texture() {
    // destroy(); <--- ¡ESTO ESTABA CAUSANDO EL CRASH! LO DEJAMOS COMENTADO.
}

// ----------------------------------------------------------------------------------
// 1. INIT DESDE ARCHIVO
// ----------------------------------------------------------------------------------
HRESULT Texture::init(Device& device, const std::string& textureName, ExtensionType extensionType) {
    if (!device.m_device) return E_POINTER;

    HRESULT hr = S_OK;
    std::string finalName = textureName;

    if (extensionType == PNG && textureName.find(".png") == std::string::npos) finalName += ".png";
    else if (extensionType == JPG && textureName.find(".jpg") == std::string::npos) finalName += ".jpg";

    int width, height, channels;
    unsigned char* data = stbi_load(finalName.c_str(), &width, &height, &channels, 4);

    if (!data) {
        data = stbi_load(textureName.c_str(), &width, &height, &channels, 4);
    }

    if (!data) {
        ERROR("Texture", "init", ("Failed to load image: " + finalName).c_str());
        return E_FAIL;
    }

    m_textureName = finalName;

    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA subData = {};
    subData.pSysMem = data;
    subData.SysMemPitch = width * 4;

    hr = device.m_device->CreateTexture2D(&desc, &subData, &m_texture);
    if (FAILED(hr)) {
        stbi_image_free(data);
        return hr;
    }

    hr = device.m_device->CreateShaderResourceView(m_texture, nullptr, &m_textureFromImg);

    D3D11_SAMPLER_DESC sampDesc = {};
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

    device.m_device->CreateSamplerState(&sampDesc, &m_samplerState);

    stbi_image_free(data);
    return hr;
}

// ----------------------------------------------------------------------------------
// 2. INIT MANUAL
// ----------------------------------------------------------------------------------
HRESULT Texture::init(Device& device,
    unsigned int width,
    unsigned int height,
    DXGI_FORMAT Format,
    unsigned int BindFlags,
    unsigned int sampleCount,
    unsigned int qualityLevels) {

    if (!device.m_device) return E_POINTER;

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

    HRESULT hr = device.m_device->CreateTexture2D(&desc, nullptr, &m_texture);
    if (FAILED(hr)) {
        ERROR("Texture", "init (Manual)", "Failed to create Texture2D");
        return hr;
    }

    if (BindFlags & D3D11_BIND_SHADER_RESOURCE) {
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = Format;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.MipLevels = 1;

        hr = device.m_device->CreateShaderResourceView(m_texture, &srvDesc, &m_textureFromImg);
        if (FAILED(hr)) {
            ERROR("Texture", "init (Manual)", "Failed to create SRV");
            return hr;
        }
    }

    return S_OK;
}

// ----------------------------------------------------------------------------------
// 3. INIT DESDE REFERENCIA
// ----------------------------------------------------------------------------------
HRESULT Texture::init(Device& device, Texture& textureRef, DXGI_FORMAT format) {
    if (!device.m_device || !textureRef.m_texture) return E_POINTER;

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;
    srvDesc.Texture2D.MostDetailedMip = 0;

    HRESULT hr = device.m_device->CreateShaderResourceView(textureRef.m_texture, &srvDesc, &m_textureFromImg);
    if (FAILED(hr)) {
        ERROR("Texture", "init (Ref)", "Failed to create SRV from ref");
        return hr;
    }
    return S_OK;
}

// ----------------------------------------------------------------------------------
// 4. CREATE CUBEMAP
// ----------------------------------------------------------------------------------
HRESULT Texture::CreateCubemap(Device& device,
    DeviceContext& deviceContext,
    const std::array<std::string, 6>& facePaths,
    bool generateMips) {

    // Llamamos a destroy manual para limpiar si había algo antes
    destroy();

    stbi_set_flip_vertically_on_load(false);

    int width = 0, height = 0, channels = 0;
    std::vector<unsigned char*> facePixels(6, nullptr);

    for (int i = 0; i < 6; ++i) {
        std::string fullPath = "Assets/" + facePaths[i];
        int w, h, c;
        facePixels[i] = stbi_load(fullPath.c_str(), &w, &h, &c, 4);

        if (!facePixels[i]) {
            for (auto* p : facePixels) if (p) stbi_image_free(p);
            // Logueamos la ruta completa para debug
            ERROR("Texture", "CreateCubemap", ("Failed to load face: " + fullPath).c_str());
            return E_FAIL;
        }

        if (i == 0) {
            width = w; height = h;
        }
        else if (w != width || h != height) {
            for (auto* p : facePixels) if (p) stbi_image_free(p);
            ERROR("Texture", "CreateCubemap", "All cubemap faces must have the same dimensions");
            return E_FAIL;
        }
    }

    D3D11_TEXTURE2D_DESC texDesc = {};
    texDesc.Width = width;
    texDesc.Height = height;
    texDesc.MipLevels = generateMips ? 0 : 1;
    texDesc.ArraySize = 6;
    texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    texDesc.SampleDesc.Count = 1;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | (generateMips ? D3D11_BIND_RENDER_TARGET : 0);
    texDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE | (generateMips ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0);

    HRESULT hr = S_OK;

    if (!generateMips) {
        D3D11_SUBRESOURCE_DATA pData[6];
        for (int i = 0; i < 6; i++) {
            pData[i].pSysMem = facePixels[i];
            pData[i].SysMemPitch = width * 4;
            pData[i].SysMemSlicePitch = 0;
        }
        hr = device.m_device->CreateTexture2D(&texDesc, pData, &m_texture);
    }
    else {
        hr = device.m_device->CreateTexture2D(&texDesc, nullptr, &m_texture);
        if (SUCCEEDED(hr)) {
            for (int i = 0; i < 6; ++i) {
                deviceContext.m_deviceContext->UpdateSubresource(
                    m_texture,
                    D3D11CalcSubresource(0, i, 0),
                    nullptr,
                    facePixels[i],
                    width * 4,
                    0
                );
            }
        }
    }

    if (FAILED(hr)) {
        for (auto* p : facePixels) stbi_image_free(p);
        ERROR("Texture", "CreateCubemap", "Failed to create Cubemap Texture2D");
        return hr;
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = texDesc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
    srvDesc.TextureCube.MostDetailedMip = 0;
    srvDesc.TextureCube.MipLevels = generateMips ? -1 : 1;

    hr = device.m_device->CreateShaderResourceView(m_texture, &srvDesc, &m_textureFromImg);

    if (generateMips && SUCCEEDED(hr)) {
        deviceContext.m_deviceContext->GenerateMips(m_textureFromImg);
    }

    for (auto* p : facePixels) stbi_image_free(p);

    m_textureName = "Cubemap";
    return hr;
}

ID3D11ShaderResourceView* Texture::CreateCubemapFaceSRV(ID3D11Device* device, ID3D11Texture2D* texture, DXGI_FORMAT format, UINT faceIndex, UINT mipLevel) {
    if (!device || !texture) return nullptr;

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
    srvDesc.Texture2DArray.MostDetailedMip = mipLevel;
    srvDesc.Texture2DArray.MipLevels = 1;
    srvDesc.Texture2DArray.FirstArraySlice = faceIndex;
    srvDesc.Texture2DArray.ArraySize = 1;

    ID3D11ShaderResourceView* faceView = nullptr;
    device->CreateShaderResourceView(texture, &srvDesc, &faceView);
    return faceView;
}

void Texture::render(DeviceContext& deviceContext, unsigned int startSlot, unsigned int numViews) {
    if (deviceContext.m_deviceContext && m_textureFromImg) {
        deviceContext.m_deviceContext->PSSetShaderResources(startSlot, numViews, &m_textureFromImg);
        if (m_samplerState) {
            deviceContext.m_deviceContext->PSSetSamplers(startSlot, numViews, &m_samplerState);
        }
    }
}

void Texture::update() {}

// ----------------------------------------------------------------------------------
// CORRECCIÓN 2: DESCOMENTAR LA IMPLEMENTACIÓN DE DESTROY
// Esta función es necesaria para que BaseApp::destroy() limpie la memoria.
// ----------------------------------------------------------------------------------
void Texture::destroy() {
    SAFE_RELEASE(m_textureFromImg);
    SAFE_RELEASE(m_samplerState);
    SAFE_RELEASE(m_texture);
}