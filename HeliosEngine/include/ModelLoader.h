// --- En ModelLoader.h ---

#pragma once
#include "MeshComponent.h"
                         

/**
 * @class ModelLoader
 * @brief Parser de OBJ funcional y robusto.
 */
class ModelLoader
{
public:
    ModelLoader() = default;
    ~ModelLoader() = default;

    /**
     * @brief Carga un modelo .obj desde un archivo.
     * @param objPath Ruta al archivo .obj.
     * @param outMesh La malla de salida que se llenará.
     * @param flipV Invierte la coordenada V de la textura (muy común).
     * @return true si la carga fue exitosa, false si no.
     */
    bool LoadOBJ(const std::string& objPath, MeshComponent& outMesh, bool flipV);
};