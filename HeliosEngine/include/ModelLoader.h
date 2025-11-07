#pragma once
#include "Prerequisites.h" // ¡Asegúrate que aquí esté tu SimpleVertex (con Normal)!
#include "MeshComponent.h" // ¡Incluimos la definición de MeshComponent!
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <map>
#include <tuple>

/**
 * @class ModelLoader
 * @brief Clase encargada de gestionar la carga de modelos 3D con un parser OBJ manual.
 * (Adaptado de tu versión "chida")
 */
class
    ModelLoader
{
public:
    ModelLoader() = default;
    ~ModelLoader() = default;

    /**
     * @brief Carga un archivo de modelo 3D (formato OBJ) usando el parser manual.
     * @param objPath Ruta del archivo OBJ a cargar.
     * @param outMesh El objeto MeshComponent que se llenará con los datos.
     * @param flipV Bool para invertir la coordenada V (textura) (true es lo normal).
     * @return true si tuvo éxito, false si falló.
     */
    bool
        LoadOBJ(const std::string& objPath, MeshComponent& outMesh, bool flipV);

private:
    /**
     * @brief Estructura auxiliar para la clave del cache de vértices.
     */
    struct VertexIndices {
        int v, vt, vn;
        // Operador de comparación para usar como clave en std::map
        bool operator<(const VertexIndices& other) const {
            if (v != other.v) return v < other.v;
            if (vt != other.vt) return vt < other.vt;
            return vn < other.vn;
        }
    };
};