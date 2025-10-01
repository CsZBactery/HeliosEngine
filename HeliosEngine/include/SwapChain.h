#pragma once
#include "Prerequisites.h"

/**
 * @file SwapChain.h
 * @brief RAII mínimo para manejar una DXGI Swap Chain y su Render Target View (RTV).
 *
 * @details
 * Esta clase encapsula:
 * - La **swap chain** de DXGI (`IDXGISwapChain`) responsable de presentar la imagen en pantalla.
 * - El **Render Target View** (`ID3D11RenderTargetView`) asociado al back-buffer actual.
 *
 * Ofrece dos niveles de API:
 * - **Alto nivel**: `init(...)` crea *Device + Immediate Context + Swap Chain + RTV* y
 *   devuelve el back-buffer como `Texture`.
 * - **Bajo nivel**: `create(...)`, `recreateRTV(...)`, `resize(...)`, `present(...)`,
 *   y utilidades de *bind* (`bindAsRenderTarget(...)`).
 *
 * @note La clase no administra `ID3D11Device` ni `ID3D11DeviceContext`; solo los usa.
 * @warning El back-buffer típico **no** soporta SRV. Para post-proceso, renderiza a un
 *          RT intermedio con `BIND_RENDER_TARGET | BIND_SHADER_RESOURCE` y resuelve/copia
 *          al back-buffer antes de `present()`.
 */

 // ---------- Forward declarations de tus wrappers ----------
class Device;
class DeviceContext;
class Window;
class Texture;

/**
 * @class SwapChain
 * @brief Envoltura de `IDXGISwapChain` + `ID3D11RenderTargetView` (back-buffer actual).
 *
 * Propietaria de:
 * - `IDXGISwapChain* m_swap`
 * - `ID3D11RenderTargetView* m_rtv`
 *
 * No copiable (evita dobles `Release()`).
 */
class SwapChain {
public:
  /// @brief Ctor por defecto (no crea recursos).
  SwapChain() = default;

  /// @brief Dtor: libera RTV y swap chain (equivalente a @ref destroy).
  ~SwapChain() { destroy(); }

  /// @name Semántica de copia
  /// @{
  SwapChain(const SwapChain&) = delete;             ///< No copiable.
  SwapChain& operator=(const SwapChain&) = delete;  ///< No asignable por copia.
  /// @}

  // =========================================================================
  //                          ALTO NIVEL / “TODO EN UNO”
  // =========================================================================

  /**
   * @brief Inicializa *Device + Immediate Context + Swap Chain + RTV + Viewport* y expone back-buffer.
   *
   * @param device         Wrapper de `ID3D11Device` a poblar (quedará adjunto al device creado).
   * @param deviceContext  Wrapper de `ID3D11DeviceContext` (adjunta el inmediato creado).
   * @param backBuffer     Recibe el `ID3D11Texture2D` del back-buffer (como `Texture`).
   * @param window         Ventana destino (debe contener un `HWND` válido).
   * @return `S_OK` en éxito; código `HRESULT` en fallo.
   *
   * @pre `window` debe contener `HWND` válido.
   * @post Existe swap chain y RTV válidos; `backBuffer` contiene el recurso del back-buffer.
   *
   * @note El back-buffer **no** suele tener SRV; para sampling/post, usa un RT intermedio.
   * @warning No mezclar MSAA en el back-buffer con DXGI 1.0 (usar RT MSAA intermedio + resolve).
   */
  HRESULT init(Device& device,
    DeviceContext& deviceContext,
    Texture& backBuffer,
    const Window& window); // <-- CORREGIDO: const ref

  // =========================================================================
  //                                  LOW-LEVEL
  // =========================================================================

  /**
   * @brief Crea la swap chain (sin crear device/context) y configura el RTV inicial.
   *
   * @param device       Dispositivo D3D11 con el que se creará la cadena.
   * @param hwnd         Handle de la ventana destino.
   * @param width        Ancho de back-buffer (px).
   * @param height       Alto de back-buffer (px).
   * @param format       Formato del back-buffer (por defecto `DXGI_FORMAT_R8G8B8A8_UNORM`).
   * @param bufferCount  Número de buffers (1 por defecto).
   * @param windowed     `TRUE` para modo ventana; `FALSE` para fullscreen.
   * @param sampleCount  Multisampling (ignorado para back-buffer; se fija a 1 en DXGI 1.0).
   * @return `S_OK` en éxito; `HRESULT` en fallo.
   *
   * @pre `device != nullptr`, `hwnd != nullptr`, `width > 0`, `height > 0`.
   * @post RTV recreado a partir del nuevo back-buffer.
   * @warning El back-buffer se crea **sin MSAA**; si necesitas MSAA usa RT intermedio.
   */
  HRESULT create(ID3D11Device* device,
    HWND hwnd,
    UINT width,
    UINT height,
    DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM,
    UINT bufferCount = 1,
    BOOL windowed = TRUE,
    UINT sampleCount = 1);

