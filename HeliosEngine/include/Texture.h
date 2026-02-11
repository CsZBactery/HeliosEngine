#pragma once
#include "Prerequisites.h"
#include "Device.h"
#include "DeviceContext.h"
#include <string>
#include <vector>
#include <array>

/**
 * @class Texture
 * @brief Encapsula una textura 2D en Direct3D 11 (Versión HeliosEngine + Extensions).
 */
class Texture {
public:
    Texture() = default;
    ~Texture();

    /**
     * @brief Inicializa una textura cargada desde archivo (Lógica Original Helios + Mejoras).
     */
    HRESULT init(Device& device, const std::string& textureName, ExtensionType extensionType);

    /**
     * @brief Inicializa una textura creada manualmente desde memoria (Nuevo: Lógica Profe).
     * Útil para Render Targets y Depth Buffers.
     */
    HRESULT init(Device& device,
        unsigned int width,
        unsigned int height,
        DXGI_FORMAT Format,
        unsigned int BindFlags,
        unsigned int sampleCount = 1,
        unsigned int qualityLevels = 0);

    /**
     * @brief Inicializa una textura a partir de otra referencia (Nuevo: Lógica Profe).
     */
    HRESULT init(Device& device, Texture& textureRef, DXGI_FORMAT format);

    /**
     * @brief Crea un Cubemap (Skybox) a partir de 6 rutas de imagen (Nuevo: Lógica Profe).
     */
    HRESULT CreateCubemap(Device& device,
        DeviceContext& deviceContext,
        const std::array<std::string, 6>& facePaths,
        bool generateMips = false);

    /**
     * @brief Crea una vista SRV para una cara específica del cubemap (Para Debug/ImGui).
     */
    ID3D11ShaderResourceView* CreateCubemapFaceSRV(
        ID3D11Device* device,
        ID3D11Texture2D* cubemapTex,
        DXGI_FORMAT format,
        UINT faceIndex,
        UINT mipLevels = 1);

    /**
     * @brief Vincula la textura al Pixel Shader.
     */
    void render(DeviceContext& deviceContext, unsigned int startSlot, unsigned int numViews);

    /**
     * @brief Libera los recursos.
     */
    void destroy();

    void update(); // Placeholder

public:
    // Recursos públicos para acceso rápido (ImGui/BaseApp)
    ID3D11Texture2D* m_texture = nullptr;        ///< Recurso crudo
    ID3D11ShaderResourceView* m_textureFromImg = nullptr; ///< Vista para el Shader
    ID3D11SamplerState* m_samplerState = nullptr;   ///< Estado de muestreo
    std::string               m_textureName;
};