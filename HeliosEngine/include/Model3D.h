/**
 * @file Model3D.h
 * @brief Clase para la carga y gestión de modelos 3D en formatos OBJ y FBX, además de generación geométrica para entornos (Skybox).
 */

#pragma once
#include "Prerequisites.h"
#include "IResource.h"
#include "MeshComponent.h"
#include "fbxsdk.h"

 /**
  * @enum ModelType
  * @brief Define los formatos de archivo de modelos 3D soportados por el motor.
  */
enum
	ModelType {
	OBJ, /**< Formato Wavefront OBJ (Vértices, UVs y Normales simples). */
	FBX  /**< Formato Autodesk FBX (Soporta jerarquías, materiales complejos y animaciones). */
};

/**
 * @class Model3D
 * @brief Recurso que representa un modelo tridimensional compuesto por una o varias mallas.
 * @details Hereda de IResource para integrarse en el sistema de gestión de recursos.
 * Utiliza el SDK de Autodesk para procesar archivos FBX y extraer geometría y materiales.
 * Adicionalmente, permite la creación manual de geometría paramétrica (como el cubo para el Skybox).
 */
class
	Model3D : public IResource {
public:
	/**
	 * @brief Constructor estándar para cargar un modelo desde un archivo en disco.
	 * @param name Nombre o ruta del recurso.
	 * @param modelType Tipo de modelo a cargar (OBJ o FBX).
	 */
	Model3D(const std::string& name, ModelType modelType)
		: IResource(name), m_modelType(modelType), lSdkManager(nullptr), lScene(nullptr) {
		SetType(ResourceType::Model3D);
		load(name);
	}

	/**
	 * @brief Constructor paramétrico especializado en la creación de geometría estática (Ej. Skybox).
	 * @details Genera un modelo creando directamente un MeshComponent a partir de arreglos en código,
	 * lo cual es ideal para generar el cubo de entorno sin depender de la lectura de un archivo 3D.
	 * @param name Nombre identificador del recurso.
	 * @param vertices Arreglo estático con los vértices especiales del entorno.
	 * @param indices Arreglo estático con el orden de los índices para formar los polígonos.
	 */
	Model3D(const std::string& name,
		const SkyboxVertex vertices[],
		const unsigned int indices[]) : IResource(name) {
		MeshComponent mesh;
		mesh.m_skyVertex.assign(vertices, vertices + 8);
		mesh.m_index.assign(indices, indices + 36);
		mesh.m_numIndex = mesh.m_index.size(); // <--- LÍNEA AÑADIDA: Crucial para el DrawIndexed
		SetType(ResourceType::Model3D);
		m_meshes.push_back(mesh);
	}

	/**
	 * @brief Destructor por defecto.
	 */
	~Model3D() = default;

	/**
	 * @brief Carga los datos del modelo desde el disco.
	 * @param path Ruta del archivo del modelo.
	 * @return true si la carga fue exitosa, false en caso contrario.
	 */
	bool
		load(const std::string& path) override;

	/**
	 * @brief Inicializa los recursos del modelo (preparación para renderizado).
	 * @return true si la inicialización fue correcta.
	 */
	bool
		init() override;

	/**
	 * @brief Libera la memoria ocupada por las mallas y el SDK.
	 */
	void
		unload() override;

	/**
	 * @brief Obtiene el tamaño aproximado del modelo en bytes.
	 * @return Tamaño en bytes basado en la cantidad de vértices e índices.
	 */
	size_t
		getSizeInBytes() const override;

	/**
	 * @brief Obtiene el contenedor de todas las mallas que conforman el modelo.
	 * @return Referencia constante al vector de MeshComponent.
	 */
	const std::vector<MeshComponent>&
		GetMeshes() const { return m_meshes; }

	/* FBX MODEL LOADER*/

	/**
	 * @brief Inicializa el administrador de memoria y el sistema de IO del SDK de Autodesk FBX.
	 * @return true si el manager se inicializó correctamente.
	 */
	bool
		InitializeFBXManager();

	/**
	 * @brief Carga un archivo FBX utilizando el sistema de importación del SDK.
	 * @param filePath Ruta completa al archivo .fbx.
	 * @return Vector con los componentes de malla extraídos y listos para usar en el motor.
	 */
	std::vector<MeshComponent>
		LoadFBXModel(const std::string& filePath);

	/**
	 * @brief Recorre recursivamente la jerarquía de nodos del archivo FBX.
	 * @param node Puntero al nodo actual a procesar.
	 */
	void
		ProcessFBXNode(FbxNode* node);

	/**
	 * @brief Extrae la geometría (vértices, normales, UVs) de un nodo FBX de tipo Mesh.
	 * @param node Nodo que contiene el componente de malla.
	 */
	void
		ProcessFBXMesh(FbxNode* node);

	/**
	 * @brief Extrae las propiedades de materiales y rutas de texturas de un material FBX.
	 * @param material Puntero al material del SDK de FBX.
	 */
	void
		ProcessFBXMaterials(FbxSurfaceMaterial* material);

	/**
	 * @brief Obtiene la lista de nombres de archivos de textura requeridos por el modelo.
	 * @return Vector de strings con las rutas/nombres de las texturas.
	 */
	std::vector<std::string>
		GetTextureFileNames() const { return textureFileNames; }

private:
	FbxManager* lSdkManager;                   /**< Administrador global de memoria del SDK de FBX. */
	FbxScene* lScene;                          /**< Objeto escena que contiene toda la información del archivo importado. */
	std::vector<std::string> textureFileNames; /**< Almacén temporal de nombres de texturas extraídas del modelo. */

public:
	ModelType m_modelType;                     /**< Almacena el formato de origen del modelo actual. */
	std::vector<MeshComponent> m_meshes;       /**< Lista de sub-mallas procesadas y listas para el motor. */
};