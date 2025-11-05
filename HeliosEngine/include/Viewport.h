#pragma once
#include "Prerequisites.h"

class
	Window;

class
	DeviceContext;

/**
 * @class Viewport
 * @brief Encapsula la configuración de un @c D3D11_VIEWPORT y su aplicación al pipeline.
 *
 * Provee inicialización a partir de una ventana o dimensiones explícitas, así como
 * métodos de ciclo de vida (update/render/destroy). El viewport define el área de
 * la salida donde se rasteriza la escena.
 */
class
	Viewport {
public:

	/**
	 * @brief Constructor por defecto.
	 */
	Viewport() = default;

	/**
	 * @brief Destructor por defecto.
	 */
	~Viewport() = default;

	/**
	 * @brief Inicializa el viewport usando las dimensiones de una ventana.
	 *
	 * @param window Referencia a la ventana desde la que se tomarán @c width y @c height.
	 * @return @c S_OK si fue exitoso; @c E_POINTER/@c E_INVALIDARG en caso de error.
	 */
	HRESULT
		init(const Window& window);

	/**
	 * @brief Inicializa el viewport con dimensiones explícitas.
	 *
	 * @param width  Ancho del viewport en píxeles.
	 * @param height Alto del viewport en píxeles.
	 * @return @c S_OK si fue exitoso; @c E_INVALIDARG si alguna dimensión es 0.
	 */
	HRESULT
		init(unsigned int width, unsigned int height);

	/**
	 * @brief Actualiza el estado del viewport.
	 *
	 * Método placeholder para lógica futura; actualmente no realiza cambios.
	 */
	void
		update();

	/**
	 * @brief Aplica el viewport al rasterizador del @c DeviceContext.
	 *
	 * Llama internamente a @c RSSetViewports con el @c D3D11_VIEWPORT almacenado.
	 *
	 * @param deviceContext Contexto de dispositivo donde se establecerá el viewport.
	 */
	void
		render(DeviceContext& deviceContext);

	/**
	 * @brief Libera o restablece recursos asociados.
	 *
	 * Placeholder: actualmente no hay recursos dinámicos que liberar.
	 */
	void
		destroy() {}

public:
	/**
	 * @brief Estructura nativa de Direct3D que describe el viewport activo.
	 */
	D3D11_VIEWPORT m_viewport;
};
