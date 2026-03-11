/**
 * @file Prerequisites.h
 * @brief Definiciones globales, estructuras de datos y macros de utilidad para HeliosEngine.
 */

#pragma once

 //--------------------------------------------------------------------------------------
 // Librerias STD
 //--------------------------------------------------------------------------------------
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

//--------------------------------------------------------------------------------------
// Librerias DirectX
//--------------------------------------------------------------------------------------
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dcompiler.h>
#include "Resource.h"
#include "resource.h"

//--------------------------------------------------------------------------------------
// Third Party Libraries (Engine Utilities)
//--------------------------------------------------------------------------------------
#include "EngineUtilities/Vectors/Vector2.h"
#include "EngineUtilities/Vectors/Vector3.h"
#include "EngineUtilities/Memory/TSharedPointer.h"
#include "EngineUtilities/Memory/TWeakPointer.h"
#include "EngineUtilities/Memory/TStaticPtr.h"
#include "EngineUtilities/Memory/TUniquePtr.h"

//--------------------------------------------------------------------------------------
// MACROS
//--------------------------------------------------------------------------------------

/** @def SAFE_RELEASE(x) Libera recursos COM de DirectX de forma segura. */
#define SAFE_RELEASE(x) if(x != nullptr) x->Release(); x = nullptr;

/** @def MESSAGE Logs de creación de recursos en la consola de salida. */
#define MESSAGE( classObj, method, state )   \
{                                            \
   std::wostringstream os_;                  \
   os_ << classObj << "::" << method << " : " << "[CREATION OF RESOURCE " << ": " << state << "] \n"; \
   OutputDebugStringW( os_.str().c_str() );  \
}

/** @def ERROR Logs de errores con bloque try-catch de seguridad. */
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
 * @brief Estructura de vértice completa para modelos 3D con soporte para Normal Mapping.
 */
struct
	SimpleVertex {
	EU::Vector3 Position;        /**< Posición en el espacio 3D. */
	EU::Vector3 Normal;          /**< Vector normal para iluminación. */
	EU::Vector3 Tangent;         /**< Tangente para Normal Mapping. */
	EU::Vector3 Bitangent;       /**< Bitangente para calcular el espacio Tangente. */
	EU::Vector2 TextureCoordinate; /**< Coordenadas UV. */
};

/** @struct SkyboxVertex Geometría mínima para el Cubemap. */
struct
	SkyboxVertex {
	float x, y, z;
};

/** @struct CBNeverChanges Buffers constantes que no varían tras la carga. */
struct
	CBNeverChanges {
	XMMATRIX mView;
};

/** @struct CBSkybox Matriz específica para el renderizado del fondo. */
struct
	CBSkybox {
	XMMATRIX mviewProj;
};

/** @struct CBChangeOnResize Datos dependientes de la resolución. */
struct
	CBChangeOnResize {
	XMMATRIX mProjection;
};

/**
 * @struct CBMain
 * @brief Buffer principal para el HeliosEngine.fx (Iluminación y Cámara).
 * @note Alineado a 16 bytes.
 */
struct
	CBMain {
	XMFLOAT4X4 View;
	XMFLOAT4X4 Projection;
	EU::Vector3 CameraPos;
	float pad0;
	EU::Vector3 LightDir;
	float pad1;
	EU::Vector3 LightColor;
	float pad2;
};

/** @struct CBChangesEveryFrame Datos por cada instancia de objeto (Actor). */
struct
	CBChangesEveryFrame {
	XMMATRIX mWorld;
	XMFLOAT4 vMeshColor;
};

//--------------------------------------------------------------------------------------
// Enums
//--------------------------------------------------------------------------------------

enum ExtensionType {
	DDS = 0,
	PNG = 1,
	JPG = 2,
	TGA = 3
};

enum ShaderType {
	VERTEX_SHADER = 0,
	PIXEL_SHADER = 1
};

/**
 * @enum ComponentType
 * @brief Identificadores para el sistema de arquitectura de HeliosEngine.
 */
enum
	ComponentType {
	NONE = 0,
	TRANSFORM = 1,
	MESH = 2,
	MATERIAL = 3,
	CAMERA = 4,
	SCRIPT = 5,
	AUDIO = 6,
	HIERARCHY = 7,
	UNKNOWN = 8
};