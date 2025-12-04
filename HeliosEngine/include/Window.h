#pragma once
#include "Prerequisites.h"

/**
 * @class Window
 * @brief Representa una ventana de aplicaci�n en Windows.
 *
 * Esta clase encapsula la creaci�n, gesti�n, actualizaci�n y destrucci�n
 * de una ventana Win32, utilizada como superficie de renderizado para DirectX.
 */
class
	Window {
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
	 * @brief Inicializa y crea la ventana de la aplicaci�n.
	 *
	 * @param hInstance Manejador de la instancia de la aplicaci�n.
	 * @param nCmdShow Par�metro que indica c�mo se mostrar� la ventana.
	 * @param wndproc Funci�n de procedimiento de ventana (callback de mensajes).
	 * @return HRESULT C�digo de resultado (S_OK si se cre� correctamente).
	 */
	HRESULT
		init(HINSTANCE hInstance, int nCmdShow, WNDPROC wndproc);

	/**
	 * @brief Actualiza el estado de la ventana.
	 *
	 * Normalmente procesa eventos o l�gica asociada al ciclo de vida de la ventana.
	 */
	void
		update();

	/**
	 * @brief Renderiza el contenido de la ventana.
	 *
	 * Generalmente se usa junto con el contexto gr�fico (DirectX/OpenGL).
	 */
	void
		render();

	/**
	 * @brief Libera los recursos y destruye la ventana.
	 */
	void
		destroy();

public:
	/**
	 * @brief Handle de la ventana Win32.
	 */
	HWND m_hWnd = nullptr;

	/**
	 * @brief Ancho actual de la ventana.
	 */
	unsigned int m_width;

	/**
	 * @brief Alto actual de la ventana.
	 */
	unsigned int m_height;

private:
	/**
	 * @brief Handle de la instancia de la aplicaci�n.
	 */
	HINSTANCE m_hInst = nullptr;

	/**
	 * @brief Rect�ngulo que define las dimensiones de la ventana.
	 */
	RECT m_rect;

	/**
	 * @brief Nombre de la ventana (por defecto "Porygon Engine").
	 */
	std::string m_windowName = "Porygon Engine";
};