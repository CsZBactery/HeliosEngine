/**
 * @file Model3D.h
 * @brief Clase para la carga y gestión de modelos 3D en formatos OBJ y FBX.
 */

#pragma once
#include "Prerequisites.h"
#include "IResource.h"
#include "MeshComponent.h"
#include <fbxsdk.h>

 /**
  * @enum ModelType
  * @brief Define los formatos de archivo de modelos 3D soportados por el motor.
  */
enum ModelType {
    OBJ, /**< Formato Wavefront OBJ (Vértices, UVs y Normales simples). */
    FBX  /**< Formato Autodesk FBX (Soporta jerarquías, materiales complejos y animaciones). */
};

/**
 * @class Model3D
 * @brief Recurso que representa un modelo tridimensional compuesto por una o varias mallas.
 * @details Hereda de IResource para integrarse en el sistema de gestión de recursos.
 * Utiliza el SDK de Autodesk para procesar archivos FBX y extraer geometría y materiales.
 */
class Model3D : public IResource {
public:
    /**
     * @brief Constructor de la clase Model3D.
     * @param name Nombre o ruta del recurso.
     * @param modelType Tipo de modelo a cargar (OBJ o FBX).
     */
    Model3D(const std::string& name, ModelType modelType)
        : IResource(name), m_modelType(modelType), lSdkManager(nullptr), lScene(nullptr) {
        SetType(ResourceType::Model3D);
        load(name);
    }

    /**
     * @brief Destructor de la clase. Se encarga de liberar los recursos del SDK de FBX si fueron inicializados.
     */
    ~Model3D();

    /**
     * @brief Carga los datos del modelo desde el disco.
     * @param path Ruta del archivo del modelo.
     * @return true si la carga fue exitosa, false en caso contrario.
     */
    bool load(const std::string& path) override;

    /**
     * @brief Inicializa los recursos del modelo (preparación para renderizado).
     * @return true si la inicialización fue correcta.
     */
    bool init() override;

    /**
     * @brief Libera la memoria ocupada por las mallas y el SDK.
     */
    void unload() override;

    /**
     * @brief Obtiene el tamaño aproximado del modelo en bytes.
     * @return Tamaño en bytes basado en la cantidad de vértices e índices.
     */
    size_t getSizeInBytes() const override;

    /**
     * @brief Obtiene el contenedor de todas las mallas que conforman el modelo.
     * @return Referencia constante al vector de MeshComponent.
     */
    const std::vector<MeshComponent>& GetMeshes() const { return m_meshes; }

    // Métodos específicos de FBX

    /**
     * @brief Inicializa el administrador de memoria y el sistema de IO del SDK de Autodesk FBX.
     * @return true si el manager se inicializó correctamente.
     */
    bool InitializeFBXManager();

    /**
     * @brief Carga un archivo FBX utilizando el sistema de importación del SDK.
     * @param filePath Ruta completa al archivo .fbx.
     */
    void LoadFBXModel(const std::string& filePath);

    /**
     * @brief Recorre recursivamente la jerarquía de nodos del archivo FBX.
     * @param node Puntero al nodo actual a procesar.
     */
    void ProcessFBXNode(FbxNode* node);

    /**
     * @brief Extrae la geometría (vértices, normales, UVs) de un nodo FBX de tipo Mesh.
     * @param node Nodo que contiene el componente de malla.
     */
    void ProcessFBXMesh(FbxNode* node);

    /**
     * @brief Extrae las propiedades de materiales y rutas de texturas de un material FBX.
     * @param material Puntero al material del SDK de FBX.
     */
    void ProcessFBXMaterials(FbxSurfaceMaterial* material);

    /**
     * @brief Obtiene la lista de nombres de archivos de textura requeridos por el modelo.
     * @return Vector de strings con las rutas/nombres de las texturas.
     */
    std::vector<std::string> GetTextureFileNames() const { return textureFileNames; }

private:
    FbxManager* lSdkManager; /**< Administrador global de memoria del SDK de FBX. */
    FbxScene* lScene;       /**< Objeto escena que contiene toda la información del archivo importado. */
    std::vector<std::string> textureFileNames; /**< Almacén temporal de nombres de texturas extraídas del modelo. */

public:
    ModelType m_modelType; /**< Almacena el formato de origen del modelo actual. */
    std::vector<MeshComponent> m_meshes; /**< Lista de sub-mallas procesadas y listas para el motor. */
};