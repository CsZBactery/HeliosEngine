#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "../include/Window.h"

static const wchar_t* kWndClassName = L"HeliosWndClass";

HRESULT Window::init(HINSTANCE hInst, int nCmdShow, WNDPROC wndproc)
{
  if (!hInst || !wndproc) return E_POINTER;
  m_hInst = hInst;

  // Registra clase si no existe todavía
  WNDCLASSEXW wc{};
  wc.cbSize = sizeof(wc);
  wc.style = CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc = wndproc;
  wc.hInstance = m_hInst;
  wc.hIcon = LoadIconW(nullptr, IDI_APPLICATION);
  wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
  wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
  wc.lpszClassName = kWndClassName;
  wc.hIconSm = wc.hIcon;

  // Si ya estaba registrada, RegisterClassExW falla; lo ignoramos si ERROR_CLASS_ALREADY_EXISTS
  ATOM a = RegisterClassExW(&wc);
  if (!a) {
    DWORD err = GetLastError();
    if (err != ERROR_CLASS_ALREADY_EXISTS)
      return HRESULT_FROM_WIN32(err);
  }

  // Crea la ventana
  m_hWnd = CreateWindowExW(
    0, kWndClassName, L"HeliosEngine",
    WS_OVERLAPPEDWINDOW,
    CW_USEDEFAULT, CW_USEDEFAULT, 1280, 720,
    nullptr, nullptr, m_hInst, nullptr);

  if (!m_hWnd) return HRESULT_FROM_WIN32(GetLastError());

  ShowWindow(m_hWnd, nCmdShow);
  UpdateWindow(m_hWnd);

  updateClientSize();
  return S_OK;
}

void Window::updateClientSize()
{
  if (!m_hWnd) return;
  if (GetClientRect(m_hWnd, &m_rect)) {
    m_width = m_rect.right - m_rect.left;
    m_height = m_rect.bottom - m_rect.top;
  }
}

void Window::destroy()
{
  if (m_hWnd) {
    DestroyWindow(m_hWnd);
    m_hWnd = nullptr;
  }
  m_rect = RECT{ 0,0,0,0 };
  m_width = 0;
  m_height = 0;
  // No desregistramos la clase; no es necesario para apps simples.
}
