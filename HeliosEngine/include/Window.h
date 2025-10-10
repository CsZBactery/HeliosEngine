/**
 * @file Window.h
 * @brief Wrapper mínimo para crear y gestionar una ventana Win32 (Unicode).
 */

#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

class Window {
public:
  Window() = default;
  ~Window() = default;

  Window(const Window&) = delete;
  Window& operator=(const Window&) = delete;

  HRESULT init(HINSTANCE hInst, int nCmdShow, WNDPROC wndproc);
  void    updateClientSize();
  void    destroy();

  // Accesores
  HWND      handle()     const { return m_hWnd; }
  HINSTANCE instance()   const { return m_hInst; }
  int       width()      const { return m_width; }
  int       height()     const { return m_height; }
  const RECT& clientRect() const { return m_rect; }

  // --- Alias de compatibilidad con código previo ---
  int getWidth()  const { return width(); }
  int getHeight() const { return height(); }

private:
  HINSTANCE m_hInst = nullptr;
  HWND      m_hWnd = nullptr;
  RECT      m_rect{ 0,0,0,0 };
  int       m_width = 0;
  int       m_height = 0;
};
