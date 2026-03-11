#pragma once
#include "Prerequisites.h"

/**
 * @file RasterizerState.h
 * @brief Gestión de la etapa de rasterización y configuración de visualización de polígonos.
 */

class Device;
class DeviceContext;

/**
 * @class RasterizerState
 * @brief Encapsula un @c ID3D11RasterizerState para configurar cómo se transforman primitivas en píxeles.
 * * En HeliosEngine, esta clase controla:
 * 1. Modo de Relleno: Permite alternar entre Solid (normal) y Wireframe (mallas de alambre).
 * 2. Culling: Optimiza el rendimiento descartando caras ocultas (Back, Front o None).
 * 3. Clipping: Gestión del recorte de profundidad basándose en el View Frustum.
 */
class
	RasterizerState {
public:
	/** @brief Constructor por defecto. */
	RasterizerState() = default;

	/** @brief Destructor por defecto. Se debe liberar con destroy(). */
	~RasterizerState() = default;

	/**
	 * @brief Inicializa el Rasterizer State con la configuración por defecto del motor.
	 * * Usualmente configura D3D11_FILL_SOLID y D3D11_CULL_BACK.
	 * @param device Dispositivo con el que se creará el recurso en la GPU.
	 * @return HRESULT S_OK si la creación fue exitosa.
	 */
	HRESULT
		init(Device device);

	/**
	 * @brief Inicializa el Rasterizer State con parámetros técnicos específicos.
	 * * Útil para estados especiales como el del Skybox (Cull None) o Debug (Wireframe).
	 * * @param device Referencia al dispositivo físico.
	 * @param fill Modo de relleno (Solid/Wireframe).
	 * @param cull Modo de descarte de caras (None/Front/Back).
	 * @param frontCCW Si es true, el sentido antihorario define la cara frontal.
	 * @param depthClip Habilita o deshabilita el recorte por profundidad.
	 */
	HRESULT
		init(Device& device,
			D3D11_FILL_MODE fill,
			D3D11_CULL_MODE cull,
			bool frontCCW,
			bool depthClip);

	/** @brief Actualización dinámica de parámetros (Placeholder). */
	void
		update();

	/**
	 * @brief Aplica la configuración de este estado al pipeline activo.
	 * * Llama a RSSetState en el contexto del dispositivo.
	 * @param deviceContext Contexto encargado de enviar los comandos a la GPU.
	 */
	void
		render(DeviceContext& deviceContext);

	/**
	 * @brief Libera el recurso ID3D11RasterizerState y limpia el puntero.
	 */
	void
		destroy();

private:
	/** @brief Interfaz de estado de rasterización nativa de Direct3D 11. */
	ID3D11RasterizerState* m_rasterizerState = nullptr;
};