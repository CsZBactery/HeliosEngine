#pragma once
#include "Prerequisites.h"

/**
 * @file DepthStencilState.h
 * @brief Gestión de las reglas lógicas para pruebas de Profundidad y Stencil en HeliosEngine.
 */

class Device;
class DeviceContext;

/**
 * @class DepthStencilState
 * @brief Encapsula un objeto ID3D11DepthStencilState de DirectX 11 para la etapa Output-Merger.
 * * A diferencia del DepthStencilView (que es la memoria física), esta clase representa
 * las "Reglas" del pipeline. Administra si la prueba de profundidad está activa,
 * cómo se comparan los píxeles (Z-Test) y cómo operan las máscaras de estarcido (Stencil).
 */
class
	DepthStencilState {
public:
	/** @brief Constructor por defecto. No reserva recursos en GPU. */
	DepthStencilState() = default;

	/** @brief Destructor por defecto. Se requiere llamar a destroy() para liberar recursos COM. */
	~DepthStencilState() = default;

	/**
	 * @brief Crea y configura el estado lógico de profundidad y estarcido.
	 * * @param device Dispositivo de hardware encargado de crear el recurso.
	 * @param depthEnable Si es true, activa el Z-Buffer (los objetos cercanos tapan a los lejanos).
	 * @param writeMask Define si se permite escribir en el Z-Buffer (ALL para objetos sólidos, ZERO para efectos).
	 * @param depthFunc Regla de comparación (ej. D3D11_COMPARISON_LESS para que lo más cercano gane).
	 * @return S_OK si la creación fue exitosa.
	 */
	HRESULT
		init(Device& device,
			bool depthEnable,
			D3D11_DEPTH_WRITE_MASK writeMask,
			D3D11_COMPARISON_FUNC depthFunc);

	/** @brief Método placeholder para futuras actualizaciones dinámicas. */
	void
		update();

	/**
	 * @brief Inyecta estas reglas en el pipeline de renderizado (OMSetDepthStencilState).
	 * * @param deviceContext Contexto de dispositivo que emite la orden.
	 * @param stencilRef Valor de referencia para operaciones de Stencil.
	 * @param reset Si es true, desvincula el estado del pipeline tras el render.
	 */
	void
		render(DeviceContext& deviceContext, unsigned int stencilRef = 0, bool reset = false);

	/**
	 * @brief Libera el recurso ID3D11DepthStencilState en la tarjeta de video.
	 */
	void
		destroy();

private:
	/** @brief Recurso COM de Direct3D 11 para el estado de profundidad/estencil. */
	ID3D11DepthStencilState* m_depthStencilState = nullptr;
};