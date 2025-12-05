#pragma once
#include "Prerequisites.h"

/**
 * @class Window
 * @brief Encapsula una ventana nativa de sistema (Win32 API).
 *
 * Esta clase se encarga de:
 * 1. Registrar la clase de ventana en Windows.
 * 2. Crear la ventana física y gestionar su ciclo de vida (creación, gestión, destrucción).
 * 3. Proporcionar el **HWND** (Handle Window), que es el identificador que DirectX
 * necesita para crear la SwapChain y saber dónde dibujar los gráficos.
 */
class Window {
public:
    /**
     * @brief Constructor por defecto.
     */
    Window() = default;

    /**
     * @brief Destructor por defecto.
     */
    ~Window() = default;

    /**
     * @brief Inicializa, registra y muestra la ventana de la aplicación.
     *
     * @param hInstance Manejador (Handle) de la instancia de la aplicación (proviene del Main).
     * @param nCmdShow Parámetro que indica cómo se mostrará la ventana (minimizado, maximizado, etc.).
     * @param wndproc Puntero a la función de procedimiento de ventana (Callback) que procesará
     * los mensajes de entrada (teclado, ratón, cierre).
     * @return HRESULT S_OK si la ventana se creó y registró correctamente.
     */
    HRESULT
        init(HINSTANCE hInstance, int nCmdShow, WNDPROC wndproc);

    /**
     * @brief Actualiza la lógica de la ventana.
     *
     * Procesa la cola de mensajes de Windows (PeekMessage/Translate/Dispatch) para mantener
     * la ventana receptiva (que no se congele).
     */
    void
        update();

    /**
     * @brief Renderiza el contenido de la ventana.
     *
     * @note En este motor, el renderizado real lo hace DirectX (Device/SwapChain),
     * por lo que este método suele estar vacío o usado para dibujar bordes de ventana (GDI).
     */
    void
        render();

    /**
     * @brief Cierra la ventana y libera los recursos del sistema operativo.
     *
     * Llama a DestroyWindow y UnregisterClass.
     */
    void
        destroy();

public:
    /**
     * @brief Handle (Manejador) de la ventana Win32.
     *
     * Este es el dato más importante de esta clase. Se debe pasar a la SwapChain
     * para vincular la salida de la GPU con esta ventana.
     */
    HWND m_hWnd = nullptr;

    /**
     * @brief Ancho actual del área cliente de la ventana en píxeles.
     */
    unsigned int m_width;

    /**
     * @brief Alto actual del área cliente de la ventana en píxeles.
     */
    unsigned int m_height;

private:
    /**
     * @brief Handle de la instancia de la aplicación (identificador del .exe en memoria).
     */
    HINSTANCE m_hInst = nullptr;

    /**
     * @brief Estructura que define las dimensiones y posición del rectángulo de la ventana.
     */
    RECT m_rect;

    /**
     * @brief Nombre que aparecerá en la barra de título.
     * @note Veo que se llama "Porygon Engine", asegúrate de cambiarlo si tu proyecto actual es "HeliosEngine".
     */
    std::string m_windowName = "Porygon Engine";
};