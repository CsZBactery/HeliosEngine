// ======================================================================================
// Archivo: Texture.cpp
// Implementación de carga, creación y gestión de texturas (2D y Cubemaps).
// ======================================================================================

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h" // Librería para cargar formatos de imagen
#include "Texture.h"
#include "Device.h"
#include "DeviceContext.h"

// ===========================================================
// GESTIÓN DE MEMORIA
// ===========================================================

// Destructor: Se asegura de liberar los recursos de la GPU cuando el objeto se destruye
Texture::~Texture() {
    destroy();
}

// Constructor de Copia: Permite duplicar el objeto Texture de forma segura.
// Al copiar, incrementamos el contador de referencias de DirectX (AddRef) 
// para que la GPU no borre la textura mientras haya alguien usándola.
Texture::Texture(const Texture& other) {
    m_texture = other.m_texture;
    m_textureFromImg = other.m_textureFromImg;
    m_samplerState = other.m_samplerState;
    m_textureName = other.m_textureName;

    if (m_texture) m_texture->AddRef();
    if (m_textureFromImg) m_textureFromImg->AddRef();
    if (m_samplerState) m_samplerState->AddRef();
}

// Operador de Asignación: Maneja la copia entre objetos ya existentes
Texture& Texture::operator=(const Texture& other) {
    if (this != &other) {
        destroy(); // Limpiamos lo que teníamos antes de recibir lo nuevo

        m_texture = other.m_texture;
        m_textureFromImg = other.m_textureFromImg;
        m_samplerState = other.m_samplerState;
        m_textureName = other.m_textureName;

        if (m_texture) m_texture->AddRef();
        if (m_textureFromImg) m_textureFromImg->AddRef();
        if (m_samplerState) m_samplerState->AddRef();
    }
    return *this;
}

// Libera los punteros de DirectX y los pone en null para evitar basura en memoria
void Texture::destroy() {
    if (m_texture) { m_texture->Release(); m_texture = nullptr; }
    if (m_textureFromImg) { m_textureFromImg->Release(); m_textureFromImg = nullptr; }
    if (m_samplerState) { m_samplerState->Release(); m_samplerState = nullptr; }
}

// ===========================================================
// INICIALIZACIÓN Y CARGA
// ===========================================================

// Carga una imagen desde el disco y la sube a la memoria de la tarjeta de video
HRESULT Texture::init(Device& device, const std::string& textureName, ExtensionType extensionType) {
    if (!device.m_device) return E_POINTER;

    // Determinamos la extensión del archivo según el tipo enviado
    std::string ext;
    switch (extensionType) {
    case PNG: ext = ".png"; break;
    case JPG: ext = ".jpg"; break;
    case TGA: ext = ".tga"; break;
    default: ext = ".png"; break;
    }
    m_textureName = textureName + ext;

    int width, height, channels;
    // Cargamos los píxeles de la imagen en RAM usando la librería STB
    unsigned char* data = stbi_load(m_textureName.c_str(), &width, &height, &channels, 4);

    // Si no abre, intentamos buscarla en la carpeta de Assets por defecto
    if (!data) {
        std::string altPath = "Assets/Textures/" + m_textureName;
        data = stbi_load(altPath.c_str(), &width, &height, &channels, 4);
        if (!data) return E_FAIL;
    }

    // Definimos cómo se guardará la textura en la GPU
    D3D11_TEXTURE2D_DESC textureDesc = {};
    textureDesc.Width = width;
    textureDesc.Height = height;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;
    textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // Formato estándar de color
    textureDesc.SampleDesc.Count = 1;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    // Pasamos los píxeles cargados de la RAM a la estructura de DirectX
    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = data;
    initData.SysMemPitch = width * 4;

    // Creamos el recurso de textura en el dispositivo
    HRESULT hr = device.m_device->CreateTexture2D(&textureDesc, &initData, &m_texture);
    stbi_image_free(data); // Una vez subida a la GPU, liberamos la copia de la RAM

    if (FAILED(hr)) return hr;

    // Creamos la Vista (SRV) que es lo que el Shader realmente usa para "leer" la textura
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = textureDesc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;

    return device.m_device->CreateShaderResourceView(m_texture, &srvDesc, &m_textureFromImg);
}

