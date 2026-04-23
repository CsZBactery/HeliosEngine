/**
 * @file RenderScene.h
 * @brief Definición de la clase RenderScene, encargada de agrupar los datos de una escena para el renderizado.
 */

#pragma once
#include "Prerequisites.h"
#include "Rendering/RenderTypes.h"

class Skybox;

/**
 * @class RenderScene
 * @brief Contenedor de datos que representa una captura lógica de la escena en un frame.
 *
 * Esta clase actúa como un "puente" entre el grafo de escena y el RenderPipeline,
 * almacenando listas de objetos clasificados por tipo y datos de iluminación.
 */
class
	RenderScene {
public:
	/**
	 * @brief Limpia todos los contenedores y reinicia los punteros de la escena.
	 *
	 * Se debe llamar al inicio de cada frame para asegurar que la información no se acumule.
	 */
	void clear();

public:
	/** @brief Lista de objetos opacos para el renderizado (sin ordenamiento de profundidad estricto). */
	std::vector<RenderObject> opaqueObjects;

	/** @brief Lista de objetos transparentes (requieren ordenamiento de atrás hacia adelante). */
	std::vector<RenderObject> transparentObjects;

	/** @brief Colección de luces direccionales activas en la escena. */
	std::vector<LightData> directionalLights;

	/** @brief Puntero al componente de Skybox (fondo de la escena). */
	Skybox* skybox = nullptr;
};