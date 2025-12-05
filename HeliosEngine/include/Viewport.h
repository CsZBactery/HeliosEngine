#pragma once
#include "Prerequisites.h"

class Window;
class DeviceContext;

/**
 * @class Viewport
 * @brief Define el área rectangular de la ventana donde se renderizará la escena.
 *
 * El Viewport es el paso final de la transformación geométrica. Convierte las coordenadas
 * normalizadas (NDC) que salen del Vertex Shader (x: -1 a 1, y: -1 a 1) en coordenadas
 * reales de píxeles de pantalla (ej: x: 0 a 1920, y: 0 a 1080).
 *
 * También controla el rango de profundidad (MinDepth/MaxDepth).
 */
class Viewport {
public:

    /**
     * @brief Constructor por defecto.
     */
    Viewport() = default;

    /**
     * @brief Destructor por defecto.
     */
    ~Viewport() = default;

    /**
     * @brief Inicializa el Viewport basándose en el tamaño de una ventana.
     *
     * Configura el viewport para que ocupe toda el área cliente de la ventana dada.
     *
     * @param window Referencia a la ventana de la aplicación.
     * @return HRESULT S_OK si la configuración fue correcta.
     */
    HRESULT
        init(const Window& window);

    /**
     * @brief Inicializa el Viewport con dimensiones personalizadas.
     *
     * Útil para "Render to Texture" (donde el tamaño de la textura es distinto al de la pantalla),
     * mapas de sombras, o para crear efectos de pantalla dividida (split-screen).
     *
     * @param width Ancho del viewport en píxeles.
     * @param height Alto del viewport en píxeles.
     * @return HRESULT S_OK si la configuración fue correcta.
     */
    HRESULT
        init(unsigned int width, unsigned int height);

    /**
     * @brief Actualiza la lógica del viewport.
     *
     * Generalmente vacío, a menos que el viewport cambie dinámicamente sin redimensionar la ventana.
     */
    void
        update();

    /**
     * @brief Activa el Viewport en el Pipeline (Etapa del Rasterizador).
     *
     * Llama internamente a `RSSetViewports`. Sin esto, la GPU no sabe en qué
     * parte del Render Target debe dibujar los píxeles.
     *
     * @param deviceContext Contexto donde se realizará el dibujo.
     */
    void
        render(DeviceContext& deviceContext);

    /**
     * @brief Limpia recursos.
     *
     * @note D3D11_VIEWPORT es una estructura simple (struct), no un puntero COM,
     * por lo que no necesita llamar a Release().
     */
    void
        destroy() {}

public:
    /**
     * @brief Estructura nativa de DirectX que define las dimensiones y profundidad.
     *
     * Contiene TopLeftX, TopLeftY, Width, Height, MinDepth y MaxDepth.
     */
    D3D11_VIEWPORT m_viewport;
};