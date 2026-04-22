/**
 * @file Viewport.h
 * @brief Declara la API de Viewport dentro del subsistema Core.
 * @ingroup core
 */

#pragma once
#include "Prerequisites.h"

class Window;
class DeviceContext;

/**
 * @class Viewport
 * @brief Define el área rectangular de la ventana donde se renderizará la escena.
 * @details El Viewport es el paso final de la transformación geométrica en Direct3D 11.
 * Su función principal es convertir las coordenadas normalizadas (NDC) que salen del
 * Vertex Shader (rango -1 a 1) en coordenadas de píxeles reales de la pantalla
 * (ej: 0 a 1920).
 * * También controla el rango de profundidad (MinDepth/MaxDepth) para el Z-Buffer.
 */
class Viewport {
public:
    /** @brief Constructor por defecto. */
    Viewport() = default;

    /** @brief Destructor por defecto. */
    ~Viewport() = default;

    /**
     * @brief Inicializa el Viewport basándose en el tamaño de una ventana.
     * @details Configura el viewport para que ocupe toda el área cliente de la ventana dada.
     * @param window Referencia a la ventana de la aplicación.
     * @return S_OK si la configuración fue correcta.
     * @post El miembro m_viewport contendrá las dimensiones actuales de la ventana.
     */
    HRESULT init(const Window& window);

    /**
     * @brief Inicializa el Viewport con dimensiones personalizadas.
     * @details Útil para "Render to Texture", mapas de sombras (Shadow Mapping),
     * o para crear efectos de pantalla dividida (split-screen).
     * @param width Ancho del viewport en píxeles.
     * @param height Alto del viewport en píxeles.
     * @return S_OK si la inicialización fue exitosa.
     */
    HRESULT init(unsigned int width, unsigned int height);

    /** * @brief Actualiza la lógica del viewport.
     * @note Actualmente es un placeholder; útil si el viewport cambiara dinámicamente
     * sin que la ventana cambie de tamaño.
     */
    void update();

    /**
     * @brief Activa el Viewport en el Pipeline (Etapa del Rasterizador).
     * @details Llama internamente a @c RSSetViewports. Sin esta llamada, la GPU no sabe
     * en qué parte del Render Target debe dibujar los píxeles procesados.
     * @param deviceContext Contexto donde se emitirá la orden a la GPU.
     * @pre El viewport debe haber sido inicializado con init().
     */
    void render(DeviceContext& deviceContext);

    /**
     * @brief Libera recursos asociados al viewport.
     * @note D3D11_VIEWPORT es una estructura simple (POD), no un recurso COM,
     * por lo que no requiere liberación de memoria dinámica.
     */
    void destroy() {}

public:
    /**
     * @brief Estructura nativa de Direct3D que define el viewport.
     * @details Contiene TopLeftX, TopLeftY, Width, Height, MinDepth y MaxDepth.
     */
    D3D11_VIEWPORT m_viewport;
};