#include "../include/Window.h"
#include "../include/Device.h"
#include "../include/DeviceContext.h"
#include "../include/Resource.h" 

HRESULT Window::init(HINSTANCE hInstance, int nCmdShow, WNDPROC wndproc)
{
    m_hInst = hInstance;

    // ---- Clase de ventana (WIDE) ----
    WNDCLASSEXW wcex{};
    wcex.cbSize = sizeof(WNDCLASSEXW);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = wndproc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = m_hInst;
    wcex.hIcon = LoadIconW(m_hInst, MAKEINTRESOURCEW(IDI_TUTORIAL1));
    wcex.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = L"TutorialWindowClass";
    wcex.hIconSm = LoadIconW(wcex.hInstance, MAKEINTRESOURCEW(IDI_TUTORIAL1));

    if (!RegisterClassExW(&wcex))
        return E_FAIL;

    // ---- Tamaño deseado del área cliente ----
    RECT rc = { 0, 0, 1200, 950 };
    m_rect = rc;
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

    // ---- Crear ventana (WIDE) ----
    m_hWnd = CreateWindowExW(
        0,
        L"TutorialWindowClass",
        L"Direct3D 11 Tutorial 7",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        rc.right - rc.left,
        rc.bottom - rc.top,
        nullptr, nullptr, hInstance, nullptr);

    if (!m_hWnd) {
        MessageBoxW(nullptr, L"CreateWindow failed!", L"Error", MB_OK);
        ERROR(L"Window", L"init", L"CHECK FOR CreateWindow()");
        return E_FAIL;
    }

    ShowWindow(m_hWnd, nCmdShow);
    UpdateWindow(m_hWnd);

    // ---- Guardar dimensiones del área cliente ----
    GetClientRect(m_hWnd, &m_rect);
    m_width = static_cast<UINT>(m_rect.right - m_rect.left);
    m_height = static_cast<UINT>(m_rect.bottom - m_rect.top);

    return S_OK;
}

void Window::update() {}
void Window::render() {}
void Window::destroy() {}
