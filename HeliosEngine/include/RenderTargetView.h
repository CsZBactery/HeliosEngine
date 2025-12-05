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
 *
 * Esta clase administra la creación, uso y destrucción de un ID3D11RenderTargetView.
 * Un RTV es la interfaz que permite al Pipeline Gráfico escribir el resultado
 * de los píxeles (colores) en una textura o en el Back Buffer de la pantalla.
 */
class RenderTargetView {
public:

    /**
     * @brief Constructor por defecto.
     */
    RenderTargetView() = default;

    /**
     * @brief Destructor por defecto.
     */
    ~RenderTargetView() = default;

    /**
     * @brief Inicializa el RTV apuntando al Back Buffer (Pantalla principal).
     *
     * Se utiliza normalmente durante la inicialización de la SwapChain para
     * indicar que queremos dibujar directamente en la ventana.
     *
     * @param device Referencia al dispositivo para crear el recurso.
     * @param backBuffer La textura del Back Buffer obtenida de la SwapChain.
     * @param format Formato de color (ej: DXGI_FORMAT_R8G8B8A8_UNORM).
     * @return HRESULT S_OK si se inicializó correctamente.
     */
    HRESULT
        init(Device& device, Texture& backBuffer, DXGI_FORMAT format);

    /**
     * @brief Inicializa el RTV para una Textura personalizada (Render to Texture).
     *
     * Útil para efectos de post-procesado, mapas de sombras, o espejos, donde
     * renderizamos la escena en una textura aparte en lugar de la pantalla.
     *
     * @param device Referencia al dispositivo.
     * @param inTex La textura donde guardaremos el renderizado.
     * @param viewDimension Tipo de vista (D3D11_RTV_DIMENSION_TEXTURE2D, etc.).
     * @param format Formato de los datos.
     * @return HRESULT S_OK si se inicializó correctamente.
     */
    HRESULT
        init(Device& device,
            Texture& inTex,
            D3D11_RTV_DIMENSION viewDimension,
            DXGI_FORMAT format);

    /**
     * @brief Actualiza lógica interna (Placeholder).
     */
    void
        update();

    /**
     * @brief Limpia el objetivo y lo asigna al Pipeline (Fase Output Merger).
     *
     * Prepara este RTV y el DepthStencilView para recibir el dibujo del frame actual.
     * También limpia la pantalla con el color de fondo especificado.
     *
     * @param deviceContext Contexto para ejecutar comandos.
     * @param depthStencilView Vista de profundidad para el test Z-Buffer.
     * @param numViews Número de render targets simultáneos (usualmente 1).
     * @param clearColor Array [R,G,B,A] con el color de limpieza (fondo).
     */
    void
        render(DeviceContext& deviceContext,
            DepthStencilView& depthStencilView,
            unsigned int numViews,
            const float clearColor[4]);

    /**
     * @brief Asigna el RTV al Pipeline SIN limpiar y SIN DepthStencil.
     *
     * Útil para dibujo 2D (UI) o pases de composición donde no se necesita
     * borrar lo anterior ni verificar profundidad.
     *
     * @param deviceContext Contexto para ejecutar comandos.
     * @param numViews Número de vistas.
     */
    void
        render(DeviceContext& deviceContext,
            unsigned int numViews);

    /**
     * @brief Libera el recurso COM de DirectX.
     */
    void
        destroy();

private:
    /**
     * @brief Puntero nativo al recurso RTV de DirectX 11.
     */
    ID3D11RenderTargetView* m_renderTargetView = nullptr;
};