#pragma once
#include "Prerequisites.h"

/**
 * @file SwapChain.h
 * @brief Encapsula una DXGI swap chain y su Render Target View (RTV) asociado.
 *
 * Clase ligera que administra el ciclo de vida de una `IDXGISwapChain` y el
 * `ID3D11RenderTargetView` creado a partir del back-buffer. Expone dos familias
 * de métodos:
 * - **Low-level**: aceptan punteros crudos de D3D11/DXGI.
 * - **Wrappers**: aceptan tus clases `Device`, `DeviceContext`, `Window` y llaman
 *   internamente a los métodos low-level.
 *
 * @note Clase no copiable. Después de un @ref resize siempre se invalida el RTV anterior
 *       y se crea uno nuevo con @ref recreateRTV.
 */

 // ---------- Forward declarations de tus wrappers ----------
class Device;
class DeviceContext;
class Window;
class Texture;   

/**
 * @class SwapChain
 * @brief RAII mínimo para swap chain + RTV.
 *
 * Propietaria de:
 * - `IDXGISwapChain* m_swap`
 * - `ID3D11RenderTargetView* m_rtv`
 *
 * No posee ni administra el `ID3D11Device` ni el `ID3D11DeviceContext`.
 */
class SwapChain {
public:
  /// Constructor por defecto (no crea recursos).
  SwapChain() = default;

  /// Destructor: libera RTV y swap chain si siguen vivos.
  ~SwapChain() { destroy(); }

  /// No copiable.
  SwapChain(const SwapChain&) = delete;
  SwapChain& operator=(const SwapChain&) = delete;

  // ---------------------------------------------------------------------------
  //                               LOW-LEVEL API
  // ---------------------------------------------------------------------------

  /**
   * @brief Crea la swap chain y almacena dimensiones/formato.
   * @param device       Dispositivo D3D11 con el que se creará la cadena.
   * @param hwnd         Handle de la ventana destino.
   * @param width        Ancho del back-buffer en píxeles.
   * @param height       Alto del back-buffer en píxeles.
   * @param format       Formato del back-buffer (RGBA8 por defecto).
   * @param bufferCount  Número de buffers (1 por defecto).
   * @param windowed     TRUE para modo ventana, FALSE para fullscreen.
   * @param sampleCount  Multisampling (1 = desactivado).
   * @return `S_OK` en éxito; HRESULT de error en fallo.
   *
   * @post Si la creación es correcta, @ref recreateRTV es invocado para crear
   *       el RTV del back-buffer.
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
   * @brief Vuelve a crear el RTV a partir del back-buffer actual.
   * @param device Dispositivo usado para crear el RTV.
   * @return `S_OK` en éxito; HRESULT en fallo.
   * @warning Debe llamarse tras un @ref resize o si se ha liberado el RTV.
   */
  HRESULT recreateRTV(ID3D11Device* device);

  /**
   * @brief Redimensiona el back-buffer (ResizeBuffers) y actualiza estado interno.
   * @param device Dispositivo para crear el nuevo RTV.
   * @param width  Nuevo ancho.
   * @param height Nuevo alto.
   * @return `S_OK` en éxito; HRESULT en fallo.
   * @post El RTV viejo se libera y se crea uno nuevo.
   */
  HRESULT resize(ID3D11Device* device, UINT width, UINT height);

  /**
   * @brief Presenta el back-buffer en pantalla.
   * @param syncInterval 0 = sin VSync; 1 = VSync activado.
   * @param flags        Flags DXGI (normalmente 0).
   * @return `S_OK` en éxito; HRESULT en fallo.
   */
  HRESULT present(UINT syncInterval = 1, UINT flags = 0);

  /**
   * @brief Enlaza el RTV interno (y un DSV opcional) al pipeline (OMSetRenderTargets).
   * @param ctx Device context que recibirá el binding.
   * @param dsv Vista de profundidad opcional (puede ser nullptr).
   * @pre Debe existir un RTV válido (tras @ref create o @ref recreateRTV).
   */
  void bindAsRenderTarget(ID3D11DeviceContext* ctx,
    ID3D11DepthStencilView* dsv) const;

  // ---------------------------------------------------------------------------
  //                               WRAPPERS (tus clases)
  // ---------------------------------------------------------------------------

  /**
   * @brief Versión wrapper de @ref create usando `Device` y `Window`.
   */
  HRESULT create(Device& device,
    Window& window,
    UINT width,
    UINT height,
    DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM,
    UINT bufferCount = 1,
    BOOL windowed = TRUE,
    UINT sampleCount = 1);

  /**
   * @brief Versión wrapper de @ref recreateRTV usando `Device`.
   */
  HRESULT recreateRTV(Device& device);

  /**
   * @brief Versión wrapper de @ref resize usando `Device`.
   */
  HRESULT resize(Device& device, UINT width, UINT height);

  /**
   * @brief Versión wrapper de @ref bindAsRenderTarget usando `DeviceContext`.
   */
  void bindAsRenderTarget(DeviceContext& ctx,
    ID3D11DepthStencilView* dsv) const;

  // ---------------------------------------------------------------------------
  //                            CICLO DE VIDA / GETTERS
  // ---------------------------------------------------------------------------

  /**
   * @brief Libera RTV y swap chain. Deja el objeto en estado vacío.
   */
  void destroy();

  /// @return Puntero crudo a la swap chain (puede ser nullptr).
  IDXGISwapChain* get()  const { return m_swap; }

  /// @return Puntero crudo al RTV (puede ser nullptr).
  ID3D11RenderTargetView* rtv()  const { return m_rtv; }

  /// @return Ancho actual en píxeles.
  UINT        width()  const { return m_width; }

  /// @return Alto actual en píxeles.
  UINT        height() const { return m_height; }

  /// @return Formato actual del back-buffer.
  DXGI_FORMAT format() const { return m_format; }

private:
  /// Libera el RTV interno (si existe).
  void destroyRTV_();

  IDXGISwapChain* m_swap = nullptr; //!< Swap chain de DXGI (propietaria).
  ID3D11RenderTargetView* m_rtv = nullptr; //!< RTV del back-buffer (propietaria).
  UINT        m_width = 0;                   //!< Dimensión X del back-buffer.
  UINT        m_height = 0;                   //!< Dimensión Y del back-buffer.
  DXGI_FORMAT m_format = DXGI_FORMAT_R8G8B8A8_UNORM; //!< Formato del back-buffer.
};
