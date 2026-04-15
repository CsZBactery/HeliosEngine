#pragma once
#include "Prerequisites.h"
#include "IResource.h"
#include "MeshComponent.h"
#include "fbxsdk.h"

/**
 * @file Model3D.h
 * @brief Orquestador de recursos 3D (OBJ/FBX) y geometría generada por HeliosEngine.
 */

 /**
  * @enum ModelType
  * @brief Formatos de archivos 3D compatibles con el motor.
  */
enum ModelType {
    OBJ, /**< Formato Wavefront OBJ para mallas estáticas simples. */
    FBX  /**< Formato Autodesk FBX para mallas complejas y jerarquías. */
};

/**
 * @class Model3D
 * @brief Representa un recurso tridimensional que contiene uno o varios MeshComponents.
 * HeliosEngine usa esta clase para gestionar la importación de datos desde disco
 * y la creación manual de geometrías especiales como el Skybox.
 */
class Model3D : public IResource {
public:
    /**
     * @brief Constructor para carga de archivos (OBJ/FBX).
     * @param name Nombre o ruta del archivo.
     * @param modelType Tipo de formato de origen.
     */
    Model3D(const std::string& name, ModelType modelType)
        : IResource(name), m_modelType(modelType), lSdkManager(nullptr), lScene(nullptr) {
        SetType(ResourceType::Model3D);
    }

    /**
     * @brief Constructor para geometría manual (utilizado para el Skybox infinito).
     * @param name Identificador del recurso.
     * @param vertices Arreglo de vértices espaciales.
     * @param indices Arreglo de índices para el dibujo de caras.
     */
    Model3D(const std::string& name,
        const SkyboxVertex vertices[],
        const unsigned int indices[]) : IResource(name) {
        MeshComponent mesh;
        mesh.m_skyVertex.assign(vertices, vertices + 8);
        mesh.m_index.assign(indices, indices + 36);
        mesh.m_numIndex = mesh.m_index.size(); // Sincronización con m_numIndex para DrawIndexed
        SetType(ResourceType::Model3D);
        m_meshes.push_back(mesh);
    }

    /** @brief Destructor. Modificado para limpieza profunda del SDK de FBX. */
    ~Model3D() override;

    /** @brief Implementación de carga de IResource. */
    bool load(const std::string& path) override;

    /** @brief Prepara los buffers de la GPU para el renderizado. */
    bool init() override;

    /** @brief Libera los recursos de memoria y el SDK. */
    void unload() override;

    /** @brief Calcula el peso en memoria del recurso. */
    size_t getSizeInBytes() const override;

    /** @brief Retorna el listado de sub-mallas procesadas. */
    const std::vector<MeshComponent>& GetMeshes() const { return m_meshes; }

    /* ==================================================================== */
    /* LOADERS Y PARSERS                                                    */
    /* ==================================================================== */

    /** @brief Inicia el sistema de gestión del SDK de FBX. */
    bool InitializeFBXManager();

    /** @brief Importa el contenido de un archivo FBX. */
    std::vector<MeshComponent> LoadFBXModel(const std::string& filePath);

    /** @brief Importa el contenido de un archivo OBJ (Añadido para el nuevo sistema). */
    std::vector<MeshComponent> LoadOBJModel(const std::string& filePath);

    /** @brief Navega por el árbol de nodos del archivo importado FBX. */
    void ProcessFBXNode(FbxNode* node);

    /** @brief Extrae geometría de vértices, normales y UVs de un nodo FBX. */
    void ProcessFBXMesh(FbxNode* node);

    /** @brief Procesa las propiedades de superficie y texturas asociadas. */
    void ProcessFBXMaterials(FbxSurfaceMaterial* material);

    /** @brief Obtiene la lista de texturas que el motor debe cargar para este modelo. */
    std::vector<std::string> GetTextureFileNames() const { return textureFileNames; }

private:
    /* ==================================================================== */
    /* SISTEMA DE CACHÉ BINARIO (Carga ultra rápida de modelos)             */
    /* ==================================================================== */

    std::string GetBinaryCachePath() const;
    bool IsBinaryCacheUpToDate(const std::string& sourcePath, const std::string& cachePath) const;
    bool LoadBinaryCache(const std::string& cachePath);
    bool SaveBinaryCache(const std::string& cachePath) const;

private:
    FbxManager* lSdkManager; /**< Gestor de memoria del SDK FBX. */
    FbxScene* lScene;        /**< Escena cargada actualmente. */
    std::vector<std::string> textureFileNames; /**< Rutas de texturas encontradas. */

public:
    ModelType m_modelType; /**< Formato cargado (OBJ/FBX). */
    std::vector<MeshComponent> m_meshes; /**< Mallas listas para renderizar. */
};