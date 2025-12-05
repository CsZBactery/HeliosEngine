#pragma once
#include "Prerequisites.h"

class Device;
class DeviceContext;

/**
 * @class Texture
 * @brief Gestiona recursos de Textura 2D y sus Vistas (SRV) en DirectX 11.
 *
 * Esta clase encapsula dos objetos fundamentales de DX11:
 * 1. **ID3D11Texture2D:** El recurso de memoria cruda que contiene los datos de la imagen.
 * 2. **ID3D11ShaderResourceView (SRV):** La "lente" a través de la cual el Shader puede leer esa textura.
 *
 * Puede inicializarse de tres formas: cargando un archivo (PNG/JPG), creando una textura vacía
 * (para Render Targets), o copiando otra textura.
 */
class Texture {
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
     * @brief Carga una textura desde un archivo de imagen en disco.
     *
     * Utiliza librerías de carga (como STB Image o DirectXTK) para leer archivos
     * PNG, JPG, DDS, etc., y subirlos a la memoria de la GPU.
     *
     * @param device Referencia al dispositivo (Factory) para crear el recurso.
     * @param textureName Ruta relativa o absoluta del archivo (ej: "Assets/Textures/Muro.png").
     * @param extensionType Tipo de archivo para ayudar al cargador (ej: PNG, DDS).
     * @return HRESULT S_OK si el archivo se encontró y cargó correctamente.
     */
    HRESULT
        init(Device& device,
            const std::string& textureName,
            ExtensionType extensionType);

    /**
     * @brief Crea una textura vacía con parámetros específicos (Manual).
     *
     * Este método es fundamental para crear:
     * - El Back Buffer (cuando resize).
     * - Texturas para Render Target (Renderizar a textura).
     * - Mapas de sombras (Depth Buffers).
     *
     * @param device Referencia al dispositivo.
     * @param width Ancho en píxeles.
     * @param height Alto en píxeles.
     * @param Format Formato de los píxeles (ej: DXGI_FORMAT_R8G8B8A8_UNORM).
     * @param BindFlags Banderas que indican cómo se usará (D3D11_BIND_SHADER_RESOURCE, BIND_RENDER_TARGET, etc.).
     * @param sampleCount Muestras MSAA (1 = desactivado).
     * @param qualityLevels Calidad MSAA.
     * @return HRESULT S_OK si la reserva de memoria fue exitosa.
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
     * @brief Inicializa esta textura tomando posesión de un recurso existente.
     *
     * Útil para inicializar la clase Texture a partir del puntero nativo del BackBuffer
     * que nos entrega la SwapChain.
     *
     * @param device Referencia al dispositivo.
     * @param textureRef Objeto textura del cual copiaremos o referenciaremos datos.
     * @param format Formato para crear la vista (SRV).
     * @return HRESULT S_OK si la operación fue exitosa.
     */
    HRESULT
        init(Device& device, Texture& textureRef, DXGI_FORMAT format);

    /**
     * @brief Actualiza la lógica de la textura (si aplica).
     *
     * @note Generalmente las texturas estáticas no requieren update por frame.
     */
    void
        update();

    /**
     * @brief Enlaza la textura al Pipeline Gráfico para que el Shader la lea.
     *
     * Llama internamente a `PSSetShaderResources`. Esto hace que la textura esté
     * disponible en los registros t0, t1, etc. del Pixel Shader.
     *
     * @param deviceContext Contexto de renderizado.
     * @param StartSlot Ranura (Slot) donde se enlazará (0 para t0, 1 para t1...).
     * @param NumViews Número de vistas a enlazar (generalmente 1).
     */
    void
        render(DeviceContext& deviceContext, unsigned int StartSlot, unsigned int NumViews);

    /**
     * @brief Libera la memoria de la textura y su vista (SRV).
     */
    void
        destroy();


public:
    /**
     * @brief Puntero al recurso de textura 2D nativo de DirectX 11.
     *
     * Contiene los datos brutos de la imagen en VRAM.
     */
    ID3D11Texture2D* m_texture = nullptr;

    /**
     * @brief Vista de Recurso de Shader (SRV).
     *
     * Es la interfaz que permite a los Shaders leer la textura. Sin esto,
     * la textura existe en memoria pero es invisible para el Pixel Shader.
     */
    ID3D11ShaderResourceView* m_textureFromImg = nullptr;

    /**
     * @brief Nombre del archivo o identificador de depuración.
     */
    std::string m_textureName;
};