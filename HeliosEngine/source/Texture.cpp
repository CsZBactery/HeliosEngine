#include "../include/Texture.h"
#include "../include/Device.h"
#include "../include/DeviceContext.h"

//
// La primera funci�n `init` est� dise�ada para cargar una textura desde un archivo,
// pero su implementaci�n actual est� incompleta (`E_NOTIMPL`).
//
HRESULT
Texture::init(Device& device,
    const std::string& textureName,
    ExtensionType extensionType) {
    //return E_NOTIMPL;
    if (!device.m_device) {
        ERROR("Texture", "init", "Device is null.");
        return E_POINTER;
    }
    if (textureName.empty()) {
        ERROR("Texture", "init", "Texture name cannot be empty.");
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;

    switch (extensionType) {
    case DDS: {
        m_textureName = textureName + ".dds";
        // Cargar textura DDS
        hr = D3DX11CreateShaderResourceViewFromFile(
            device.m_device,
            m_textureName.c_str(),
            nullptr,
            nullptr,
            &m_textureFromImg,
            nullptr
        );

        if (FAILED(hr)) {
            ERROR("Texture", "init",
                ("Failed to load DDS texture. Verify filepath: " + m_textureName).c_str());
            return hr;
        }
        break;
    }

    case PNG: {

        break;
    }
    case JPG: {

        break;
    }
    default:
        ERROR("Texture", "init", "Unsupported extension type");
        return E_INVALIDARG;
    }

    return hr;

}

//
// La segunda funci�n `init` crea una textura vac�a en la GPU con los par�metros especificados.
// Esta textura puede usarse como un render target o un buffer de profundidad.
//
HRESULT
Texture::init(Device& device,
    unsigned int width,
    unsigned int height,
    DXGI_FORMAT Format,
    unsigned int BindFlags,
    unsigned int sampleCount,
    unsigned int qualityLevels) {
    // Se verifica que el dispositivo sea v�lido y que el ancho y alto no sean cero.
    if (!device.m_device) {
        ERROR("Texture", "init", "Device is null");
        return E_POINTER;
    }
    if (width == 0 || height == 0) {
        ERROR("Texture", "init", "Width and height must be greater than 0");
        return E_INVALIDARG;
    }

    //
    // Se configura la descripci�n de la textura 2D (`D3D11_TEXTURE2D_DESC`).
    // Se especifican el ancho, el alto, el formato y las propiedades de uso (flags de enlace, etc.).
    //
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

    // Se crea la textura usando la funci�n del dispositivo de Direct3D.
    HRESULT hr = device.CreateTexture2D(&desc, nullptr, &m_texture);

    // Se verifica si la creaci�n fue exitosa y se devuelve el resultado.
    if (FAILED(hr)) {
        ERROR("Texture", "init",
            ("Failed to create texture with specified params. HRESULT: " + std::to_string(hr)).c_str());
        return hr;
    }

    return S_OK;
}

//
// La tercera funci�n `init` crea una vista de recurso de sombreador (`Shader Resource View`)
// a partir de una textura existente. Esto permite que la textura sea utilizada por los sombreadores.
//
HRESULT
Texture::init(Device& device, Texture& textureRef, DXGI_FORMAT format) {
    // Se verifica que el dispositivo y la textura de referencia no sean nulos.
    if (!device.m_device) {
        ERROR("Texture", "init", "Device is null.");
        return E_POINTER;
    }
    if (!textureRef.m_texture) {
        ERROR("Texture", "init", "Texture is null");
        return E_POINTER;
    }

    //
    // Se configura la descripci�n para la vista del recurso de sombreador.
    // Se especifica el formato y la dimensi�n de la vista.
    //
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;
    srvDesc.Texture2D.MostDetailedMip = 0;

    // Se crea la vista de recurso de sombreador a partir de la textura de referencia.
    HRESULT hr = device.m_device->CreateShaderResourceView(textureRef.m_texture,
        &srvDesc,
        &m_textureFromImg);

    // Se verifica si la creaci�n fue exitosa.
    if (FAILED(hr)) {
        ERROR("Texture", "init",
            ("Failed to create shader resource view for texture. HRESULT: " +
                std::to_string(hr)).c_str());
        return hr;
    }
    return S_OK;
}

//
// La funci�n `update` est� vac�a, lo que sugiere que no hay l�gica de actualizaci�n
// de la textura en tiempo de ejecuci�n en esta implementaci�n.
//
void
Texture::update() {
}

//
// La funci�n `render` asigna la vista de recurso de sombreador al Pixel Shader.
// Esto hace que la textura est� disponible para que el sombreador la lea y la use.
//
void
Texture::render(DeviceContext& deviceContext,
    unsigned int StartSlot,
    unsigned int NumViews) {
    // Se verifica que el contexto del dispositivo sea v�lido.
    if (!deviceContext.m_deviceContext) {
        ERROR("Texture", "render", "Device Context is null.");
        return;
    }

    // Se asigna el recurso si la vista de recurso de sombreador es v�lida.
    if (m_textureFromImg) {
        deviceContext.PSSetShaderResources(StartSlot,
            NumViews,
            &m_textureFromImg);
    }
}

//
// La funci�n `destroy` libera todos los recursos de Direct3D de la textura.
//
void
Texture::destroy() {
    // Se liberan las interfaces de forma segura.
    if (m_texture) {
        SAFE_RELEASE(m_texture);
    }
    if (m_textureFromImg) {
        SAFE_RELEASE(m_textureFromImg);
    }
}