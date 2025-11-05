#pragma once
#include "Prerequisites.h"

class
	Device;

class
	DeviceContext;

class
	Texture;

/**
 * @file DepthStencilView.h
 * @brief Declaración de la clase que encapsula una vista de profundidad/stencil en DirectX 11.
 *
 * Provee una interfaz ligera para crear, usar (limpieza por frame) y liberar un
 * ID3D11DepthStencilView asociado a una textura de profundidad/stencil. Pensada para
 * integrarse en el pipeline gráfico junto con un Render Target View.
 */

 /**
  * @class DepthStencilView
  * @brief Encapsula una vista de profundidad y stencil en DirectX.
  *
  * Esta clase administra la creación, actualización, renderizado y destrucción de un
  * recurso DepthStencilView para el pipeline gráfico. La inicialización elige
  * automáticamente la dimensión correcta (TEXTURE2D o TEXTURE2DMS) en función
  * de la configuración de multisampling de la textura subyacente.
  *
  * @note El método render realiza un clear del depth/stencil cada frame.
  * @see Device, DeviceContext, Texture
  */
class
	DepthStencilView {
public:

	/**
	 * @brief Constructor por defecto.
	 */
	DepthStencilView() = default;

	/**
	 * @brief Destructor por defecto.
	 */
	~DepthStencilView() = default;

	/**
	 * @brief Inicializa la vista de profundidad y stencil.
	 *
	 * Crea un ID3D11DepthStencilView sobre la textura provista, seleccionando
	 * la ViewDimension adecuada según el recuento de muestras (MSAA).
	 *
	 * @param device Referencia al dispositivo de DirectX.
	 * @param depthStencil Textura que servirá como buffer de profundidad/stencil.
	 * @param format Formato DXGI usado para la vista (p. ej. DXGI_FORMAT_D24_UNORM_S8_UINT).
	 * @return HRESULT Código de estado de la operación (S_OK si fue exitosa).
	 */
	HRESULT
		init(Device& device, Texture& depthStencil, DXGI_FORMAT format);

	/**
	 * @brief Actualiza el estado interno de la vista.
	 *
	 * Actualmente no realiza ninguna operación; se deja para futura extensión.
	 */
	void
		update() {};

	/**
	 * @brief Renderiza usando la vista de profundidad y stencil.
	 *
	 * Limpia el depth-stencil al inicio del frame (ClearDepthStencilView) con
	 * profundidad 1.0f y stencil 0.
	 *
	 * @param deviceContext Contexto del dispositivo de DirectX.
	 */
	void
		render(DeviceContext& deviceContext);

	/**
	 * @brief Libera los recursos asociados al DepthStencilView.
	 *
	 * Llama a SAFE_RELEASE sobre el recurso subyacente.
	 */
	void
		destroy();

public:
	ID3D11DepthStencilView* m_depthStencilView = nullptr; /**< Puntero al recurso de DepthStencilView de DirectX. */
};
