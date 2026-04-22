/**
 * @file RenderTargetView.h
 * @brief Declara la API de RenderTargetView dentro del subsistema Core.
 * @ingroup core
 */

#pragma once
#include "Prerequisites.h"

 // Forward Declarations
class Device;
class DeviceContext;
class Texture;
class DepthStencilView;

/**
 * @class RenderTargetView
 * @brief Encapsula una Vista de Destino de Renderizado (RTV) de DirectX 11.
 * @details Esta clase administra la creación, uso y destrucción de un ID3D11RenderTargetView.
 * Un RTV es la interfaz que permite al Pipeline Gráfico escribir el resultado
 * de los píxeles (colores) en una textura o en el Back Buffer de la pantalla.
 *
 * Es un componente crítico para el Deferred Rendering, permitiendo la creación de G-Buffers.
 */
class RenderTargetView {
public:
    /** @brief Constructor por defecto. No reserva recursos en GPU. */
    RenderTargetView() = default;

    /** @brief Destructor por defecto. Se debe llamar a destroy() para liberar recursos COM. */
    ~RenderTargetView() = default;

    /**
     * @brief Inicializa el RTV apuntando al Back Buffer (Pantalla principal).
     * @details Se utiliza normalmente durante la inicialización de la SwapChain para
     * indicar que queremos dibujar directamente en la ventana.
     *
     * @param device Referencia al dispositivo para crear el recurso.
     * @param backBuffer La textura del Back Buffer obtenida de la SwapChain.
     * @param Format Formato de color (ej: DXGI_FORMAT_R8G8B8A8_UNORM).
     * @return S_OK si se inicializó correctamente.
     * @post Si retorna S_OK, m_renderTargetView != nullptr.
     */
    HRESULT init(Device& device, Texture& backBuffer, DXGI_FORMAT Format);

    /**
     * @brief Inicializa el RTV para una Textura personalizada (Render to Texture).
     * @details Útil para G-Buffers, efectos de post-procesado, mapas de sombras o espejos,
     * donde renderizamos la escena en una textura aparte en lugar de la pantalla.
     *
     * @param device Referencia al dispositivo.
     * @param inTex La textura donde guardaremos el renderizado.
     * @param ViewDimension Tipo de vista (D3D11_RTV_DIMENSION_TEXTURE2D, etc.).
     * @param Format Formato de los datos.
     * @return S_OK si se inicializó correctamente.
     */
    HRESULT init(Device& device,
        Texture& inTex,
        D3D11_RTV_DIMENSION ViewDimension,
        DXGI_FORMAT Format);

    /** @brief Actualiza parámetros internos del RTV (Placeholder para extensiones futuras). */
    void update();

    /**
     * @brief Limpia el objetivo y lo asigna al Pipeline (Fase Output Merger).
     * @details Prepara este RTV y el DepthStencilView para recibir el dibujo del frame actual.
     * También limpia la pantalla con el color de fondo especificado.
     *
     * @param deviceContext Contexto para ejecutar comandos.
     * @param depthStencilView Vista de profundidad para el test Z-Buffer.
     * @param numViews Número de render targets simultáneos (usualmente 1).
     * @param ClearColor Array [R,G,B,A] con el color de limpieza (fondo).
     * @pre m_renderTargetView debe estar creado con init().
     */
    void render(DeviceContext& deviceContext,
        DepthStencilView& depthStencilView,
        unsigned int numViews,
        const float ClearColor[4]);

    /**
     * @brief Asigna el RTV al Pipeline SIN limpiar y SIN DepthStencil.
     * @details Útil para dibujo 2D (UI) o pases de composición donde no se necesita
     * borrar lo anterior ni verificar profundidad.
     *
     * @param deviceContext Contexto para ejecutar comandos.
     * @param numViews Número de vistas de render (típicamente 1).
     * @pre m_renderTargetView debe estar creado con init().
     */
    void render(DeviceContext& deviceContext,
        unsigned int numViews);

    /**
     * @brief Libera el recurso COM ID3D11RenderTargetView de forma segura.
     * @details Método idempotente. Reinicia el puntero interno a nullptr.
     * @post m_renderTargetView == nullptr.
     */
    void destroy();

private:
    /**
     * @brief Recurso COM de Direct3D 11 para la vista de Render Target.
     * @details Válido tras init(); nullptr después de destroy().
     */
    ID3D11RenderTargetView* m_renderTargetView = nullptr;
};