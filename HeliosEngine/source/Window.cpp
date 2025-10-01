// Window.cpp
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include "../include/Resource.h"   // IDs de icono (ej. IDI_TUTORIAL1)
#include "../include/Window.h"

static const wchar_t* kWindowClass = L"DX11SampleWindowClass";         

HRESULT Window::init(HINSTANCE hInst, int nCmdShow, WNDPROC wndproc)
{
  m_hInst = hInst;

  // Si la clase no está registrada aún, regístrala
  WNDCLASSEXW wcQuery{};
  if (!GetClassInfoExW(hInst, kWindowClass, &wcQuery)) {
    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = wndproc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInst;

    // Iconos desde resources; si fallan, usa los del sistema
    HICON bigIcon = LoadIconW(hInst, MAKEINTRESOURCEW(IDI_TUTORIAL1));
    if (!bigIcon) bigIcon = LoadIconW(nullptr, IDI_APPLICATION);
    wc.hIcon = bigIcon;

    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = nullptr;
    wc.lpszClassName = kWindowClass;

    HICON smallIcon = LoadIconW(hInst, MAKEINTRESOURCEW(IDI_TUTORIAL1));
    if (!smallIcon) smallIcon = LoadIconW(nullptr, IDI_APPLICATION);
    wc.hIconSm = smallIcon;

    if (!RegisterClassExW(&wc))
      return HRESULT_FROM_WIN32(GetLastError());
  }

  // Tamaño cliente deseado
  RECT rc{ 0, 0, 1200, 950 };
  AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

  // Crear ventana
  m_hWnd = CreateWindowExW(
    0,
    kWindowClass,
    L"HELIOSENGINE",           // título
    WS_OVERLAPPEDWINDOW,
    CW_USEDEFAULT, CW_USEDEFAULT,
    rc.right - rc.left, rc.bottom - rc.top,
    nullptr, nullptr, hInst, nullptr);

  if (!m_hWnd)
    return HRESULT_FROM_WIN32(GetLastError());

  ShowWindow(m_hWnd, nCmdShow);
  UpdateWindow(m_hWnd);

  updateClientSize();
  return S_OK;
}

void Window::updateClientSize()
{
  if (!m_hWnd) { m_width = m_height = 0; m_rect = RECT{ 0,0,0,0 }; return; }
  GetClientRect(m_hWnd, &m_rect);
  m_width = m_rect.right - m_rect.left;
  m_height = m_rect.bottom - m_rect.top;
}

void Window::destroy()
{
  if (m_hWnd) {
    DestroyWindow(m_hWnd);
    m_hWnd = nullptr;
  }
  m_width = m_height = 0;
  m_rect = RECT{ 0,0,0,0 };
}
