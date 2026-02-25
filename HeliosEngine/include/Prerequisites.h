/**
 * @file Prerequisites.h
 * @brief Definiciones globales, estructuras de datos y macros de utilidad para el HeliosEngine.
 */

#pragma once
 // Librerias STD
#include <string>
#include <sstream>
#include <vector>
#include <windows.h>
#include <xnamath.h> 
#include <thread>
#include <memory>
#include <unordered_map>
#include <type_traits>
#include <array>

// Librerias DirectX
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dcompiler.h>
#include "Resource.h"
// #include "resource.h" // Descomentar solo si tienes un archivo de recursos de Windows (.rc) configurado.

// Third Party Libraries
#include "EngineUtilities/Vectors/Vector2.h"
#include "EngineUtilities/Vectors/Vector3.h"
#include "EngineUtilities/Memory/TSharedPointer.h"
#include "EngineUtilities/Memory/TWeakPointer.h"
#include "EngineUtilities/Memory/TStaticPtr.h"
#include "EngineUtilities/Memory/TUniquePtr.h"

// ======================================================================================
// MACROS
// ======================================================================================

/**
 * @def SAFE_RELEASE(x)
 * @brief Libera de forma segura un recurso de DirectX (interfaz COM) y lo establece a nullptr.
 * @param x Puntero al recurso a liberar.
 */
#define SAFE_RELEASE(x) if(x != nullptr) x->Release(); x = nullptr;

 /**
  * @def MESSAGE(classObj, method, state)
  * @brief Envía un mensaje formateado sobre la creación de recursos a la consola de salida de depuración.
  * @param classObj Nombre de la clase que genera el mensaje.
  * @param method Nombre del método donde ocurre el evento.
  * @param state Estado o descripción del recurso creado.
  */
#define MESSAGE( classObj, method, state )   \
{                                            \
   std::wostringstream os_;                  \
   os_ << classObj << "::" << method << " : " << "[CREATION OF RESOURCE " << ": " << state << "] \n"; \
   OutputDebugStringW( os_.str().c_str() );  \
}

  /**
   * @def ERROR(classObj, method, errorMSG)
   * @brief Registra un error detallado en la salida de depuración utilizando un bloque try-catch para mayor seguridad.
   * @param classObj Nombre de la clase donde ocurrió el error.
   * @param method Método donde se detectó el error.
   * @param errorMSG Mensaje descriptivo del error.
   */
#define ERROR(classObj, method, errorMSG)                     \
{                                                             \
    try {                                                     \
        std::wostringstream os_;                              \
        os_ << L"ERROR : " << classObj << L"::" << method     \
            << L" : " << errorMSG << L"\n";                   \
        OutputDebugStringW(os_.str().c_str());                \
    } catch (...) {                                           \
        OutputDebugStringW(L"Failed to log error message.\n");\
    }                                                         \
}

   // ======================================================================================
   // STRUCTURES
   // ======================================================================================

   /**
	* @struct SimpleVertex
	* @brief Estructura de entrada estándar para los vértices de modelos 3D en el Vertex Shader.
	*/
struct
	SimpleVertex {
	XMFLOAT3 Pos;    /**< Posición del vértice en el espacio 3D. */
	XMFLOAT2 Tex;    /**< Coordenadas de textura (UV). */
	XMFLOAT3 Normal; /**< Vector normal para cálculos de iluminación. */
};

/**
 * @struct SkyboxVertex
 * @brief Estructura de entrada mínima optimizada para la geometría del entorno (Cielo).
 */
struct
	SkyboxVertex {
	float x, y, z;   /**< Coordenadas espaciales locales del cubo. */
};

/**
 * @struct CBNeverChanges
 * @brief Buffer constante para datos estáticos de escena e iluminación.
 */
struct
	CBNeverChanges {
	XMMATRIX mView;       /**< Matriz de Vista (Cámara). */
	XMVECTOR mLightDir;   /**< Dirección de la luz direccional principal en el mundo. */
	XMVECTOR mLightColor; /**< Color e intensidad de la luz principal. */
};

/**
 * @struct CBSkybox
 * @brief Buffer constante especializado para el renderizado del Skybox.
 */
struct
	CBSkybox {
	XMMATRIX mviewProj;   /**< Matriz combinada de Vista y Proyección sin traslación. */
};

/**
 * @struct CBChangeOnResize
 * @brief Buffer constante para datos que dependen de la resolución de la ventana.
 */
struct
	CBChangeOnResize {
	XMMATRIX mProjection; /**< Matriz de Proyección (Perspectiva u Ortográfica). */
};

/**
 * @struct CBChangesEveryFrame
 * @brief Buffer constante para datos únicos por cada objeto renderizado.
 */
struct
	CBChangesEveryFrame {
	XMMATRIX mWorld;      /**< Matriz de Mundo (Transformación local a global). */
	XMFLOAT4 vMeshColor;  /**< Tinte de color general aplicable a la malla. */
};

// ======================================================================================
// ENUMS
// ======================================================================================

/**
 * @enum ExtensionType
 * @brief Define los formatos de archivo de imagen soportados por el gestor de texturas.
 */
enum
	ExtensionType {
	DDS = 0, /**< DirectDraw Surface (Formato nativo y optimizado para DirectX). */
	PNG = 1, /**< Portable Network Graphics (Con soporte para canal Alpha). */
	JPG = 2, /**< Joint Photographic Experts Group. */
	TGA = 3  /**< Truevision TGA (Común en exportaciones de modelado 3D). */
};

/**
 * @enum ShaderType
 * @brief Identificadores de etapa en el pipeline programable.
 */
enum
	ShaderType {
	VERTEX_SHADER = 0, /**< Etapa de procesamiento de vértices (Transformaciones). */
	PIXEL_SHADER = 1   /**< Etapa de procesamiento de fragmentos (Color y Texturizado). */
};

/**
 * @enum ComponentType
 * @brief Identificadores de la arquitectura ECS para adjuntar comportamientos a los actores.
 */
enum
	ComponentType {
	NONE = 0,      /**< Sin tipo definido o nulo. */
	TRANSFORM = 1, /**< Componente de posición, rotación y escala en el mundo. */
	MESH = 2,      /**< Componente portador de geometría (vértices e índices). */
	MATERIAL = 3,  /**< Componente definidor de propiedades visuales y texturas. */
	CAMERA = 4,    /**< Componente para proyectar la escena en pantalla. */
	SCRIPT = 5,    /**< Componente contenedor de lógica personalizada. */
	AUDIO = 6,     /**< Componente emisor de sonido espacial. */
	HIERARCHY = 7, /**< Componente de gestión de relaciones padre/hijo. */
	UNKNOWN = 8    /**< Tipo de componente no registrado nativamente. */
};