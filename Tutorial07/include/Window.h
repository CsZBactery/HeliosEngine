#pragma once
#include "Prerequisites.h"
#include <string>

// Versi�n simple: dejamos los miembros p�blicos para que tu c�digo viejo
// (g_window.m_width / m_hWnd / etc.) siga funcionando SIN tocar nada m�s.
class Window {
public:
  HRESULT init(HINSTANCE hInstance, int nCmdShow, WNDPROC wndproc);
  void update();
  void render();
  void destroy();

  // Miembros accesibles (compat)
  HINSTANCE    m_hInst = nullptr;
  HWND         m_hWnd = nullptr;
  RECT         m_rect{ 0,0,0,0 };
  int          m_width = 0;
  int          m_height = 0;
  std::wstring m_windowName = L"HeliosEngine";
};
