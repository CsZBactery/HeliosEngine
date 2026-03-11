#pragma once
#include "Prerequisites.h"

/**
 * @file DepthStencilView.h
 * @brief Gestión del Buffer de Profundidad (Z-Buffer) y Plantilla (Stencil) para HeliosEngine.
 */

class Device;
class DeviceContext;
class Texture;

/**
 * @class DepthStencilView
 * @brief Encapsula la vista de Profundidad y Plantilla (Depth-Stencil View) en DirectX 11.
 * * Esta clase administra la interfaz ID3D11DepthStencilView, esencial para:
 * 1. Depth Test (Z-Buffering): Determina la visibilidad de los píxeles basándose en su distancia
 * a la cámara, evitando que objetos lejanos se dibujen sobre los cercanos.
 * 2. Stencil Test: Permite realizar máscaras de dibujado para efectos como espejos o portales.
 */
class
	DepthStencilView {
public:
	/** @brief Constructor por defecto. No reserva memoria en GPU. */
	DepthStencilView() = default;

	/** @brief Destructor por defecto. Se debe liberar manualmente con destroy(). */
	~DepthStencilView() = default;

	/**
	 * @brief Inicializa la vista de profundidad vinculándola a una textura.
	 * * @param device Dispositivo DirectX para la creación del recurso.
	 * @param depthStencil Textura que servirá como almacén de datos (debe tener bind de DepthStencil).
	 * @param format Formato de datos (ej: DXGI_FORMAT_D24_UNORM_S8_UINT).
	 * @return HRESULT S_OK si la operación fue exitosa.
	 */
	HRESULT
		init(Device& device, Texture& depthStencil, DXGI_FORMAT format);

	/**
	 * @brief Inicialización avanzada con especificación de dimensión.
	 * * Útil para configuraciones específicas de hardware o efectos de post-procesado.
	 * * @param device Dispositivo DirectX.
	 * @param depthStencil Textura de origen.
	 * @param format Formato de la vista.
	 * @param viewDimension Define cómo el shader accede a la textura (1D, 2D, MS, etc.).
	 */
	HRESULT
		init(Device& device,
			Texture& depthStencil,
			DXGI_FORMAT format,
			D3D11_DSV_DIMENSION viewDimension);

	/** @brief Actualización lógica de la vista (Placeholder). */
	void
		update() {};

	/**
	 * @brief Aplica o limpia la vista de profundidad en el pipeline activo.
	 * * En HeliosEngine, este método se encarga de preparar el buffer para un nuevo frame.
	 * @param deviceContext Contexto donde se emiten los comandos de renderizado.
	 */
	void
		render(DeviceContext& deviceContext);

	/**
	 * @brief Libera el recurso ID3D11DepthStencilView de la GPU.
	 */
	void
		destroy();

public:
	/** @brief Puntero nativo a la interfaz de vista de profundidad de Direct3D 11. */
	ID3D11DepthStencilView* m_depthStencilView = nullptr;
};