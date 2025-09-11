#include "../include/Prerequisites.h"
#include "../include/Window.h"

HRESULT Window::init(HINSTANCE hInstance, int nCmdShow, WNDPROC wndproc) {
  m_hInst = hInstance;

  static const wchar_t* kClass = L"TutorialWindowClass";

  WNDCLASSEXW wc{};
  wc.cbSize = sizeof(WNDCLASSEXW);
  wc.style = CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc = wndproc;
  wc.hInstance = m_hInst;
  wc.hIcon = LoadIconW(nullptr, IDI_APPLICATION);
  wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
  wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
  wc.lpszClassName = kClass;
  wc.hIconSm = LoadIconW(nullptr, IDI_APPLICATION);

  if (!RegisterClassExW(&wc))
    return HRESULT_FROM_WIN32(GetLastError());

  RECT rc{ 0,0,1200,950 };
  m_rect = rc;
  AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

  const wchar_t* title = m_windowName.empty() ? L"HeliosEngine" : m_windowName.c_str();

  m_hWnd = CreateWindowExW(
    0, kClass, title, WS_OVERLAPPEDWINDOW,
    CW_USEDEFAULT, CW_USEDEFAULT,
    rc.right - rc.left, rc.bottom - rc.top,
    nullptr, nullptr, m_hInst, nullptr);

  if (!m_hWnd) {
    MessageBoxW(nullptr, L"CreateWindow failed!", L"Error", MB_OK | MB_ICONERROR);
    return HRESULT_FROM_WIN32(GetLastError());
  }

  ShowWindow(m_hWnd, nCmdShow);
  UpdateWindow(m_hWnd);

  GetClientRect(m_hWnd, &m_rect);
  m_width = m_rect.right - m_rect.left;
  m_height = m_rect.bottom - m_rect.top;

  return S_OK;
}

void Window::update() {}
void Window::render() {}
void Window::destroy() {}
