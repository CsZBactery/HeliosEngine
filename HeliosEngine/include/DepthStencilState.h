/**
 * @file DepthStencilState.h
 * @brief Clase que define las reglas lógicas para las pruebas de Profundidad (Z-Buffer) y Plantilla (Stencil).
 */

#pragma once
#include "Prerequisites.h"

class Device;
class DeviceContext;

/**
 * @class DepthStencilState
 * @brief Encapsula un objeto ID3D11DepthStencilState de DirectX 11.
 *
 * @details A diferencia del DepthStencilView (que es la memoria física donde se guarda la profundidad),
 * esta clase representa las "Reglas" del pipeline en la etapa Output-Merger.
 * Administra si la prueba de profundidad está activa, cómo se comparan los píxeles (ej. si están más cerca o lejos),
 * y qué hacer con las máscaras de estarcido (Stencil).
 */
class
	DepthStencilState {
public:
	/**
	 * @brief Constructor por defecto. No inicializa el recurso en GPU.
	 */
	DepthStencilState() = default;

	/**
	 * @brief Destructor.
	 * @warning No libera la memoria automáticamente; se debe llamar a destroy().
	 */
	~DepthStencilState() = default;

	/**
	 * @brief Crea y configura el estado lógico de profundidad y estarcido.
	 *
	 * @details Construye el ID3D11DepthStencilState definiendo si los objetos deben
	 * ocultarse unos a otros (Depth) y si se deben usar máscaras especiales (Stencil).
	 *
	 * @param device Dispositivo de hardware encargado de crear el recurso.
	 * @param enableDepth Si es true, activa el Z-Buffer (los objetos cercanos tapan a los lejanos).
	 * @param enableStencil Si es true, activa pruebas avanzadas de enmascarado.
	 * @return S_OK si el estado se configuró correctamente en la GPU.
	 */
	HRESULT
		init(Device& device, bool enableDepth = true, bool enableStencil = false);

	/**
	 * @brief Método para actualizar configuraciones del estado en tiempo real.
	 * @note Actualmente actúa como un placeholder (espacio reservado) para uso futuro.
	 */
	void
		update();

	/**
	 * @brief Inyecta estas reglas en el pipeline de renderizado actual.
	 *
	 * @details Llama a OMSetDepthStencilState. Todos los objetos dibujados después de llamar
	 * a este método obedecerán las reglas de profundidad/stencil definidas aquí.
	 *
	 * @param deviceContext Contexto del dispositivo que emite la orden de renderizado.
	 * @param stencilRef Valor de referencia numérico usado si el Stencil está activado (por defecto 0).
	 * @param reset Si es true, limpia el estado actual del pipeline dejándolo en null.
	 */
	void
		render(DeviceContext& deviceContext, unsigned int stencilRef = 0, bool reset = false);

	/**
	 * @brief Libera la memoria del objeto de estado COM en la tarjeta de video.
	 * @post m_depthStencilState vuelve a ser nullptr.
	 */
	void
		destroy();

private:
	/**
	 * @brief Puntero al recurso de estado lógico en DirectX 11.
	 */
	ID3D11DepthStencilState* m_depthStencilState = nullptr;
};