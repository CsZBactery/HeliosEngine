#pragma once
#include "Prerequisites.h"

class
    Device;

class
    DeviceContext;

class
    Window;

class
    Texture;

/**
 * @class SwapChain
 * @brief Encapsula la funcionalidad de un *swap chain* en DirectX 11.
 *
 * El *swap chain* administra el intercambio de buffers entre el *back buffer* y
 * el *front buffer*, permitiendo presentar en pantalla los fotogramas renderizados.
 * También concentra la configuración de *multisampling* (MSAA) para mejorar la calidad visual.
 */
class
    SwapChain {
public:
    /**
     * @brief Constructor por defecto.
     */
    SwapChain() = default;

    /**
     * @brief Destructor por defecto.
     */
    ~SwapChain() = default;

    /**
     * @brief Inicializa el *swap chain* y obtiene el *back buffer*.
     *
     * Crea el dispositivo y el contexto D3D11 (si procede), configura MSAA,
     * crea el *swap chain* asociado a la ventana y recupera el *back buffer*
     * en @p backBuffer.
     *
     * @param device          Referencia al dispositivo de DirectX 11.
     * @param deviceContext   Referencia al contexto del dispositivo.
     * @param backBuffer      Textura asociada al *back buffer* que se rellenará.
     * @param window          Ventana donde se presentará el contenido.
     * @return HRESULT        @c S_OK si todo fue correcto; código de error en caso contrario.
     */
    HRESULT
        init(Device& device,
            DeviceContext& deviceContext,
            Texture& backBuffer,
            Window window);
    // multi aliasing mejora la calidad de píxeles

    /**
     * @brief Actualiza el estado del *swap chain*.
     *
     * Método *placeholder* para lógica de actualización relacionada con el *swap chain*.
     */
    void
        update();

    /**
     * @brief Ejecuta la fase de render previa a la presentación (si aplica).
     *
     * Método *placeholder* para lógica de render previa a @ref present().
     */
    void
        render();

    /**
     * @brief Libera los recursos asociados al *swap chain* y a las interfaces DXGI auxiliares.
     */
    void
        destroy();

    /**
     * @brief Presenta el contenido del *back buffer* en la ventana.
     *
     * Intercambia *front/back buffer* para mostrar en pantalla el último fotograma renderizado.
     * Internamente llama a @c IDXGISwapChain::Present.
     */
    void
        present();



public:
    /**
     * @brief Puntero al objeto @c IDXGISwapChain de DirectX 11.
     */
    IDXGISwapChain* m_swapChain = nullptr;

    /**
     * @brief Tipo de controlador utilizado (hardware, WARP, referencia, etc.).
     */
    D3D_DRIVER_TYPE m_driverType = D3D_DRIVER_TYPE_NULL;

private:
    /**
     * @brief Nivel de características de DirectX soportado por el dispositivo.
     */
    D3D_FEATURE_LEVEL m_featureLevel = D3D_FEATURE_LEVEL_11_0;

    /**
     * @brief Número de muestras de *multisampling* configuradas (MSAA).
     */
    unsigned int m_sampleCount;

    /**
     * @brief Niveles de calidad soportados para MSAA.
     */
    unsigned int m_qualityLevels;

    /**
     * @brief Interfaz @c IDXGIDevice asociada al dispositivo D3D11.
     */
    IDXGIDevice* m_dxgiDevice = nullptr;

    /**
     * @brief Adaptador de DXGI (tarjeta/gráfica) obtenido desde @c IDXGIDevice.
     */
    IDXGIAdapter* m_dxgiAdapter = nullptr;

    /**
     * @brief Fábrica de DXGI utilizada para crear el @c IDXGISwapChain.
     */
    IDXGIFactory* m_dxgiFactory = nullptr;
};
