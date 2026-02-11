#include "Texture.h"
// Asegúrate de que STB_IMAGE esté disponible. 
#include "stb_image.h" 

// ... (Tus implementaciones existentes de init(string), render y destroy MANTENLAS IGUAL) ...
// ... Si no tienes el init original a mano, avísame, pero asumo que ya lo tienes ...

// ----------------------------------------------------------------------------------
// NUEVA IMPLEMENTACIÓN: INIT MANUAL (Para DepthStencil / RenderTargets)
// ----------------------------------------------------------------------------------
HRESULT Texture::init(Device& device, int width, int height, DXGI_FORMAT format,
    unsigned int bindFlags, int sampleCount, int sampleQuality) {

    if (!device.m_device) return E_POINTER;

    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = format;
    desc.SampleDesc.Count = sampleCount;
    desc.SampleDesc.Quality = sampleQuality;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = bindFlags;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;

    // Crear la textura cruda
    HRESULT hr = device.m_device->CreateTexture2D(&desc, nullptr, &m_texture);
    if (FAILED(hr)) {
        ERROR("Texture", "init (Manual)", "Failed to create Texture2D");
        return hr;
    }

    // Si el bind flag incluye SHADER_RESOURCE, creamos la vista SRV
    if (bindFlags & D3D11_BIND_SHADER_RESOURCE) {
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = format;
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
// IMPLEMENTACIÓN DE LOS MÉTODOS PARA CUBEMAP
// ----------------------------------------------------------------------------------

void Texture::CreateCubemap(Device& device, DeviceContext& deviceContext, const std::array<std::string, 6>& faces, bool flip) {
    int width, height, nrChannels;
    unsigned char* data = nullptr;
    std::vector<unsigned char*> texturesData;

    for (const auto& path : faces) {
        std::string fullPath = "Assets/" + path;
        // stbi_set_flip_vertically_on_load(flip);
        data = stbi_load(fullPath.c_str(), &width, &height, &nrChannels, 4);

        if (data) {
            texturesData.push_back(data);
        }
        else {
            ERROR("Texture", "CreateCubemap", ("Failed to load cubemap face: " + fullPath).c_str());
            return;
        }
    }

    D3D11_TEXTURE2D_DESC texDesc = {};
    texDesc.Width = width;
    texDesc.Height = height;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 6;
    texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    texDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

    D3D11_SUBRESOURCE_DATA pData[6];
    for (int i = 0; i < 6; i++) {
        pData[i].pSysMem = texturesData[i];
        pData[i].SysMemPitch = width * 4;
        pData[i].SysMemSlicePitch = 0;
    }

    HRESULT hr = device.m_device->CreateTexture2D(&texDesc, pData, &m_texture);
    if (FAILED(hr)) ERROR("Texture", "CreateCubemap", "Failed to create Cubemap Texture2D");

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = texDesc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
    srvDesc.TextureCube.MostDetailedMip = 0;
    srvDesc.TextureCube.MipLevels = 1;

    hr = device.m_device->CreateShaderResourceView(m_texture, &srvDesc, &m_textureFromImg);
    if (FAILED(hr)) ERROR("Texture", "CreateCubemap", "Failed to create Cubemap SRV");

    for (auto* ptr : texturesData) {
        stbi_image_free(ptr);
    }
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