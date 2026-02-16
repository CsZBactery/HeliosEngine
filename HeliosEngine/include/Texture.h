#pragma once
#include "Prerequisites.h"
#include "Device.h"
#include "DeviceContext.h"
#include <string>
#include <vector>
#include <array>

// NOTA: ExtensionType debe estar en Prerequisites.h para evitar redefiniciones.
// Si te da error de "undeclared identifier", asegúrate de que Prerequisites.h lo tenga.

class Texture {
public:
    Texture() = default;

    // --- REGLA DE LOS TRES (Para evitar el Crash) ---
    ~Texture(); // 1. Destructor
    Texture(const Texture& other); // 2. Constructor de Copia
    Texture& operator=(const Texture& other); // 3. Operador de Asignación
    // ------------------------------------------------

    // Cargar desde archivo
    HRESULT init(Device& device, const std::string& textureName, ExtensionType extensionType);

    // Crear manualmente
    HRESULT init(Device& device, unsigned int width, unsigned int height, DXGI_FORMAT Format, unsigned int BindFlags, unsigned int sampleCount = 1, unsigned int qualityLevels = 0);

    // Inicializar copia
    HRESULT init(Device& device, Texture& textureRef, DXGI_FORMAT format);

    // Crear Skybox
    HRESULT CreateCubemap(Device& device, DeviceContext& deviceContext, const std::array<std::string, 6>& facePaths, bool generateMips = false);

    // Crear vista para ImGui
    ID3D11ShaderResourceView* CreateCubemapFaceSRV(ID3D11Device* device, ID3D11Texture2D* cubemapTex, DXGI_FORMAT format, UINT faceIndex, UINT mipLevels = 1);

    void render(DeviceContext& deviceContext, unsigned int startSlot, unsigned int numViews);
    void update();
    void destroy();

public:
    ID3D11Texture2D* m_texture = nullptr;
    ID3D11ShaderResourceView* m_textureFromImg = nullptr;
    ID3D11SamplerState* m_samplerState = nullptr;
    std::string m_textureName;
};