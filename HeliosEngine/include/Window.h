#pragma once
#include "../include/Prerequisites.h"

/**
 * @file Window.h
 * @brief Declaración de la clase Window y su interfaz pública.
 *
 * @details
 * Representa y administra una ventana Win32 que servirá como superficie de
 * renderizado para DirectX. Expone métodos de ciclo de vida (init/update/render/destroy)
 * y mantiene los handles y dimensiones necesarios.
 *
 * @since 1.0
 */

 /**
  * @class Window
  * @brief Representa una ventana de aplicación en Windows.
  *
  * Esta clase encapsula la creación, gestión, actualización y destrucción
  * de una ventana Win32, utilizada como superficie de renderizado para DirectX.
  */
class
	Window {
public:
	/**
	 * @brief Constructor por defecto.
	 *
	 * No realiza inicialización de la ventana; sólo construye el objeto.
	 */
	Window() = default;

	/**
	 * @brief Destructor por defecto.
	 *
	 * @note No destruye la ventana automáticamente. Llama a ::destroy()
	 *       explícitamente si necesitas liberar recursos antes.
	 */
	~Window() = default;

	/**
	 * @brief Inicializa y crea la ventana de la aplicación.
	 *
	 * Registra la clase de ventana (si aplica), crea el HWND y muestra
	 * la ventana según @p nCmdShow.
	 *
	 * @param hInstance Manejador de la instancia de la aplicación.
	 * @param nCmdShow Parámetro que indica cómo se mostrará la ventana
	 *        (valores típicos: @c SW_SHOW, @c SW_SHOWDEFAULT, etc.).
	 * @param wndproc Función de procedimiento de ventana (callback de mensajes).
	 * @return HRESULT Código de resultado:
	 *         - @c S_OK si se creó correctamente.
	 *         - @c E_FAIL u otro código de error en fallos de creación/registro.
	 *
	 * @post Si el resultado es @c S_OK, @ref m_hWnd será válido y
	 *       @ref m_width y @ref m_height reflejarán el tamaño inicial.
	 */
	HRESULT
		init(HINSTANCE hInstance, int nCmdShow, WNDPROC wndproc);

	/**
	 * @brief Actualiza el estado de la ventana.
	 *
	 * Normalmente procesa eventos o lógica asociada al ciclo de vida de la ventana
	 * (por ejemplo, despacho de mensajes).
	 */
	void
		update();

	/**
	 * @brief Renderiza el contenido de la ventana.
	 *
	 * Generalmente se usa junto con el contexto gráfico (DirectX/OpenGL).
	 * Puede invocar el render del motor o preparar el back buffer.
	 */
	void
		render();

	/**
	 * @brief Libera los recursos y destruye la ventana.
	 *
	 * Cierra el @ref m_hWnd si está creado y limpia los miembros asociados.
	 * Es seguro llamar múltiples veces (idempotente).
	 */
	void
		destroy();

public:
	/**
	 * @brief Handle de la ventana Win32.
	 *
	 * @note Será @c nullptr hasta que @ref init tenga éxito.
	 */
	HWND m_hWnd = nullptr;

	/**
	 * @brief Ancho actual de la ventana (en píxeles).
	 *
	 * @warning El valor puede cambiar tras eventos de redimensionado.
	 */
	unsigned int m_width;

	/**
	 * @brief Alto actual de la ventana (en píxeles).
	 *
	 * @warning El valor puede cambiar tras eventos de redimensionado.
	 */
	unsigned int m_height;

private:
	/**
	 * @brief Handle de la instancia de la aplicación.
	 */
	HINSTANCE m_hInst = nullptr;

	/**
	 * @brief Rectángulo que define las dimensiones de la ventana.
	 *
	 * Usado durante la creación y para consultas de tamaño del área cliente.
	 */
	RECT m_rect;

	/**
	 * @brief Nombre de la ventana (por defecto "Helios Engine").
	 *
	 * Puede usarse como título al crear/actualizar la ventana.
	 */
	std::string m_windowName = "Helios Engine";
};
