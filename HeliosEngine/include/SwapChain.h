#pragma once
#include "Prerequisites.h"

/**
 * @file SwapChain.h
 * @brief Gestión de la infraestructura DXGI y el intercambio de buffers.
 */

class Device;
class DeviceContext;
class Window;
class Texture;

/**
 * @class SwapChain
 * @brief Encapsula la cadena de intercambio (IDXGISwapChain) para la gestión del Double Buffering.
 * @details El Swap Chain es el puente crítico entre la GPU y la ventana de Windows. Administra:
 * 1. El Front Buffer: Imagen visible actualmente.
 * 2. El Back Buffer: Imagen donde se realiza el renderizado actual.
 * Facilita el intercambio de estos buffers para evitar el parpadeo visual (tearing).
 */
class SwapChain {
public:
    /** @brief Constructor por defecto. */
    SwapChain() = default;

    /** @brief Destructor por defecto. Libera mediante destroy(). */
    ~SwapChain() = default;

    /**
     * @brief Inicializa la infraestructura DXGI y vincula la cadena con la ventana.
     * @details
     * 1. Localiza la fábrica DXGI (Factory).
     * 2. Configura el formato de píxel y el muestreo MSAA.
     * 3. Crea la conexión física con el HWND de la ventana proporcionada.
     * * @param device Referencia al dispositivo físico (GPU).
     * @param deviceContext Contexto para la ejecución de comandos.
     * @param backBuffer Textura donde se almacenará el búfer de dibujo.
     * @param window Instancia de la ventana de aplicación.
     * @return HRESULT S_OK si la operación fue exitosa.
     */
    HRESULT init(Device& device, DeviceContext& deviceContext, Texture& backBuffer, Window window);

    /** @brief Actualización lógica de la cadena (Placeholder). */
    void update();

    /** @brief Operaciones de renderizado previas a la presentación. */
    void render();

    /** @brief Libera todas las interfaces DXGI y el SwapChain. */
    void destroy();

    /**
     * @brief Presenta el Back Buffer en la pantalla (Flip).
     * @details Envía el contenido renderizado al monitor y sincroniza con el refresco vertical (V-Sync).
     */
    void present();

    /**
     * @brief Ajusta el tamaño de los buffers internos al cambiar el tamaño de la ventana.
     * @param width Nuevo ancho en píxeles.
     * @param height Nuevo alto en píxeles.
     */
    HRESULT resizeBuffers(unsigned int width, unsigned int height);

    /**
     * @brief Recupera la textura del Back Buffer desde la cadena de intercambio.
     * @param backBuffer Referencia a la textura que recibirá el recurso.
     */
    HRESULT getBackBuffer(Texture& backBuffer);

public:
    /** @brief Puntero a la interfaz nativa de DirectX Graphics Infrastructure. */
    IDXGISwapChain* m_swapChain = nullptr;

    /** @brief Especifica el tipo de controlador utilizado por D3D11. */
    D3D_DRIVER_TYPE m_driverType = D3D_DRIVER_TYPE_NULL;

private:
    /** @brief Máximo nivel de hardware soportado por la tarjeta de video. */
    D3D_FEATURE_LEVEL m_featureLevel = D3D_FEATURE_LEVEL_11_0;

    /** @brief Cantidad de muestras por píxel para Anti-Aliasing (Multi-Sampling). */
    unsigned int m_sampleCount;

    /** @brief Niveles de calidad técnica para el suavizado de bordes. */
    unsigned int m_qualityLevels;

    /** @brief Interfaz DXGI para la comunicación con el dispositivo. */
    IDXGIDevice* m_dxgiDevice = nullptr;

    /** @brief Interfaz que representa el adaptador físico (GPU). */
    IDXGIAdapter* m_dxgiAdapter = nullptr;

    /** @brief Fábrica responsable de generar el SwapChain. */
    IDXGIFactory* m_dxgiFactory = nullptr;
};