  /**
   * @brief Recrea el RTV del back-buffer actual (tras resize o invalidación).
   *
   * @param device Dispositivo con el que crear el RTV.
   * @return `S_OK` en éxito; `HRESULT` en fallo.
   * @pre `m_swap != nullptr`.
   * @post `m_rtv` apunta al RTV del nuevo back-buffer.
   */
  HRESULT recreateRTV(ID3D11Device* device);

  /**
   * @brief Redimensiona el back-buffer y actualiza el RTV.
   *
   * @param device Dispositivo para crear el RTV.
   * @param width  Nuevo ancho (px).
   * @param height Nuevo alto (px).
   * @return `S_OK` en éxito; `HRESULT` en fallo.
   *
   * @pre `m_swap != nullptr`, `width > 0`, `height > 0`.
   * @post RTV recreado a partir del nuevo back-buffer; `width()/height()` actualizados.
   */
  HRESULT resize(ID3D11Device* device, UINT width, UINT height);

  /**
   * @brief Presenta el back-buffer en pantalla.
   *
   * @param syncInterval 0 = sin VSync; 1 = VSync activado.
   * @param flags        Flags DXGI (normalmente 0).
   * @return `S_OK` en éxito; `HRESULT` en fallo.
   *
   * @pre `m_swap != nullptr`.
   */
  HRESULT present(UINT syncInterval = 1, UINT flags = 0);

  /**
   * @brief Enlaza el RTV interno (y DSV opcional) al pipeline (OMSetRenderTargets).
   *
   * @param ctx Contexto inmediato (raw pointer).
   * @param dsv Depth-Stencil View opcional (puede ser `nullptr`).
   * @pre `ctx != nullptr`, `m_rtv != nullptr`.
   */
  void bindAsRenderTarget(ID3D11DeviceContext* ctx, ID3D11DepthStencilView* dsv) const;

  // =========================================================================
  //                                  WRAPPERS
  // =========================================================================

  /// @brief Versión wrapper de @ref create usando `Device` y `Window`.
  HRESULT create(Device& device,
    Window& window,
    UINT width,
    UINT height,
    DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM,
    UINT bufferCount = 1,
    BOOL windowed = TRUE,
    UINT sampleCount = 1);

  /// @brief Versión wrapper de @ref recreateRTV usando `Device`.
  HRESULT recreateRTV(Device& device);

  /// @brief Versión wrapper de @ref resize usando `Device`.
  HRESULT resize(Device& device, UINT width, UINT height);

  /// @brief Versión wrapper de @ref bindAsRenderTarget usando `DeviceContext`.
  void bindAsRenderTarget(DeviceContext& ctx, ID3D11DepthStencilView* dsv) const;

  // =========================================================================
  //                                 GESTIÓN
  // =========================================================================

  /**
   * @brief Libera RTV y swap chain y deja el objeto en estado vacío.
   *
   * @post `get() == nullptr`, `rtv() == nullptr`, `width() == height() == 0`.
   */
  void destroy();

  // ---------------------- Getters ----------------------

  /// @return Puntero crudo a la swap chain (puede ser `nullptr`).
  IDXGISwapChain* get()  const { return m_swap; }

  /// @return Puntero crudo al RTV actual (puede ser `nullptr`).
  ID3D11RenderTargetView* rtv()  const { return m_rtv; }

  /// @return Ancho actual (px) del back-buffer.
  UINT        width()  const { return m_width; }

  /// @return Alto actual (px) del back-buffer.
  UINT        height() const { return m_height; }

  /// @return Formato actual del back-buffer.
  DXGI_FORMAT format() const { return m_format; }

private:
  /// @brief Libera el RTV interno (si existe). @post `m_rtv == nullptr`.
  void destroyRTV_();

private:
  IDXGISwapChain* m_swap = nullptr;                 ///< Swap chain de DXGI (propietaria).
  ID3D11RenderTargetView* m_rtv = nullptr;                 ///< RTV del back-buffer actual (propietaria).
  UINT                   m_width = 0;                       ///< Ancho del back-buffer (px).
  UINT                   m_height = 0;                       ///< Alto del back-buffer (px).
  DXGI_FORMAT            m_format = DXGI_FORMAT_R8G8B8A8_UNORM; ///< Formato del back-buffer.
};
