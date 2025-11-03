#pragma once
#include "Prerequisites.h"

class
	Device;

class
	DeviceContext;

class
	Texture;

/**
 * @brief Encapsula una vista de profundidad y stencil en DirectX.
 *
 * Esta clase administra la creaci�n, actualizaci�n, renderizado
 * y destrucci�n de un recurso DepthStencilView para el pipeline gr�fico.
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
	 * @param device Referencia al dispositivo de DirectX.
	 * @param depthStencil Textura que servir� como buffer de profundidad/stencil.
	 * @param format Formato DXGI usado para la vista.
	 * @return HRESULT C�digo de estado de la operaci�n (S_OK si fue exitosa).
	 */
	HRESULT
		init(Device& device, Texture& depthStencil, DXGI_FORMAT format);

	/**
	 * @brief Actualiza el estado interno de la vista.
	 *
	 * Actualmente no realiza ninguna operaci�n.
	 */
	void
		update() {};

	/**
	 * @brief Renderiza usando la vista de profundidad y stencil.
	 *
	 * @param deviceContext Contexto del dispositivo de DirectX.
	 */
	void
		render(DeviceContext& deviceContext);

	/**
	 * @brief Libera los recursos asociados al DepthStencilView.
	 */
	void
		destroy();

public:
	ID3D11DepthStencilView* m_depthStencilView = nullptr; /**< Puntero al recurso de DepthStencilView de DirectX. */

};