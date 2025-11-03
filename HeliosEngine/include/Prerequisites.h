#pragma once
//Librerias STD
#include <string>
#include <sstream>
#include <vector>
#include <windows.h>
#include <xnamath.h>
#include <thread>


//Librerias DirectX
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dcompiler.h>
#include "Resource.h"
#include "resource.h"


//third Party Libraries

// MACROS

/**
 * @brief Libera de manera segura un recurso de DirectX.
 *
 * Si el puntero no es nulo, libera la memoria con Release()
 * y lo asigna a nullptr para evitar accesos inv�lidos.
 *
 * @param x Puntero al recurso que se va a liberar.
 */
#define SAFE_RELEASE(x) if(x != nullptr) x->Release(); x = nullptr;

 /**
  * @brief Macro para mostrar mensajes de creaci�n de recursos en la ventana de depuraci�n.
  *
  * Formatea un mensaje con la clase, el m�todo y el estado actual de la creaci�n.
  *
  * @param classObj Nombre de la clase donde ocurre el evento.
  * @param method Nombre del m�todo donde ocurre el evento.
  * @param state Estado del recurso (ejemplo: "OK", "FAILED").
  */
#define MESSAGE( classObj, method, state )   \
{                                            \
   std::wostringstream os_;                  \
   os_ << classObj << "::" << method << " : " << "[CREATION OF RESOURCE " << ": " << state << "] \n"; \
   OutputDebugStringW( os_.str().c_str() );  \
}

  /**
   * @brief Macro para registrar mensajes de error en la ventana de depuraci�n.
   *
   * Captura informaci�n detallada de la clase, m�todo y descripci�n del error.
   * Si ocurre un fallo durante el registro, captura la excepci�n y notifica.
   *
   * @param classObj Nombre de la clase donde ocurre el error.
   * @param method Nombre del m�todo donde ocurre el error.
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

   /**
    * @brief Representa un v�rtice simple con posici�n y coordenadas de textura.
    */
struct
    SimpleVertex {
    XMFLOAT3 Pos;  /**< Coordenadas de posici�n del v�rtice (x, y, z). */
    XMFLOAT2 Tex;  /**< Coordenadas de textura (u, v). */
};

/**
 * @brief Constantes que nunca cambian: contiene la matriz de vista.
 */
struct
    CBNeverChanges {
    XMMATRIX mView; /**< Matriz de vista usada en la c�mara. */
};

/**
 * @brief Constantes que cambian al redimensionar la ventana.
 */
struct
    CBChangeOnResize {
    XMMATRIX mProjection; /**< Matriz de proyecci�n ajustada al tama�o de la ventana. */
};

/**
 * @brief Constantes que cambian en cada frame.
 */
struct
    CBChangesEveryFrame {
    XMMATRIX mWorld;      /**< Matriz de mundo para transformar los objetos. */
    XMFLOAT4 vMeshColor;  /**< Color aplicado a la malla. */
};

/**
 * @brief Tipos de extensi�n soportados para las texturas.
 */
enum
    ExtensionType {
    DDS = 0, /**< Textura en formato DDS (DirectDraw Surface). */
    PNG = 1, /**< Textura en formato PNG (Portable Network Graphics). */
    JPG = 2  /**< Textura en formato JPG (Joint Photographic Experts Group). */
};

enum
    ShaderType {
    VERTEX_SHADER = 0,
    PIXEL_SHADER = 1
};