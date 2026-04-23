/**
 * @file Mesh.h
 * @brief Definición de las estructuras y clases para la gestión de mallas y submallas.
 */

#pragma once
#include "Prerequisites.h"
#include "Buffer.h"

 /**
  * @struct Submesh
  * @brief Representa una sección de una malla que comparte un mismo material.
  *
  * Contiene sus propios buffers de vértices e índices, así como la información necesaria
  * para realizar la llamada de dibujo (draw call) en la GPU.
  */
struct
	Submesh {
	Buffer vertexBuffer;      /**< Buffer que contiene los datos de los vértices. */
	Buffer indexBuffer;       /**< Buffer que contiene los índices para la topología. */
	unsigned int indexCount = 0;   /**< Cantidad total de índices a dibujar. */
	unsigned int startIndex = 0;   /**< Offset inicial dentro del buffer de índices. */
	unsigned int materialSlot = 0; /**< Índice del material asignado a esta submalla. */
};

/**
 * @class Mesh
 * @brief Clase contenedora que agrupa múltiples submallas para formar un modelo 3D completo.
 */
class
	Mesh {
public:
	/**
	 * @brief Obtiene la lista de submallas de la malla.
	 * @return Referencia al vector de submallas.
	 */
	std::vector<Submesh>& getSubmeshes() { return m_submeshes; }

	/**
	 * @brief Obtiene la lista de submallas de la malla (versión constante).
	 * @return Referencia constante al vector de submallas.
	 */
	const std::vector<Submesh>& getSubmeshes() const { return m_submeshes; }

	/**
	 * @brief Libera los recursos de hardware de todas las submallas.
	 *
	 * Itera sobre cada submalla, destruye sus buffers de GPU y limpia el contenedor.
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
	std::vector<Submesh> m_submeshes; /**< Contenedor interno de submallas. */
};