// Crea una textura vacía (útil para Buffers de profundidad o Render Targets)
HRESULT Texture::init(Device& device, unsigned int width, unsigned int height, DXGI_FORMAT Format, unsigned int BindFlags, unsigned int sampleCount, unsigned int qualityLevels) {
    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = Format;

    // 🔥 AQUÍ ESTABA EL BUG. AHORA RESPETA LOS VALORES DE MSAA ENVIADOS DESDE BaseApp.cpp 🔥
    desc.SampleDesc.Count = sampleCount;
    desc.SampleDesc.Quality = qualityLevels;

    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = BindFlags;

    return device.m_device->CreateTexture2D(&desc, nullptr, &m_texture);
}

// Crea una Vista (SRV) a partir de una textura ya existente
HRESULT Texture::init(Device& device, Texture& textureRef, DXGI_FORMAT format) {
    if (!textureRef.m_texture) return E_FAIL;

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;

    return device.m_device->CreateShaderResourceView(textureRef.m_texture, &srvDesc, &m_textureFromImg);
}

void Texture::update() {}

// Vincula la textura al Pixel Shader para que se pueda dibujar
void Texture::render(DeviceContext& deviceContext, unsigned int StartSlot, unsigned int NumViews) {
    if (m_textureFromImg) {
        deviceContext.m_deviceContext->PSSetShaderResources(StartSlot, NumViews, &m_textureFromImg);
    }
}

// ===========================================================
// MANEJO DE SKYBOX (CUBEMAPS)
// ===========================================================

// Crea un mapa de cubo cargando 6 imágenes diferentes (una para cada cara)
HRESULT Texture::CreateCubemap(Device& device, DeviceContext& deviceContext, const std::array<std::string, 6>& facePaths, bool generateMips) {
    destroy();
    stbi_set_flip_vertically_on_load(false); // Evitamos voltear el cielo

    int width = 0, height = 0, c = 0;
    std::array<unsigned char*, 6> facePixels{};

    // Cargamos las 6 imágenes desde el disco
    for (int i = 0; i < 6; ++i) {
        facePixels[i] = stbi_load(facePaths[i].c_str(), &width, &height, &c, 4);
        if (!facePixels[i]) return E_FAIL;
    }

    // Definimos la textura con ArraySize = 6 y la bandera de TEXTURECUBE
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

    // Organizamos los datos de las 6 caras para DirectX
    std::array<D3D11_SUBRESOURCE_DATA, 6> data = {};
    for (int i = 0; i < 6; ++i) {
        data[i].pSysMem = facePixels[i];
        data[i].SysMemPitch = width * 4;
        data[i].SysMemSlicePitch = 0;
    }

    HRESULT hr = device.m_device->CreateTexture2D(&texDesc, data.data(), &m_texture);

    if (SUCCEEDED(hr)) {
        // Creamos la vista especial para Cubemaps
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = texDesc.Format;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
        srvDesc.TextureCube.MostDetailedMip = 0;
        srvDesc.TextureCube.MipLevels = 1;
        hr = device.m_device->CreateShaderResourceView(m_texture, &srvDesc, &m_textureFromImg);
    }

    // Limpiamos los píxeles de la RAM
    for (auto* p : facePixels) stbi_image_free(p);
    return hr;
}

// Genera una vista de una sola cara del cubo (usado para previsualizar en la interfaz)
ID3D11ShaderResourceView* Texture::CreateCubemapFaceSRV(ID3D11Device* device, ID3D11Texture2D* cubemapTex, DXGI_FORMAT format, UINT faceIndex, UINT mipLevels) {
    if (!cubemapTex) return nullptr;
    D3D11_SHADER_RESOURCE_VIEW_DESC d = {};
    d.Format = format;
    // Engañamos al shader diciéndole que es un arreglo de texturas 2D para ver solo una
    d.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
    d.Texture2DArray.MostDetailedMip = 0;
    d.Texture2DArray.MipLevels = 1;
    d.Texture2DArray.FirstArraySlice = faceIndex; // Elegimos qué cara ver
    d.Texture2DArray.ArraySize = 1;

    ID3D11ShaderResourceView* srv = nullptr;
    device->CreateShaderResourceView(cubemapTex, &d, &srv);
    return srv;
}