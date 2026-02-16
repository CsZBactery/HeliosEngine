#pragma once
// Librerias STD
#include <string>
#include <sstream>
#include <vector>
#include <windows.h>

// NOTA: Si xnamath te da problemas con ImGui o versiones nuevas de VS,
// considera cambiar <xnamath.h> por <DirectXMath.h> y agregar "using namespace DirectX;"
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
#define SAFE_RELEASE(x) if(x != nullptr) x->Release(); x = nullptr;

#define MESSAGE( classObj, method, state )   \
{                                            \
   std::wostringstream os_;                  \
   os_ << classObj << "::" << method << " : " << "[CREATION OF RESOURCE " << ": " << state << "] \n"; \
   OutputDebugStringW( os_.str().c_str() );  \
}

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

struct SimpleVertex
{
    XMFLOAT3 Pos;
    XMFLOAT2 Tex;
    XMFLOAT3 Normal;
};

struct CBNeverChanges
{
    XMMATRIX mView;
    XMVECTOR mLightDir;
    XMVECTOR mLightColor;
};

struct CBChangeOnResize
{
    XMMATRIX mProjection;
};

struct CBChangesEveryFrame
{
    XMMATRIX mWorld;
    XMFLOAT4 vMeshColor;
};

// --- AQUÍ ESTABA EL FALTANTE DE TGA ---
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

enum ComponentType {
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