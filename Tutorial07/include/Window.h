#pragma once
#include "Prerequisites.h"
#include <string>

class Window {
public:
  HRESULT init(HINSTANCE hInstance, int nCmdShow, WNDPROC wndproc);
  void update();
  void render();
  void destroy();

  // Públicos para compatibilidad con tu código existente
  HINSTANCE    m_hInst = nullptr;
  HWND         m_hWnd = nullptr;
  RECT         m_rect{ 0,0,0,0 };
  int          m_width = 0;
  int          m_height = 0;
  std::wstring m_windowName = L"HeliosEngine";
};
