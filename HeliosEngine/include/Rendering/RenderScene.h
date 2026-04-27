/**
 * @file RenderScene.h
 * @brief Definición de la clase RenderScene para organizar los datos de renderizado por frame.
 */

#pragma once
#include "Prerequisites.h"
#include "Rendering/RenderTypes.h"

class Skybox;

/**
 * @class RenderScene
 * @brief Clase contenedora que agrupa los objetos, luces y entorno visibles para el pipeline.
 *
 * Esta estructura actúa como un puente de datos, permitiendo que el motor recolecte todo lo
 * necesario antes de iniciar los pases de renderizado (Forward o Deferred).
 */
class
	RenderScene {
public:
	/**
	 * @brief Limpia todos los vectores de objetos y luces.
	 * Se debe invocar al inicio de cada frame para asegurar que no se acumule basura de frames anteriores.
	 */
	void clear();

public:
	/** @brief Colección de objetos opacos recolectados para el frame actual. */
	std::vector<RenderObject> opaqueObjects;

	/** @brief Colección de objetos que requieren procesamiento de transparencia y ordenamiento. */
	std::vector<RenderObject> transparentObjects;

	/** @brief Lista de luces direccionales activas que afectan a la iluminación global. */
	std::vector<LightData> directionalLights;

	/** @brief Puntero al componente de Skybox encargado del renderizado del fondo. */
	Skybox* skybox = nullptr;
};