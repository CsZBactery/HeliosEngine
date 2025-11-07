#pragma once
/**
 * @file Prerequisites.h
 * @brief Cabecera con dependencias, macros utilitarias y estructuras básicas del engine.
 * @details Declara includes del sistema/DirectX, macros de logging/recursos y
 *          estructuras comunes de constantes y vértices para el pipeline.
 * @date 2025-11-04
 * @version 1.0
 */

 // = STD ==
#include <string>
#include <sstream>
#include <vector>
#include <thread>
#include <cfloat>


#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif

// == Math ==
#include <windows.h>
#include <xnamath.h>

// == DirectX 11 ==
#include <d3d11.h>
#include <d3dcompiler.h>

// Recursos del engine
#include "Resource.h"
#include "resource.h"

// == MACROS ==

/**
 * @def SAFE_RELEASE(x)
 * @brief Libera de manera segura un recurso COM/DirectX.
 * @details Si el puntero no es nulo, invoca <tt>Release()</tt> y lo pone a <tt>nullptr</tt>
 *          para evitar dobles liberaciones o accesos inválidos.
 * @param x Puntero al recurso a liberar.
 *
 * @warning Debe pasarse un puntero válido a una interfaz COM (e.g., <tt>ID3D11Buffer*</tt>).
 */
#define SAFE_RELEASE(x) if((x) != nullptr){ (x)->Release(); (x) = nullptr; }

 /**
  * @def MESSAGE(classObj, method, state)
  * @brief Traza un mensaje de creación/estado a la ventana de depuración.
  * @details Formatea y manda a <tt>OutputDebugStringW</tt> un mensaje con
  *          clase, método y estado, útil para diagnosticar creación de recursos.
  * @param classObj Nombre de la clase (literal amplio o <tt>LPCWSTR</tt> imprimible).
  * @param method   Nombre del método.
  * @param state    Texto del estado (por ejemplo: <tt>"OK"</tt>, <tt>"FAILED"</tt>).
  */
#define MESSAGE( classObj, method, state )   \
{                                            \
   std::wostringstream os_;                  \
   os_ << classObj << L"::" << method << L" : " << L"[CREATION OF RESOURCE : " << state << L"]\n"; \
   OutputDebugStringW( os_.str().c_str() );  \
}

  /**
   * @def ERROR(classObj, method, errorMSG)
   * @brief Registra un mensaje de error en la ventana de depuración.
   * @details Captura clase, método y la descripción del error. Envía el texto a
   *          <tt>OutputDebugStringW</tt>. En caso de excepción al formatear, informa el fallo.
   * @param classObj Nombre de la clase donde ocurre el error.
   * @param method   Nombre del método que reporta el error.
   * @param errorMSG Mensaje descriptivo del error (amplio o convertible a <tt>std::wstring</tt>).
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

   // == Tipos del Engine ==

/** Vértice completo con posición, textura y normal. */
struct SimpleVertex {
    XMFLOAT3 Pos;
    XMFLOAT2 Tex;
    XMFLOAT3 Normal;
};

/** Buffer constante invariable (cámara). */
struct CBNeverChanges {
    XMMATRIX mView;
};

/** Buffer constante que cambia al redimensionar. */
struct CBChangeOnResize {
    XMMATRIX mProjection;
};

/** Buffer constante por frame. */
struct CBChangesEveryFrame {
    XMMATRIX mWorld;
    XMFLOAT4 vMeshColor;
};

/** Tipos de extensiones soportadas para texturas. */
enum ExtensionType { DDS = 0, PNG = 1, JPG = 2 };

/** Tipo de shader manejado por el sistema. */
enum ShaderType { VERTEX_SHADER = 0, PIXEL_SHADER = 1 };
