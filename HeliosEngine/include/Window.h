#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

class Window {
public:
  Window() = default;
  ~Window() = default;

  // No copiable
  Window(const Window&) = delete;
  Window& operator=(const Window&) = delete;

  // Inicializa y crea la ventana (registra la clase si hace falta)
  HRESULT init(HINSTANCE hInst, int nCmdShow, WNDPROC wndproc);

  // Recalcula el tamaño del área cliente (opcional, útil en WM_SIZE)
  void updateClientSize();

  // Cierra/limpia la ventana si existe (opcional)
  void destroy();

  // Accesores
  HWND        handle()      const { return m_hWnd; }
  HINSTANCE   instance()    const { return m_hInst; }
  int         width()       const { return m_width; }
  int         height()      const { return m_height; }
  const RECT& clientRect()  const { return m_rect; }

private:
  HINSTANCE m_hInst = nullptr;
  HWND      m_hWnd = nullptr;
  RECT      m_rect{ 0, 0, 0, 0 };
  int       m_width = 0;
  int       m_height = 0;
};
