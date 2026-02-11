#pragma once
#include "Prerequisites.h"
#include "Device.h"
#include "DeviceContext.h"
#include <string>
#include <vector>
#include <array> 

class Texture {
public:
    Texture() = default;
    ~Texture();

    /**
     * @brief Inicializa una textura 2D cargándola desde archivo.
     */
    HRESULT init(Device device, const std::string& textureName, ExtensionType extensionType);

    /**
     * @brief Inicializa una textura vacía manualmente (NECESARIO PARA DEPTH STENCIL).
     * Soluciona el error: "function does not take 7 arguments".
     */
    HRESULT init(Device& device, int width, int height, DXGI_FORMAT format,
        unsigned int bindFlags, int sampleCount = 1, int sampleQuality = 0);

    /**
     * @brief Crea un Cubemap (Skybox).
     */
    void CreateCubemap(Device& device, DeviceContext& deviceContext, const std::array<std::string, 6>& faces, bool flip = false);

    /**
     * @brief Crea una vista SRV de una cara específica (Debug GUI).
     */
    ID3D11ShaderResourceView* CreateCubemapFaceSRV(ID3D11Device* device, ID3D11Texture2D* texture, DXGI_FORMAT format, UINT faceIndex, UINT mipLevel);

    void render(DeviceContext& deviceContext, unsigned int startSlot, unsigned int numViews);

    void destroy();

public:
    ID3D11ShaderResourceView* m_textureFromImg = nullptr;
    ID3D11SamplerState* m_samplerState = nullptr;
    ID3D11Texture2D* m_texture = nullptr;
};