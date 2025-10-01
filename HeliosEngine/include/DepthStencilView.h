/**
 * @file DepthStencilView.h
 * @brief Wrapper minimal para crear y gestionar un Depth Stencil View (DSV) en Direct3D 11.
 *
 * Proporciona utilidades para:
 * - Crear la textura de profundidad y su DSV asociado (sin MSAA).
 * - Asegurar/obtener un estado de depth-stencil común (lectura/escritura).
 * - Hacer bind del DSV junto con un RTV externo al Output-Merger (OM).
 *
 * @note Este wrapper usa punteros COM crudos (sin smart pointers). La función
 *       destroy() libera internamente los recursos; el destructor la invoca automáticamente.
 * @warning No es thread-safe.
 */

#pragma once
#include <d3d11.h>

 /**
  * @class DepthStencilView
  * @brief Encapsula una textura de profundidad, su vista (DSV) y un estado de depth-stencil opcional.
  *
  * @details
  * - @ref create crea (o recrea) la textura de profundidad y su @c ID3D11DepthStencilView.
  * - @ref ensureDepthState crea en lazy un @c ID3D11DepthStencilState con o sin escritura de profundidad.
  * - @ref bind hace OMSetRenderTargets(1, &rtv, dsv).
  *
  * @par Ciclo de vida
  * - Llama a @ref destroy explícitamente cuando ya no lo uses, o confía en el destructor.
  * - Repite @ref create si cambia el tamaño del backbuffer (por ejemplo en WM_SIZE).
  *
  * @par Limitaciones
  * - No cubre MSAA. Si necesitas MSAA, crea las texturas/DSV con SampleDesc.Count > 1.
  */
class DepthStencilView {
public:
  /// Constructor por defecto (no adquiere recursos).
  DepthStencilView() = default;

  /**
   * @brief Destructor: libera los recursos COM (si existen) llamando a @ref destroy.
   */
  ~DepthStencilView() { destroy(); }

  /// No copiable (posee recursos COM).
  DepthStencilView(const DepthStencilView&) = delete;
  DepthStencilView& operator=(const DepthStencilView&) = delete;

  /**
   * @brief Libera @c m_tex, @c m_dsv y @c m_state si están creados.
   *
   * @post Después de llamar, todos los punteros internos quedan en @c nullptr.
   */
  void destroy();

  /**
   * @brief Crea la textura de profundidad y su @c ID3D11DepthStencilView (sin MSAA).
   *
   * @param dev       Dispositivo de D3D11 válido.
   * @param width     Ancho en píxeles.
   * @param height    Alto en píxeles.
   * @param fmt       Formato del depth-stencil. Por defecto @c DXGI_FORMAT_D24_UNORM_S8_UINT.
   *
   * @return @c S_OK en éxito; HRESULT de error en caso contrario.
   *
   * @note Si ya existían recursos, se liberan y se vuelven a crear con el nuevo tamaño/formato.
   * @warning No crea estados de depth-stencil; usa @ref ensureDepthState si lo necesitas.
   */
  HRESULT create(ID3D11Device* dev, UINT width, UINT height,
    DXGI_FORMAT fmt = DXGI_FORMAT_D24_UNORM_S8_UINT);

  /**
   * @brief Asegura un @c ID3D11DepthStencilState interno (lazy) con o sin escritura de Z.
   *
   * @param dev   Dispositivo de D3D11 válido.
   * @param write Si es @c true, habilita escritura de profundidad; si es @c false, la deshabilita
   *              (útil para dibujar transparencias manteniendo pruebas de Z).
   *
   * @return @c S_OK en éxito; HRESULT de error en caso contrario.
   *
   * @note No hace @c OMSetDepthStencilState; solo garantiza que @ref state() no sea @c nullptr.
   */
  HRESULT ensureDepthState(ID3D11Device* dev, bool write = true);

  /**
   * @brief Realiza el bind en el Output-Merger: RTV externo + DSV interno.
   *
   * @param ctx Contexto inmediato de D3D11.
   * @param rtv Render Target View externo (no se posee ni se libera aquí).
   *
   * @pre @ref dsv() debe ser válido (haber llamado a @ref create).
   * @note Si también necesitas estado de depth-stencil, llama primero a @ref ensureDepthState
   *       y luego @c OMSetDepthStencilState(state(), stencilRef) manualmente.
   */
  void bind(ID3D11DeviceContext* ctx, ID3D11RenderTargetView* rtv);

  /**
   * @brief Acceso directo a la vista de profundidad.
   * @return Puntero a @c ID3D11DepthStencilView o @c nullptr si no existe.
   */
  ID3D11DepthStencilView* dsv()  const { return m_dsv; }

  /**
   * @brief Acceso directo a la textura subyacente.
   * @return Puntero a @c ID3D11Texture2D o @c nullptr si no existe.
   */
  ID3D11Texture2D* tex()  const { return m_tex; }

  /**
   * @brief Acceso al estado de depth-stencil (si fue creado con @ref ensureDepthState).
   * @return Puntero a @c ID3D11DepthStencilState o @c nullptr si no existe.
   */
  ID3D11DepthStencilState* state() const { return m_state; }

private:
  ID3D11Texture2D* m_tex = nullptr; ///< Textura de profundidad (propiedad de la clase).
  ID3D11DepthStencilView* m_dsv = nullptr; ///< Vista de profundidad para OM (propiedad de la clase).
  ID3D11DepthStencilState* m_state = nullptr; ///< Estado de depth-stencil opcional (propiedad de la clase).
};

/**
 * @example DepthStencilView_example.cpp
 * @code
 * DepthStencilView dsv;
 * HRESULT hr = dsv.create(device, width, height);
 * if (SUCCEEDED(hr)) {
 *   dsv.ensureDepthState(device, true); // opcional
 *   context->OMSetDepthStencilState(dsv.state(), 0); // si aseguraste el estado
 *   dsv.bind(context, renderTargetView);
 * }
 * @endcode
 */
