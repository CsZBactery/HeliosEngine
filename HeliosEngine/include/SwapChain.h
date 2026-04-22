/**
 * @file SwapChain.h
 * @brief Gestión de la infraestructura DXGI y el intercambio de buffers de presentación.
 * @ingroup core
 */

#pragma once
#include "Prerequisites.h"

 // Forward Declarations
class Device;
class DeviceContext;
class Window;
class Texture;

/**
 * @class SwapChain
 * @brief Encapsula la cadena de intercambio (IDXGISwapChain) para la gestión del Double Buffering.
 * @details El Swap Chain es el componente crítico que conecta la GPU con la ventana de Windows.
 * Administra el ciclo de vida de dos recursos principales:
 * 1. **Front Buffer:** La imagen que el usuario está viendo actualmente en el monitor.
 * 2. **Back Buffer:** El lienzo oculto donde el motor realiza el renderizado del frame actual.
 *
 * Facilita el intercambio (Flip) de estos buffers para eliminar el parpadeo visual y el "tearing".
 * Además, gestiona la configuración de **MSAA (Multisample Anti-Aliasing)** para el suavizado de bordes.
 */
class SwapChain {
public:
    /** @brief Constructor por defecto. No reserva recursos COM. */
    SwapChain() = default;

    /** @brief Destructor por defecto. Se debe liberar manualmente con destroy(). */
    ~SwapChain() = default;

    /**
     * @brief Inicializa la infraestructura DXGI y vincula la cadena con la ventana.
     * @details Realiza las siguientes operaciones críticas:
     * 1. Localiza la fábrica DXGI (IDXGIFactory) para la creación de recursos.
     * 2. Configura el formato de píxel, la resolución y el muestreo MSAA.
     * 3. Crea la conexión física (SwapChain) con el manejador de ventana (HWND).
     *
     * @param device Referencia al dispositivo físico (GPU).
     * @param deviceContext Contexto para la ejecución de comandos.
     * @param backBuffer Textura donde se recibirá el recurso del búfer de dibujo.
     * @param window Instancia de la ventana de aplicación.
     * @return S_OK si la creación fue exitosa; código HRESULT en caso de error.
     * @post Si retorna S_OK, m_swapChain != nullptr.
     */
    HRESULT init(Device& device, DeviceContext& deviceContext, Texture& backBuffer, Window window);

    /**
     * @brief Actualiza parámetros internos de la cadena.
     * @note Placeholder para soportar cambios dinámicos de configuración en caliente.
     */
    void update();

    /**
     * @brief Operaciones de renderizado previas a la presentación.
     * @note Placeholder para sincronización de buffers si fuera necesario.
     */
    void render();

    /**
     * @brief Libera todas las interfaces DXGI y el objeto SwapChain de la memoria.
     * @details Limpia de forma segura m_swapChain, m_dxgiDevice, m_dxgiAdapter y m_dxgiFactory.
     * @post m_swapChain == nullptr.
     */
    void destroy();

    /**
     * @brief Presenta el Back Buffer en la pantalla (Flip).
     * @details Envía el contenido renderizado al monitor. Si se implementa V-Sync,
     * este método sincroniza la presentación con la frecuencia de refresco vertical.
     */
    void present();

    /**
     * @brief Ajusta el tamaño de los buffers internos al cambiar el tamaño de la ventana.
     * @details Esencial para mantener la fidelidad visual tras un evento de Resize.
     * @param width Nuevo ancho en píxeles del área cliente.
     * @param height Nuevo alto en píxeles del área cliente.
     * @return S_OK si los buffers se redimensionaron correctamente.
     */
    HRESULT resizeBuffers(unsigned int width, unsigned int height);

    /**
     * @brief Recupera la textura del Back Buffer desde la cadena de intercambio.
     * @param backBuffer Referencia al objeto Texture que recibirá el recurso subyacente.
     * @return S_OK si se obtuvo el acceso al buffer.
     */
    HRESULT getBackBuffer(Texture& backBuffer);

public:
    /** @brief Objeto principal del Swap Chain en Direct3D 11. */
    IDXGISwapChain* m_swapChain = nullptr;

    /** @brief Especifica el tipo de controlador utilizado (Hardware, Software, Reference). */
    D3D_DRIVER_TYPE m_driverType = D3D_DRIVER_TYPE_NULL;

private:
    /** @brief Nivel de características de hardware soportado por el dispositivo. */
    D3D_FEATURE_LEVEL m_featureLevel = D3D_FEATURE_LEVEL_11_0;

    /** @brief Cantidad de muestras por píxel para Anti-Aliasing (ej: 4 = 4x MSAA). */
    unsigned int m_sampleCount;

    /** @brief Niveles de calidad técnica soportados para el suavizado de bordes. */
    unsigned int m_qualityLevels;

    /** @brief Interfaz DXGI para la comunicación con el dispositivo gráfico. */
    IDXGIDevice* m_dxgiDevice = nullptr;

    /** @brief Interfaz que representa el adaptador físico (la tarjeta de video). */
    IDXGIAdapter* m_dxgiAdapter = nullptr;

    /** @brief Fábrica DXGI responsable de la creación del SwapChain. */
    IDXGIFactory* m_dxgiFactory = nullptr;
};