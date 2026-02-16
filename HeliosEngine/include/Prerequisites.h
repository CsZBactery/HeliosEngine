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

// Librerias DirectX
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dcompiler.h>
#include "Resource.h"

// Third Party Libraries
#include "EngineUtilities/Vectors/Vector2.h"
#include "EngineUtilities/Vectors/Vector3.h"
#include "EngineUtilities/Memory/TSharedPointer.h"
#include "EngineUtilities/Memory/TWeakPointer.h"
#include "EngineUtilities/Memory/TStaticPtr.h"
#include "EngineUtilities/Memory/TUniquePtr.h"

// MACROS

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

   //--------------------------------------------------------------------------------------
   // Structures
   //--------------------------------------------------------------------------------------

   /**
    * @struct SimpleVertex
    * @brief Estructura de entrada para los vértices en el Vertex Shader.
    */
struct SimpleVertex
{
    XMFLOAT3 Pos;    /**< Posición del vértice en el espacio 3D. */
    XMFLOAT2 Tex;    /**< Coordenadas de textura (UV). */
    XMFLOAT3 Normal; /**< Vector normal para cálculos de iluminación. */
};

/**
 * @struct CBNeverChanges
 * @brief Buffer constante para datos que permanecen estáticos o cambian por frame pero no por objeto.
 */
struct CBNeverChanges
{
    XMMATRIX mView;       /**< Matriz de Vista (Cámara). */
    XMVECTOR mLightDir;   /**< Dirección de la luz en el mundo. */
    XMVECTOR mLightColor; /**< Color e intensidad de la luz. */
};

/**
 * @struct CBChangeOnResize
 * @brief Buffer constante para datos que solo se actualizan cuando cambia el tamaño de la ventana.
 */
struct CBChangeOnResize
{
    XMMATRIX mProjection; /**< Matriz de Proyección. */
};

/**
 * @struct CBChangesEveryFrame
 * @brief Buffer constante para datos que cambian por cada objeto dibujado.
 */
struct CBChangesEveryFrame
{
    XMMATRIX mWorld;      /**< Matriz de Mundo (Transformación del objeto). */
    XMFLOAT4 vMeshColor;  /**< Color base o tinte de la malla. */
};

/**
 * @enum ExtensionType
 * @brief Define los formatos de archivo de imagen soportados para texturas.
 */
enum ExtensionType {
    DDS = 0, /**< DirectDraw Surface (Formato nativo de DirectX). */
    PNG = 1, /**< Portable Network Graphics. */
    JPG = 2, /**< Joint Photographic Experts Group. */
    TGA = 3  /**< Truevision TGA. */
};

/**
 * @enum ShaderType
 * @brief Identificadores para los diferentes tipos de etapas de shader.
 */
enum ShaderType {
    VERTEX_SHADER = 0, /**< Shader de procesamiento de vértices. */
    PIXEL_SHADER = 1   /**< Shader de procesamiento de fragmentos/píxeles. */
};

/**
 * @enum ComponentType
 * @brief Identificadores de tipos de componentes para el sistema Actor-Componente.
 */
enum ComponentType {
    NONE = 0,      /**< Sin tipo definido. */
    TRANSFORM = 1, /**< Componente de posición, rotación y escala. */
    MESH = 2,      /**< Componente de malla geométrica. */
    MATERIAL = 3,  /**< Componente de propiedades visuales y texturas. */
    CAMERA = 4,    /**< Componente de cámara. */
    SCRIPT = 5,    /**< Componente de lógica de script. */
    AUDIO = 6,     /**< Componente de fuente de sonido. */
    HIERARCHY = 7, /**< Componente de gestión de jerarquía (padre/hijo). */
    UNKNOWN = 8    /**< Tipo desconocido o personalizado. */
};