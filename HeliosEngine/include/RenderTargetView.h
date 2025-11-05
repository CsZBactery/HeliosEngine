#pragma once
#include "Prerequisites.h"

class Device;
class DeviceContext;
class Texture;
class DepthStencilView;

/**
 * @file RenderTargetView.h
 * @brief Declaración de la clase que encapsula un @c ID3D11RenderTargetView.
 * @details Proporciona una interfaz ligera para crear, enlazar, limpiar y liberar
 *          vistas de render (RTV) en Direct3D 11, incluyendo soporte para
 *          back-buffers de swap chain y texturas personalizadas (con o sin MSAA).
 */

 /**
  * @class RenderTargetView
  * @brief Encapsula un @c ID3D11RenderTargetView.
  * @details Gestiona el ciclo de vida de una RTV y su uso en el pipeline:
  *          inicialización desde un back-buffer o una textura 2D, limpieza
  *          con color, enlace junto con un DepthStencilView y liberación segura.
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
     * @brief Inicializa la RTV desde el back-buffer de la swap chain.
     * @param device    Dispositivo D3D11 que crea la vista.
     * @param backBuffer Textura del back-buffer obtenida de la swap chain.
     * @param format    Formato DXGI del render target (ej. @c DXGI_FORMAT_R8G8B8A8_UNORM).
     * @return @c S_OK en éxito; código de error @c HRESULT en caso contrario.
     * @note La implementación detecta automáticamente si el back-buffer es MSAA
     *       y selecciona la dimensión de vista adecuada (@c TEXTURE2D o @c TEXTURE2DMS).
     */
    HRESULT init(Device& device, Texture& backBuffer, DXGI_FORMAT format);

    /**
     * @brief Inicializa la RTV desde una textura arbitraria.
     * @param device        Dispositivo D3D11 que crea la vista.
     * @param inTex         Textura fuente (p.ej. color target off-screen).
     * @param viewDimension Dimensión de la vista (@c D3D11_RTV_DIMENSION_*).
     * @param format        Formato DXGI del render target.
     * @return @c S_OK en éxito; código de error @c HRESULT en caso contrario.
     * @warning @p viewDimension debe ser compatible con la descripción de @p inTex
     *          (mips, arrays, MSAA).
     */
    HRESULT init(Device& device, Texture& inTex, D3D11_RTV_DIMENSION viewDimension, DXGI_FORMAT format);

    /**
     * @brief Actualiza el estado interno de la RTV.
     * @note Método de marcador; actualmente no realiza operaciones.
     */
    void    update();

    /**
     * @brief Enlaza la RTV y un DepthStencilView y limpia el color.
     * @param deviceContext     Contexto de dispositivo para emitir los comandos.
     * @param depthStencilView  Vista de profundidad/esténcil asociada.
     * @param numViews          Número de vistas de color a enlazar (normalmente 1).
     * @param clearColor        Color RGBA para limpiar el render target.
     * @post El render target queda como destino activo en el OM y con color limpio.
     */
    void    render(DeviceContext& deviceContext, DepthStencilView& depthStencilView, unsigned int numViews, const float clearColor[4]);

    /**
     * @brief Enlaza la RTV como destino de render sin limpiar color ni DSV.
     * @param deviceContext Contexto de dispositivo para emitir los comandos.
     * @param numViews      Número de vistas de color a enlazar (normalmente 1).
     * @note Útil cuando se quiere preservar el contenido existente del target.
     */
    void    render(DeviceContext& deviceContext, unsigned int numViews);

    /**
     * @brief Libera de forma segura la vista de render asociada.
     * @post @c m_renderTargetView se establece a @c nullptr.
     */
    void    destroy();

private:
    /** Puntero COM a la vista de render de Direct3D 11. */
    ID3D11RenderTargetView* m_renderTargetView = nullptr;
};
