#include "Window.h"
#include "BaseApp.h" 

// ======================================================================================
// La función `init` inicializa la ventana de la aplicación.
// En este contexto de DirectX, la ventana actúa como el lienzo sobre el cual se renderiza 
// la escena 3D (área cliente) y es el punto de entrada para los eventos del sistema.
// ======================================================================================
HRESULT
Window::init(HINSTANCE hInstance, int nCmdShow, WNDPROC wndproc, BaseApp* app) {
    // Se guarda la instancia de la aplicación (handle de la instancia) en una variable miembro.
    m_hInst = hInstance;

    // Se registra la clase de la ventana. Esto define el "tipo" de ventana que se creará,
    // incluyendo su icono, cursor y procedimiento de ventana (la función que manejará los mensajes).
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW; // La ventana se redibujará si se cambia su tamaño.
    wcex.lpfnWndProc = wndproc;           // Se asigna el procedimiento de ventana estático.
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = m_hInst;
    wcex.hIcon = LoadIcon(m_hInst, (LPCTSTR)IDI_TUTORIAL1);       // Icono principal.
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);                   // Cursor del mouse por defecto.
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = "TutorialWindowClass";                   // Identificador interno de la clase.
    wcex.hIconSm = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_TUTORIAL1);

    // Se intenta registrar la clase de la ventana. Si falla, se devuelve un error.
    if (!RegisterClassEx(&wcex)) {
        return E_FAIL;
    }

    // Se crea la ventana. Primero, se define el tamaño deseado del "área de cliente" (el área de dibujo),
    // y luego se ajusta para incluir la barra de título y los bordes.
    RECT rc = { 0, 0, 1200, 950 };
    m_rect = rc;

    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

    // Se llama a `CreateWindow` para crear la ventana real.
    m_hWnd = CreateWindow(
        "TutorialWindowClass",      // Nombre de la clase de la ventana.
        m_windowName.c_str(),       // Título de la ventana (Obtenido dinámicamente de la cabecera).
        WS_OVERLAPPEDWINDOW,        // Estilo de la ventana (bordes, minimizar, maximizar).
        CW_USEDEFAULT,              // Posición x por defecto (Windows decide).
        CW_USEDEFAULT,              // Posición y por defecto.
        rc.right - rc.left,         // Ancho total calculado de la ventana.
        rc.bottom - rc.top,         // Alto total calculado de la ventana.
        NULL,                       // Sin ventana padre.
        NULL,                       // Sin menú nativo de Windows.
        hInstance,                  // Handle de la instancia.
        app                         // ¡CRÍTICO! Pasamos el puntero BaseApp para gestionar el input.
    );

    // Si la creación de la ventana falla, se muestra un mensaje de error y se devuelve E_FAIL.
    if (!m_hWnd) {
        MessageBox(nullptr, "CreateWindow failed!", "Error", MB_OK);
        ERROR("Window", "init", "CHECK FOR CreateWindow()");
        return E_FAIL;
    }

    // Se muestra la ventana en la pantalla obedeciendo el estado sugerido (nCmdShow).
    ShowWindow(m_hWnd, nCmdShow);

    // Se actualiza la ventana para forzar el primer dibujado.
    UpdateWindow(m_hWnd);

    // Se obtienen las dimensiones reales del área de cliente de la ventana
    // y se guardan en las variables miembro para su uso en el SwapChain y el Viewport.
    GetClientRect(m_hWnd, &m_rect);
    m_width = m_rect.right - m_rect.left;
    m_height = m_rect.bottom - m_rect.top;

    return S_OK;
}

// ======================================================================================
// La función `update` está vacía, ya que la lógica de actualización (procesar mensajes 
// del sistema operativo) se realiza en el bucle principal de BaseApp.
// ======================================================================================
void Window::update()
{
}

// ======================================================================================
// La función `render` está vacía, ya que la lógica de renderizado se delega completamente
// a DirectX a través del SwapChain y el DeviceContext.
// ======================================================================================
void Window::render()
{
}

// ======================================================================================
// La función `destroy` está vacía, ya que los recursos nativos de ventana se liberan
// enviando mensajes (como WM_DESTROY) que el SO procesa automáticamente.
// ======================================================================================
void Window::destroy()
{
}