#pragma once
#include "Prerequisites.h"

class Device;
class DeviceContext;
class Window;
class Texture;

/**
 * @class SwapChain
 * @brief Gestiona la Cadena de Intercambio (Swap Chain) e infraestructura DXGI.
 *
 * El Swap Chain es el mecanismo que conecta la memoria de la GPU con la ventana de Windows.
 * Implementa el patrón de **Doble Búfer (Double Buffering)**:
 * 1. **Front Buffer:** La imagen que el monitor está mostrando actualmente.
 * 2. **Back Buffer:** La imagen que la GPU está dibujando en este momento.
 *
 * Cuando el dibujo termina, la función present() intercambia los punteros,
 * mostrando el nuevo frame instantáneamente y evitando el parpadeo (tearing).
 */
class SwapChain {
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
     * @brief Inicializa la Cadena de Intercambio y vincula DirectX con la Ventana.
     *
     * Este método es crucial porque:
     * 1. Obtiene la "Fábrica" (DXGI Factory) para crear recursos de bajo nivel.
     * 2. Configura el formato de píxel y el muestreo (Anti-Aliasing/MSAA).
     * 3. Crea la conexión física con el HWND de la ventana.
     * 4. Obtiene la textura del Back Buffer para que podamos dibujar en ella.
     *
     * @param device Referencia al dispositivo (GPU).
     * @param deviceContext Contexto del dispositivo.
     * @param backBuffer Referencia a la textura donde almacenaremos el Back Buffer.
     * @param window Objeto ventana donde se mostrará el resultado.
     * @return HRESULT S_OK si la cadena se creó correctamente.
     */
    HRESULT
        init(Device& device,
            DeviceContext& deviceContext,
            Texture& backBuffer,
            Window window);

    /**
     * @brief Actualiza la lógica interna (Placeholder).
     */
    void
        update();

    /**
     * @brief Renderiza operaciones previas a la presentación (si las hubiera).
     */
    void
        render();

    /**
     * @brief Libera los recursos DXGI y la cadena de intercambio.
     */
    void
        destroy();

    /**
     * @brief Realiza el intercambio de buffers (Flip / Present).
     *
     * Promueve el Back Buffer a Front Buffer para que sea visible en el monitor.
     * Aquí es donde se suele controlar la Sincronización Vertical (V-Sync).
     */
    void
        present();

public:
    /**
     * @brief Puntero a la interfaz nativa IDXGISwapChain.
     */
    IDXGISwapChain* m_swapChain = nullptr;

    /**
     * @brief Tipo de driver usado (Hardware, Software/WARP, o Referencia).
     */
    D3D_DRIVER_TYPE m_driverType = D3D_DRIVER_TYPE_NULL;

private:
    /**
     * @brief Nivel de características (Feature Level) soportado por la GPU (ej: DX11.0).
     */
    D3D_FEATURE_LEVEL m_featureLevel = D3D_FEATURE_LEVEL_11_0;

    /**
     * @brief Número de muestras para Anti-Aliasing (MSAA). 1 = Desactivado.
     */
    unsigned int m_sampleCount;

    /**
     * @brief Niveles de calidad de imagen soportados por la GPU para el MSAA.
     */
    unsigned int m_qualityLevels;

    // ------------------------------------------------------------------------
    // PUNTEROS DXGI (INFRAESTRUCTURA DE HARDWARE)
    // ------------------------------------------------------------------------

    /** @brief Interfaz al dispositivo gráfico físico (Tarjeta Gráfica). */
    IDXGIDevice* m_dxgiDevice = nullptr;

    /** @brief Interfaz al adaptador de pantalla (Drivers/Hardware). */
    IDXGIAdapter* m_dxgiAdapter = nullptr;

    /** @brief Interfaz a la fábrica DXGI (Generador de cadenas de intercambio). */
    IDXGIFactory* m_dxgiFactory = nullptr;
};