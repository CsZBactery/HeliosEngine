/**
 * @file Mesh.h
 * @brief Definición de la estructura Submesh y la clase Mesh para la gestión de geometría en el motor.
 */

#pragma once
#include "Prerequisites.h"
#include "Buffer.h"

 /**
  * @struct Submesh
  * @brief Representa una sección de una malla que utiliza un único material.
  *
  * Contiene los buffers de vértices e índices específicos, permitiendo que una malla compleja
  * se divida en partes renderizables individualmente.
  */
struct
	Submesh {
	Buffer vertexBuffer;      ///< Buffer que contiene los datos de los vértices (VBO).
	Buffer indexBuffer;       ///< Buffer que contiene los índices (IBO).
	unsigned int indexCount = 0;   ///< Número total de índices a dibujar en esta submalla.
	unsigned int startIndex = 0;   ///< Desplazamiento inicial dentro del buffer de índices.
	unsigned int materialSlot = 0; ///< Índice del slot de material asociado a esta parte de la geometría.
};

/**
 * @class Mesh
 * @brief Clase contenedora que agrupa múltiples submallas para representar un modelo 3D completo.
 */
class
	Mesh {
public:
	/**
	 * @brief Obtiene la lista de submallas que componen la malla.
	 * @return Referencia al vector de objetos Submesh.
	 */
	std::vector<Submesh>& getSubmeshes() { return m_submeshes; }

	/**
	 * @brief Obtiene la lista de submallas (versión constante).
	 * @return Referencia constante al vector de objetos Submesh.
	 */
	const std::vector<Submesh>& getSubmeshes() const { return m_submeshes; }

	/**
	 * @brief Libera los recursos de hardware (buffers) de todas las submallas.
	 *
	 * Itera por cada submalla para destruir sus respectivos buffers de GPU y limpia el contenedor interno.
	 */
	void
		destroy() {
		for (Submesh& submesh : m_submeshes) {
			submesh.vertexBuffer.destroy();
			submesh.indexBuffer.destroy();
		}
		m_submeshes.clear();
	}

private:
	std::vector<Submesh> m_submeshes; ///< Contenedor interno de las partes geométricas (submallas) del modelo.
};