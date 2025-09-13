#pragma once
/**
 * @file Window.h
 * @brief Encapsula una ventana Win32 básica para apps con Direct3D 11.
 *
 * @details
 * Provee utilidades mínimas para crear y mostrar una ventana (Unicode),
 * además de exponer el HWND y las dimensiones del área cliente para
 * inicialización de gráficos.
 */

#include "Prerequisites.h"
#include <string>

 /**
  * @class Window
  * @brief Wrapper simple de una ventana Win32.
  *
  * @details
  * - Registra una clase de ventana y crea la ventana con estilo `WS_OVERLAPPEDWINDOW`.
  * - Llama internamente a `ShowWindow` y `UpdateWindow`.
  * - Calcula y guarda el tamaño del área cliente (ancho/alto).
  *
  * @note Esta clase **no** administra recursos de D3D11; sólo la ventana.
  * @warning Los miembros públicos existen por compatibilidad con tu código actual.
  */
class Window {
public:
  /**
   * @brief Crea y muestra la ventana principal.
   * @param hInstance Identificador de instancia del proceso (Win32).
   * @param nCmdShow  Parámetro estándar para `ShowWindow` (e.g., `SW_SHOW`).
   * @param wndproc   Procedimiento de ventana (callback) con firma `LRESULT (CALLBACK*)(HWND, UINT, WPARAM, LPARAM)`.
   * @return `S_OK` si tuvo éxito; en caso de error, un `HRESULT` apropiado (p. ej., `HRESULT_FROM_WIN32(GetLastError())`).
   *
   * @post Si tiene éxito:
   * - `m_hInst` y `m_hWnd` quedan inicializados.
   * - `m_width` y `m_height` reflejan el tamaño del área cliente.
   * - `m_rect` guarda el rectángulo cliente actual.
   */
  HRESULT init(HINSTANCE hInstance, int nCmdShow, WNDPROC wndproc);

  /**
   * @brief Actualización por cuadro (placeholder).
   * @details Gancho para lógica de ventana/timers/inputs si lo necesitas.
   */
  void update();

  /**
   * @brief Render por cuadro (placeholder).
   * @details No realiza dibujo por sí misma; mantenlo para consistencia.
   */
  void render();

  /**
   * @brief Destruye/oculta la ventana y limpia su estado.
   * @details Suele llamar a `DestroyWindow(m_hWnd)` y poner punteros a `nullptr`.
   * @warning No destruye recursos de D3D11. Haz esa limpieza aparte.
   */
  void destroy();

  // ------------------------------------------------------------------
  // Campos públicos (compatibilidad con tu código existente)
  // ------------------------------------------------------------------

  /** @brief Instancia del módulo (Win32). */
  HINSTANCE    m_hInst = nullptr;

  /** @brief Handle de la ventana creada. */
  HWND         m_hWnd = nullptr;

  /** @brief Rectángulo del área cliente (resultado de `GetClientRect`). */
  RECT         m_rect{ 0, 0, 0, 0 };

  /** @brief Ancho del área cliente en píxeles. */
  int          m_width = 0;

  /** @brief Alto del área cliente en píxeles. */
  int          m_height = 0;

  /**
   * @brief Título/nombre de la ventana (Unicode).
   * @note Puedes cambiarlo antes de llamar a `init()` para personalizar el caption.
   */
  std::wstring m_windowName = L"HeliosEngine";
};
