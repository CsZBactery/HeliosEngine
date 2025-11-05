#pragma once
#include "Prerequisites.h"
#include "MeshComponent.h"

/**
 * @class ModelLoader
 * @brief Cargador de modelos 3D sencillo para OBJ (vértices/índices/UVs).
 *
 * Usa la librería OBJ_Loader (header-only) para leer archivos .obj
 * y vuelca el contenido en un MeshComponent del engine.
 *
 * @note Soporta UVs si existen en el .obj. El triangulado
 *       se recomienda hacerlo al exportar (Blender).
 */
class ModelLoader {
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
     * @brief Carga un modelo OBJ desde disco y lo convierte a MeshComponent.
     *
     * @param objPath Ruta (relativa o absoluta) al archivo .obj (p.ej. "Assets/Moto/repsol3.obj").
     * @param outMesh Referencia a MeshComponent donde se escribirán vértices e índices.
     * @param flipV Si es true, invierte la coordenada V de las UVs (útil cuando la textura se ve invertida).
     * @return true si la carga tuvo éxito y se llenó el mesh; false en caso contrario.
     */
    bool LoadOBJ(const std::string& objPath, MeshComponent& outMesh, bool flipV = false);
};
