/**
 * @file Texture.h
 * @brief Clase encargada de la gestión de recursos de textura en DirectX 11.
 */

#pragma once
#include "Prerequisites.h"
#include "Device.h"
#include "DeviceContext.h"
#include <string>
#include <vector>
#include <array>

 

 /**
  * @class Texture
  * @brief Encapsula una textura 2D de Direct3D 11, gestionando su recurso, vistas de shader y estados de muestreo.
  * * Implementa la Regla de los Tres para garantizar una gestión segura de los contadores de
  * referencia de los objetos COM de DirectX al realizar copias.
  */
class Texture {
public:
    /**
     * @brief Constructor por defecto.
     */
    Texture() = default;


    /**
     * @brief Destructor. Se encarga de liberar los recursos de DirectX.
     */
    ~Texture();

    /**
     * @brief Constructor de copia. Incrementa las referencias de los recursos existentes.
     * @param other Referencia a la textura a copiar.
     */
    Texture(const Texture& other);

    /**
     * @brief Operador de asignación. Gestiona la liberación del recurso actual y la copia del nuevo.
     * @param other Referencia a la textura a asignar.
     * @return Referencia a la instancia actual.
     */
    Texture& operator=(const Texture& other);
    // ------------------------------------------------

    /**
     * @brief Inicializa una textura cargándola desde un archivo de imagen.
     * @param device Referencia al dispositivo de hardware.
     * @param textureName Nombre o ruta del archivo (sin extensión).
     * @param extensionType Formato del archivo (PNG, JPG, etc.).
     * @return Código HRESULT de la operación.
     */
    HRESULT init(Device& device, const std::string& textureName, ExtensionType extensionType);

    /**
     * @brief Inicializa una textura vacía con parámetros manuales.
     * @details Útil para crear Render Targets, Depth Buffers o texturas dinámicas.
     * @param device Referencia al dispositivo de hardware.
     * @param width Ancho en píxeles.
     * @param height Alto en píxeles.
     * @param Format Formato de datos (DXGI_FORMAT).
     * @param BindFlags Banderas de uso (ej. D3D11_BIND_RENDER_TARGET).
     * @param sampleCount Conteo de muestras para MSAA.
     * @param qualityLevels Niveles de calidad para MSAA.
     * @return Código HRESULT de la operación.
     */
    HRESULT init(Device& device, unsigned int width, unsigned int height, DXGI_FORMAT Format, unsigned int BindFlags, unsigned int sampleCount = 1, unsigned int qualityLevels = 0);

    /**
     * @brief Inicializa una vista de recurso a partir de un recurso de textura existente.
     * @param device Referencia al dispositivo.
     * @param textureRef Referencia a la textura base.
     * @param format Formato de la vista.
     * @return Código HRESULT de la operación.
     */
    HRESULT init(Device& device, Texture& textureRef, DXGI_FORMAT format);

    /**
     * @brief Crea un Cubemap (Skybox) a partir de 6 rutas de archivos.
     * @param device Referencia al dispositivo.
     * @param deviceContext Contexto del dispositivo para actualización de subrecursos.
     * @param facePaths Arreglo con las 6 rutas de imagen para las caras del cubo.
     * @param generateMips Indica si se deben generar Mipmaps automáticamente.
     * @return Código HRESULT de la operación.
     */
    HRESULT CreateCubemap(Device& device, DeviceContext& deviceContext, const std::array<std::string, 6>& facePaths, bool generateMips = false);

    /**
     * @brief Genera una vista (SRV) de una cara específica del cubemap tratada como textura 2D.
     * @details Función de utilidad para mostrar caras individuales en la interfaz de ImGui.
     * @param device Puntero al dispositivo de Direct3D.
     * @param cubemapTex Recurso de textura de cubo.
     * @param format Formato de la vista.
     * @param faceIndex Índice de la cara (0-5).
     * @param mipLevels Niveles de mipmap a incluir.
     * @return Puntero a la vista de recurso de shader creada.
     */
    ID3D11ShaderResourceView* CreateCubemapFaceSRV(ID3D11Device* device, ID3D11Texture2D* cubemapTex, DXGI_FORMAT format, UINT faceIndex, UINT mipLevels = 1);

    /**
     * @brief Vincula la textura a la etapa de Pixel Shader.
     * @param deviceContext Contexto del dispositivo.
     * @param startSlot Slot inicial de vinculación.
     * @param numViews Número de vistas a vincular.
     */
    void render(DeviceContext& deviceContext, unsigned int startSlot, unsigned int numViews);

    /**
     * @brief Método para actualización de datos de textura (Placeholder).
     */
    void update();

    /**
     * @brief Libera todos los recursos COM de DirectX y limpia punteros.
     */
    void destroy();

public:
    ID3D11Texture2D* m_texture = nullptr;           /**< Puntero al recurso de textura real en memoria de video. */
    ID3D11ShaderResourceView* m_textureFromImg = nullptr; /**< Vista que permite a los shaders leer los datos de la textura. */
    ID3D11SamplerState* m_samplerState = nullptr;   /**< Estado que define cómo se filtran y direccionan las muestras de la textura. */
    std::string m_textureName;                      /**< Nombre o identificador de la textura. */
};