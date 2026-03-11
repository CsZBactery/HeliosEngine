#pragma once
#include "Prerequisites.h"
#include "ECS\Component.h"

/**
 * @file MeshComponent.h
 * @brief Componente ECS encargado de almacenar la geometría de los objetos en HeliosEngine.
 */

class DeviceContext;

/**
 * @class MeshComponent
 * @brief Componente que encapsula la información de geometría (vértices e índices) de un Actor.
 * * Forma parte de la arquitectura ECS y es fundamental para el proceso de renderizado.
 * Contiene los datos en crudo que luego son cargados en los Buffers de DirectX 11.
 * * La malla soporta:
 * - Vértices estándar (SimpleVertex) para modelos como tu Xbox.
 * - Vértices optimizados (SkyboxVertex) para el entorno.
 * - Índices para el ensamblado de triángulos.
 */
class
	MeshComponent : public Component {
public:
	/**
	 * @brief Constructor por defecto.
	 * * Inicializa contadores y registra el componente como tipo MESH.
	 */
	MeshComponent() : m_numVertex(0), m_numIndex(0), Component(ComponentType::MESH) {}

	/**
	 * @brief Destructor virtual por defecto.
	 */
	virtual
		~MeshComponent() = default;

	/**
	 * @brief Inicialización de lógica de malla (heredado de Component).
	 */
	void
		init() override {};

	/**
	 * @brief Actualización de datos de geometría en tiempo real.
	 * @param deltaTime Tiempo transcurrido desde el último frame.
	 */
	void
		update(float deltaTime) override {};

	/**
	 * @brief Preparación de la malla para ser enviada al pipeline de renderizado.
	 * @param deviceContext Contexto de dispositivo para operaciones gráficas.
	 */
	void
		render(DeviceContext& deviceContext) override {};

	/**
	 * @brief Limpieza de recursos y vectores de memoria.
	 */
	void
		destroy() override {};

public:
	/** @brief Nombre identificador de la sub-malla. */
	std::string m_name;

	/** @brief Contenedor de vértices estándar (Posición, Normal, UV). */
	std::vector<SimpleVertex> m_vertex;

	/** @brief Contenedor de vértices simplificados para el Skybox. */
	std::vector<SkyboxVertex> m_skyVertex;

	/** @brief Lista de índices que define el orden de dibujo (Topology). */
	std::vector<unsigned int> m_index;

	/** @brief Contador total de vértices en el sistema. */
	int m_numVertex;

	/** @brief Contador total de índices para DrawIndexed. */
	int m_numIndex;
};