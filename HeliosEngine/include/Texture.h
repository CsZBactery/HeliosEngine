#pragma once
#include "Prerequisites.h"

class
    Device;

class
    DeviceContext;

/**
 * @class Texture
 * @brief Representa una textura en DirectX 11.
 *
 * Esta clase encapsula la creación, gestión y destrucción de texturas 2D
 * en DirectX 11, así como su vinculación al pipeline gráfico.
 * Puede inicializarse desde archivo, como un recurso en memoria,
 * o copiando otra textura existente.
 */
class
    Texture {
public:
    /**
     * @brief Constructor por defecto.
     */
    Texture() = default;

    /**
     * @brief Destructor por defecto.
     */
    ~Texture() = default;

    /**
     * @brief Inicializa la textura desde un archivo de imagen.
     *
     * Carga una imagen desde disco y crea los recursos de GPU necesarios
     * (p. ej., @c ID3D11Texture2D y su @c ShaderResourceView).
     *
     * @param device      Referencia al dispositivo de DirectX.
     * @param textureName Nombre o ruta del archivo de la textura.
     * @param extensionType Tipo de extensión de la textura (por ejemplo, @c PNG, @c JPG, @c DDS).
     * @return @c S_OK si se cargó correctamente; un @c HRESULT de error en caso contrario.
     */
    HRESULT
        init(Device& device,
            const std::string& textureName,
            ExtensionType extensionType);

    /**
     * @brief Inicializa la textura como un recurso vacío en memoria (textura 2D).
     *
     * Crea una textura con las dimensiones y el formato indicados, opcionalmente
     * habilitando multisampling (MSAA) según @p sampleCount y @p qualityLevels.
     *
     * @param device         Referencia al dispositivo de DirectX.
     * @param width          Ancho de la textura en píxeles.
     * @param height         Alto de la textura en píxeles.
     * @param Format         Formato de la textura (valor de @c DXGI_FORMAT).
     * @param BindFlags      Banderas de enlace (p. ej., @c D3D11_BIND_RENDER_TARGET, @c D3D11_BIND_SHADER_RESOURCE).
     * @param sampleCount    Número de muestras para MSAA (por defecto 1 = sin MSAA).
     * @param qualityLevels  Niveles de calidad para MSAA (por defecto 0).
     * @return @c S_OK si se creó correctamente; un @c HRESULT de error en caso contrario.
     */
    HRESULT
        init(Device& device,
            unsigned int width,
            unsigned int height,
            DXGI_FORMAT Format,
            unsigned int BindFlags,
            unsigned int sampleCount = 1,
            unsigned int qualityLevels = 0);

    /**
     * @brief Inicializa la textura copiando la configuración de otra textura.
     *
     * Útil para crear una textura compatible (mismas dimensiones/MSAA)
     * a partir de @p textureRef, con un @p format concreto.
     *
     * @param device     Referencia al dispositivo de DirectX.
     * @param textureRef Textura de referencia para crear la nueva.
     * @param format     Formato de la textura resultante (@c DXGI_FORMAT).
     * @return @c S_OK si se creó correctamente; un @c HRESULT de error en caso contrario.
     */
    HRESULT
        init(Device& device, Texture& textureRef, DXGI_FORMAT format);

    /**
     * @brief Actualiza el estado de la textura.
     *
     * Método placeholder para lógica de actualización (si fuese necesaria).
     */
    void
        update();

    /**
     * @brief Enlaza la textura al pipeline gráfico para la etapa de pixel shader.
     *
     * Asigna @c m_textureFromImg en el slot indicado mediante
     * @c ID3D11DeviceContext::PSSetShaderResources.
     *
     * @param deviceContext Contexto del dispositivo de DirectX.
     * @param StartSlot     Índice de slot inicial donde se va a asignar la SRV.
     * @param NumViews      Número de vistas a asignar a partir de @p StartSlot.
     */
    void
        render(DeviceContext& deviceContext, unsigned int StartSlot, unsigned int NumViews);

    /**
     * @brief Libera los recursos asociados a la textura.
     *
     * Llama a @c Release() sobre @c m_texture y @c m_textureFromImg
     * y los pone a @c nullptr.
     */
    void
        destroy();


public:
    /**
     * @brief Puntero al recurso de textura 2D de DirectX 11 (@c ID3D11Texture2D).
     */
    ID3D11Texture2D* m_texture = nullptr;

    /**
     * @brief Vista de recurso de shader (@c ID3D11ShaderResourceView) creada a partir de la textura.
     */
    ID3D11ShaderResourceView* m_textureFromImg = nullptr;

    /**
     * @brief Nombre o ruta de la textura cargada desde archivo.
     */
    std::string m_textureName;
};
