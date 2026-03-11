#pragma once
#include "Prerequisites.h"

/**
 * @file Window.h
 * @brief Gestión de la ventana nativa de Windows para HeliosEngine.
 */

class BaseApp;

/**
 * @class Window
 * @brief Clase encargada de la creación, gestión y ciclo de vida de la ventana Win32.
 * * Proporciona el HWND (Window Handle) necesario para que DirectX 11 pueda
 * vincular el SwapChain y realizar el renderizado en el área cliente.
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
	 * @brief Inicializa y registra la clase de ventana en el sistema operativo.
	 * * @param hInstance Instancia de la aplicación proporcionada por el sistema.
	 * @param nCmdShow Estado de visualización inicial de la ventana.
	 * @param wndproc Puntero a la función de procedimiento (Callback) para mensajes.
	 * @param app Puntero a la aplicación base para vinculación de lógica interna.
	 * @return HRESULT S_OK si la creación fue exitosa, código de error en caso contrario.
	 */
	HRESULT
		init(HINSTANCE hInstance, int nCmdShow, WNDPROC wndproc, BaseApp* app);

	/**
	 * @brief Procesa la cola de mensajes de Windows para mantener la ventana activa.
	 */
	void
		update();

	/**
	 * @brief Punto de entrada para el dibujado de la ventana a nivel de OS.
	 */
	void
		render();

	/**
	 * @brief Libera los recursos de la ventana y la destruye en el sistema.
	 */
	void
		destroy();

public:
	/** @brief Identificador único de la ventana (Window Handle). */
	HWND m_hWnd = nullptr;

	/** @brief Ancho de la ventana en píxeles. */
	unsigned int m_width;

	/** @brief Alto de la ventana en píxeles. */
	unsigned int m_height;

private:
	/** @brief Instancia de la aplicación (HINSTANCE). */
	HINSTANCE m_hInst = nullptr;

	/** @brief Estructura de dimensiones de la ventana (Rect). */
	RECT m_rect;

	/** @brief Nombre identificador del motor en la barra de título. */
	std::string m_windowName = "HeliosEngine";
};