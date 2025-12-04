#include "../include/Window.h"

//
// La funci�n `init` inicializa la ventana de la aplicaci�n.
// En este contexto de DirectX, la ventana act�a como el lienzo sobre el cual se renderiza la escena 3D.
//
HRESULT
Window::init(HINSTANCE hInstance,
    int nCmdShow,
    WNDPROC wndproc) {
    // Se guarda la instancia de la aplicaci�n (handle de la instancia) en una variable miembro.
    m_hInst = hInstance;

    //
    // Se registra la clase de la ventana. Esto define el "tipo" de ventana que se crear�,
    // incluyendo su icono, cursor y procedimiento de ventana (la funci�n que manejar� los mensajes).
    //
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW; // La ventana se redibujar� si se cambia su tama�o.
    wcex.lpfnWndProc = wndproc; // Se asigna el procedimiento de ventana.
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = m_hInst;
    wcex.hIcon = LoadIcon(m_hInst, (LPCTSTR)IDI_TUTORIAL1); // Se carga el icono de la aplicaci�n.
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW); // Se asigna el cursor por defecto.
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = "TutorialWindowClass"; // Se le da un nombre a la clase de la ventana.
    wcex.hIconSm = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_TUTORIAL1);
    // Se intenta registrar la clase de la ventana. Si falla, se devuelve un error.
    if (!RegisterClassEx(&wcex))
        return E_FAIL;

    //
    // Se crea la ventana. Primero, se define el tama�o deseado del "�rea de cliente" (el �rea de dibujo),
    // y luego se ajusta para incluir la barra de t�tulo y los bordes.
    //
    RECT
        rc = { 0, 0, 1200, 950 };
    m_rect = rc;

    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

    // Se llama a `CreateWindow` para crear la ventana real.
    m_hWnd = CreateWindow("TutorialWindowClass", // Nombre de la clase de la ventana que se va a crear.
        "Direct3D 11 Tutorial 7", // T�tulo de la ventana.
        WS_OVERLAPPEDWINDOW, // Estilo de la ventana.
        CW_USEDEFAULT, // Posici�n x por defecto.
        CW_USEDEFAULT, // Posici�n y por defecto.
        rc.right - rc.left, // Ancho de la ventana.
        rc.bottom - rc.top, // Alto de la ventana.
        NULL, // Sin ventana padre.
        NULL, // Sin men�.
        hInstance, // Handle de la instancia.
        NULL);
    // Si la creaci�n de la ventana falla, se muestra un mensaje de error y se devuelve E_FAIL.
    if (!m_hWnd) {
        MessageBox(nullptr, "CreateWindow failed!", "Error", MB_OK);
        ERROR("Window", "init", "CHECK FOR CreateWindow()");
        return E_FAIL;
    }

    // Se muestra la ventana en la pantalla.
    ShowWindow(m_hWnd, nCmdShow);

    // Se actualiza la ventana para que se dibuje por primera vez.
    UpdateWindow(m_hWnd);

    //
    // Se obtienen las dimensiones reales del �rea de cliente de la ventana
    // y se guardan en las variables miembro.
    //
    GetClientRect(m_hWnd, &m_rect);
    m_width = m_rect.right - m_rect.left;
    m_height = m_rect.bottom - m_rect.top;

    return S_OK;
}

//
// La funci�n `update` est� vac�a, ya que la l�gica de actualizaci�n de la ventana
// (como el manejo de mensajes de teclado o rat�n) se suele hacer fuera de esta clase.
//
void Window::update()
{
}

//
// La funci�n `render` est� vac�a, ya que la l�gica de renderizado se realiza en el
// dispositivo de Direct3D.
//
void Window::render()
{
}

//
// La funci�n `destroy` est� vac�a, ya que la destrucci�n de la ventana
// suele ser gestionada autom�ticamente por el sistema operativo al salir de la aplicaci�n.
//
void Window::destroy()
{
}