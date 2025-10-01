/**
 * @file RenderTargetView.h
 * @brief Wrapper mínimo para crear y gestionar un Render Target View (RTV) en D3D11.
 *
 * Permite:
 * - Crear un RTV desde el backbuffer de una swap chain.
 * - Crear un RTV desde una textura 2D existente (con desc opcional).
 *
 * @note Usa punteros COM crudos; @ref destroy libera los recursos. No es thread-safe.
 */

#pragma once
#include <d3d11.h>
#include <dxgi.h>

 /**
  * @class RenderTargetView
  * @brief Encapsula un @c ID3D11RenderTargetView y su ciclo de vida.
  *
  * @details
  * - @ref createFromBackbuffer obtiene el backbuffer 0 de una @c IDXGISwapChain y crea el RTV.
  * - @ref createFromTexture crea el RTV a partir de una @c ID3D11Texture2D ya existente.
  * - @ref destroy libera el RTV si existe (invocado también por el destructor).
  *
  * @par Uso típico
  * 1) Llama a @ref createFromBackbuffer tras crear la swap chain o después de un resize.
  * 2) Haz bind con @c OMSetRenderTargets(1, &rtv, dsv).
  */
class RenderTargetView {
public:
  /// Constructor por defecto (no adquiere recursos).
  RenderTargetView() = default;

  /**
   * @brief Destructor: libera el RTV llamando a @ref destroy.
   */
  ~RenderTargetView() { destroy(); }

  /// No copiable (posee recursos COM).
  RenderTargetView(const RenderTargetView&) = delete;
  RenderTargetView& operator=(const RenderTargetView&) = delete;

  /**
   * @brief Libera el @c ID3D11RenderTargetView si existe.
   *
   * @post @ref get() pasa a devolver @c nullptr.
   */
  void destroy();

  /**
   * @brief Crea el RTV desde el backbuffer (índice 0) de una swap chain.
   *
   * @param swap  Swap chain válida (no nula).
   * @param dev   Dispositivo D3D11 usado para crear la vista.
   * @return @c S_OK en éxito; HRESULT de error en caso contrario.
   *
   * @note Llama internamente a @c IDXGISwapChain::GetBuffer(0, ...) y @c ID3D11Device::CreateRenderTargetView.
   * @warning En caso de resize, debes llamar a @ref destroy y recrear el RTV.
   */
  HRESULT createFromBackbuffer(IDXGISwapChain* swap, ID3D11Device* dev);

  /**
   * @brief Crea el RTV a partir de una textura 2D existente.
   *
   * @param dev   Dispositivo D3D11.
   * @param tex   Textura 2D origen (no nula).
   * @param desc  Descriptor opcional del RTV; si es @c nullptr se infiere por defecto.
   * @return @c S_OK en éxito; HRESULT de error en caso contrario.
   *
   * @note Útil para render targets offscreen o formatos especiales.
   */
  HRESULT createFromTexture(ID3D11Device* dev, ID3D11Texture2D* tex,
    const D3D11_RENDER_TARGET_VIEW_DESC* desc = nullptr);

  /**
   * @brief Acceso al puntero COM del RTV.
   * @return @c ID3D11RenderTargetView* o @c nullptr si no existe.
   */
  ID3D11RenderTargetView* get() const { return m_rtv; }

private:
  ID3D11RenderTargetView* m_rtv = nullptr; ///< Vista de render (propiedad de la clase).
};

/**
 * @example RenderTargetView_example.cpp
 * @code
 * RenderTargetView rtv;
 * // Desde backbuffer:
 * HRESULT hr = rtv.createFromBackbuffer(swapChain, device);
 * if (SUCCEEDED(hr)) {
 *   ID3D11RenderTargetView* view = rtv.get();
 *   context->OMSetRenderTargets(1, &view, dsv); // dsv puede ser nullptr si no usas profundidad
 * }
 *
 * // Desde textura:
 * ID3D11Texture2D* customRT = /* textura creada con BIND_RENDER_TARGET * /;
 * rtv.destroy();
 * hr = rtv.createFromTexture(device, customRT);
 * @endcode
 */
