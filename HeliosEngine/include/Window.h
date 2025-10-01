/**
 * @file Window.h
 * @brief Wrapper mínimo para crear y gestionar una ventana Win32 (Unicode).
 *
 * Responsabilidades:
 * - Registrar (si hace falta) y crear una ventana top-level.
 * - Mantener tamaño del área cliente (útil para RTV/DSV y reproyección).
 * - Exponer accesores seguros (HWND, HINSTANCE, ancho, alto, RECT).
 *
 * @note El WndProc se inyecta desde fuera (parámetro de @ref init).
 * @warning No es thread-safe. Usar en el hilo de UI (message loop).
 */

#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

 /**
  * @class Window
  * @brief Encapsula la creación y manejo básico de una ventana Win32.
  *
  * @details
  * - @ref init registra la clase y crea el HWND.
  * - @ref updateClientSize cachea ancho/alto/RECT del área cliente.
  * - @ref destroy destruye la ventana si existe.
  *
  * @par Ciclo de vida típico
  * 1) Llamar a @ref init en @c wWinMain.
  * 2) En @c WM_SIZE, invocar @ref updateClientSize y recrear RTV/DSV si aplica.
  * 3) Al salir, @ref destroy (opcional).
  */
class Window {
public:
  /// Constructor por defecto (no crea la ventana).
  Window() = default;

  /// Destructor por defecto (no llama destroy()).
  ~Window() = default;

  /// No copiable (posee HWND).
  Window(const Window&) = delete;
  Window& operator=(const Window&) = delete;

  /**
   * @brief Inicializa y crea la ventana (registra la clase si hace falta).
   *
   * @param hInst    Instancia del módulo (de @c wWinMain).
   * @param nCmdShow Modo de presentación (por ejemplo @c SW_SHOW).
   * @param wndproc  Procedimiento de ventana (WndProc) a usar.
   * @return @c S_OK en éxito; HRESULT de error en caso contrario.
   *
   * @post Si retorna @c S_OK, @ref handle() es no nulo y @ref updateClientSize ha sido llamado.
   * @note Usa API Unicode (sufijo W).
   */
  HRESULT init(HINSTANCE hInst, int nCmdShow, WNDPROC wndproc);

  /**
   * @brief Recalcula y cachea el tamaño del área cliente.
   *
   * @details Llama a @c GetClientRect sobre @ref handle() y actualiza @ref m_rect,
   *          @ref m_width y @ref m_height.
   * @pre @ref handle() debe ser válido.
   */
  void updateClientSize();

  /**
   * @brief Destruye la ventana si existe y limpia los campos internos.
   *
   * @post @ref handle() pasa a @c nullptr, tamaños = 0.
   * @note No desregistra la clase (no suele ser necesario).
   */
  void destroy();

  // -------- Accesores --------

  /**
   * @brief Handle de la ventana.
   * @return @c HWND o @c nullptr si no se ha creado.
   */
  HWND        handle()      const { return m_hWnd; }

  /**
   * @brief Instancia del módulo asociada.
   * @return @c HINSTANCE o @c nullptr si no se ha inicializado.
   */
  HINSTANCE   instance()    const { return m_hInst; }

  /**
   * @brief Ancho del área cliente en píxeles.
   */
  int         width()       const { return m_width; }

  /**
   * @brief Alto del área cliente en píxeles.
   */
  int         height()      const { return m_height; }

  /**
   * @brief RECT del área cliente (coordenadas relativas al HWND).
   */
  const RECT& clientRect()  const { return m_rect; }

private:
  HINSTANCE m_hInst = nullptr;     ///< Instancia del módulo.
  HWND      m_hWnd = nullptr;     ///< Handle de la ventana.
  RECT      m_rect{ 0, 0, 0, 0 };  ///< Área cliente cacheada.
  int       m_width = 0;          ///< Ancho del área cliente.
  int       m_height = 0;          ///< Alto del área cliente.
};

/**
 * @example Window_example.cpp
 * @code
 * // En wWinMain:
 * Window g_window;
 * if (FAILED(g_window.init(hInstance, nCmdShow, WndProc))) return 0;
 *
 * // En WndProc:
 * LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
 *   switch (msg) {
 *     case WM_SIZE:
 *       g_window.updateClientSize();
 *       // Recreate RTV/DSV + reproyección aquí...
 *       return 0;
 *     case WM_DESTROY:
 *       PostQuitMessage(0);
 *       return 0;
 *   }
 *   return DefWindowProcW(hWnd, msg, wParam, lParam);
 * }
 * @endcode
 */
