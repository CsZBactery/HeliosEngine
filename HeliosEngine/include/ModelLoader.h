#pragma once
#include "Prerequisites.h"

/**
 * @file ModelLoader.h
 * @brief Declaración de la clase @c ModelLoader para gestionar el ciclo de vida y carga de modelos 3D.
 * @details Provee los puntos de entrada típicos de un subsistema de carga/render (init, update, render, destroy)
 *          y deja preparado (comentado) un método de carga de archivos OBJ para futura integración.
 * @version 1.0
 * @date 2025-11-04
 */

 /**
  * @class ModelLoader
  * @brief Clase responsable de la carga, actualización, renderizado y destrucción de modelos 3D.
  *
  * Permite importar datos desde archivos externos (p. ej., OBJ) mediante un cargador que se encuentra
  * actualmente comentado y listo para habilitarse cuando se requiera. La clase define un ciclo de vida
  * simple (init/update/render/destroy) para integrarse con el motor o aplicación.
  */
class
	ModelLoader
{
public:

	/**
	 * @brief Constructor por defecto.
	 */
	ModelLoader() = default;

	/**
	 * @brief Destructor por defecto.
	 */
	~ModelLoader() = default;

	/**
	 * @brief Inicializa cualquier recurso necesario previo a la carga de modelos.
	 * @note No realiza operaciones de E/S; úsese para reservar estructuras internas o estados.
	 */
	void
		init();

	/**
	 * @brief Actualiza el estado interno del cargador (si aplica).
	 * @note Puede emplearse para tareas diferidas, recarga en caliente o streaming.
	 */
	void
		update();

	/**
	 * @brief Solicita el render del/los modelo(s) cargados (si existe implementación).
	 * @note La implementación concreta depende del pipeline de render externo.
	 */
	void
		render();

	/**
	 * @brief Libera recursos asociados al cargador y a los modelos administrados.
	 * @warning Debe llamarse antes de finalizar la aplicación para evitar fugas de memoria.
	 */
	void
		destroy();

	/**
	 * @brief Carga un archivo de modelo 3D (p. ej., formato OBJ) y devuelve su información.
	 * @param objFileName Ruta o nombre del archivo OBJ a cargar.
	 * @return Estructura @c LoadData con los datos del modelo cargado.
	 * @note Método actualmente deshabilitado (comentado). Activarlo cuando el cargador OBJ esté integrado.
	 */
	 /*LoadData
		 Load(std::string objFileName);*/

private:
	/**
	 * @brief Cargador OBJ (comentado actualmente). Puede habilitarse para importar geometrías.
	 * @details Mantener sincronizado con la estructura @c LoadData y los tipos de vértice del motor.
	 */
	 //objl::Loader m_loader;
};
