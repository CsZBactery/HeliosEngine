#include "Window.h"

HRESULT
Window::init(HINSTANCE hInstance, int nCmdShow, WNDPROC wndproc) {
  // Guardar instancia
  m_hInst = hInstance;

  // Registrar clase (UNICODE)
  static const wchar_t* kClassName = L"TutorialWindowClass";

  WNDCLASSEXW wcex{};
  wcex.cbSize = sizeof(WNDCLASSEXW);
  wcex.style = CS_HREDRAW | CS_VREDRAW;
  wcex.lpfnWndProc = wndproc;
  wcex.cbClsExtra = 0;
  wcex.cbWndExtra = 0;
  wcex.hInstance = m_hInst;
  wcex.hIcon = LoadIconW(nullptr, IDI_APPLICATION); // sin resource.h
  wcex.hCursor = LoadCursorW(nullptr, IDC_ARROW);
  wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
  wcex.lpszMenuName = nullptr;
  wcex.lpszClassName = kClassName;
  wcex.hIconSm = LoadIconW(nullptr, IDI_APPLICATION);
  if (!RegisterClassExW(&wcex))
    return HRESULT_FROM_WIN32(GetLastError());

  // Crear ventana
  RECT rc = { 0, 0, 1200, 950 };
  m_rect = rc;
  AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

  const wchar_t* title = m_windowName.empty() ? L"HeliosEngine" : m_windowName.c_str();

  m_hWnd = CreateWindowExW(
    0,
    kClassName,
    title,
    WS_OVERLAPPEDWINDOW,
    CW_USEDEFAULT, CW_USEDEFAULT,
    rc.right - rc.left,
    rc.bottom - rc.top,
    nullptr, nullptr,
    m_hInst,
    nullptr
  );

  if (!m_hWnd) {
    MessageBoxW(nullptr, L"CreateWindow failed!", L"Error", MB_OK | MB_ICONERROR);
    return HRESULT_FROM_WIN32(GetLastError());
  }

  ShowWindow(m_hWnd, nCmdShow);
  UpdateWindow(m_hWnd);

  // Dimensiones del área cliente
  GetClientRect(m_hWnd, &m_rect);
  m_width = m_rect.right - m_rect.left;
  m_height = m_rect.bottom - m_rect.top;

  return S_OK;
}

void Window::update() {}
void Window::render() {}
void Window::destroy() {}
