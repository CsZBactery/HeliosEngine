/**
 * @file Texture.h
 * @brief Clase encargada de la gestión de recursos de textura en DirectX 11.
 * @ingroup core
 */

#pragma once
#include "Prerequisites.h"
#include <string>
#include <vector>
#include <array>

 // Forward Declarations
class Device;
class DeviceContext;

/**
 * @class Texture
 * @brief Encapsula una textura 2D de Direct3D 11, gestionando su recurso y vistas de shader.
 * @details Esta clase implementa la **Regla de los Tres** para garantizar una gestión segura
 * de los contadores de referencia de los objetos COM de DirectX al realizar copias o asignaciones.
 * Administra texturas desde archivos (PNG, JPG), texturas creadas en memoria (G-Buffer) y Cubemaps.
 */
class Texture {
public:
    // ======================================================================================
    // GESTIÓN DE MEMORIA (Regla de los Tres)
    // ======================================================================================

    /** @brief Constructor por defecto. */
    Texture() = default;

    /** @brief Destructor. Libera automáticamente los recursos mediante destroy(). */
    ~Texture();

    /** * @brief Constructor de copia.
     * @details Incrementa las referencias COM de los recursos existentes para evitar liberaciones prematuras.
     */
    Texture(const Texture& other);

    /** * @brief Operador de asignación.
     * @details Gestiona la liberación segura del recurso actual antes de copiar el nuevo.
     */
    Texture& operator=(const Texture& other);

    // ======================================================================================
    // INICIALIZACIÓN
    // ======================================================================================

    /**
     * @brief Inicializa una textura cargándola desde un archivo de imagen.
     * @param device Referencia al dispositivo de hardware.
     * @param textureName Nombre o ruta del archivo.
     * @param extensionType Formato del archivo (PNG, JPG, DDS, etc.).
     * @return HRESULT S_OK si la carga fue exitosa.
     * @post m_texture y m_textureFromImg != nullptr.
     */
    HRESULT init(Device& device, const std::string& textureName, ExtensionType extensionType);

    /**
     * @brief Inicializa una textura vacía con parámetros manuales.
     * @details Crucial para crear componentes del G-Buffer, Render Targets o Depth Buffers.
     * @param device Referencia al dispositivo de hardware.
     * @param width Ancho en píxeles.
     * @param height Alto en píxeles.
     * @param Format Formato DXGI (ej. DXGI_FORMAT_R8G8B8A8_UNORM).
     * @param BindFlags Banderas de uso (ej. D3D11_BIND_RENDER_TARGET).
     * @param sampleCount Conteo de muestras para MSAA (por defecto 1).
     * @param qualityLevels Niveles de calidad para MSAA.
     */
    HRESULT init(Device& device, unsigned int width, unsigned int height, DXGI_FORMAT Format, unsigned int BindFlags, unsigned int sampleCount = 1, unsigned int qualityLevels = 0);

    /**
     * @brief Inicializa una vista de recurso a partir de un recurso de textura existente.
     * @param device Referencia al dispositivo.
     * @param textureRef Referencia a la textura base de la cual se copiará la descripción.
     * @param format Formato de la nueva vista.
     */
    HRESULT init(Device& device, Texture& textureRef, DXGI_FORMAT format);

    // ======================================================================================
    // CUBEMAPS Y SKYBOX
    // ======================================================================================

    /**
     * @brief Crea un Cubemap (TextureCube) a partir de 6 rutas de archivos.
     * @param device Referencia al dispositivo.
     * @param deviceContext Contexto para actualización de subrecursos.
     * @param facePaths Arreglo con las 6 rutas de imagen (PosX, NegX, PosY, NegY, PosZ, NegZ).
     * @param generateMips Indica si se deben generar Mipmaps automáticamente.
     */
    HRESULT CreateCubemap(Device& device, DeviceContext& deviceContext, const std::array<std::string, 6>& facePaths, bool generateMips = false);

    /**
     * @brief Genera una vista (SRV) de una cara específica del cubemap.
     * @details Utilidad necesaria para mostrar caras individuales en la interfaz del editor.
     */
    ID3D11ShaderResourceView* CreateCubemapFaceSRV(ID3D11Device* device, ID3D11Texture2D* cubemapTex, DXGI_FORMAT format, UINT faceIndex, UINT mipLevels = 1);

    // ======================================================================================
    // PIPELINE Y LIMPIEZA
    // ======================================================================================

    /**
     * @brief Vincula la textura a la etapa de Pixel Shader.
     * @param deviceContext Contexto del dispositivo.
     * @param startSlot Slot inicial de vinculación (ej. t0, t1 en HLSL).
     * @param numViews Número de vistas a vincular simultáneamente.
     */
    void render(DeviceContext& deviceContext, unsigned int startSlot, unsigned int numViews);

    /** @brief Método placeholder para actualizaciones dinámicas. */
    void update();

    /** @brief Libera todos los recursos COM de DirectX y limpia los punteros. */
    void destroy();

public:
    /** @brief Puntero al recurso de textura real en memoria de video (VRAM). */
    ID3D11Texture2D* m_texture = nullptr;

    /** @brief Vista de Shader (SRV) que permite a los programas HLSL leer la textura. */
    ID3D11ShaderResourceView* m_textureFromImg = nullptr;

    /** @brief Estado de muestreo opcional vinculado a esta textura. */
    ID3D11SamplerState* m_samplerState = nullptr;

    /** @brief Nombre o identificador para depuración en el Inspector. */
    std::string m_textureName;
};