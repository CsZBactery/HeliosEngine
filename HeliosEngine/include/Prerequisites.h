#pragma once
/**
 * @file Prerequisites.h
 * @brief Cabecera con dependencias, macros utilitarias y estructuras básicas del engine.
 * @details Declara includes del sistema/DirectX, macros de logging/recursos y
 *          estructuras comunes de constantes y vértices para el pipeline.
 * @date 2025-11-04
 * @version 1.0
 */

 // Librerias STD
#include <string>
#include <sstream>
#include <vector>
#include <windows.h>
#include <xnamath.h>
#include <thread>

// Librerias DirectX
#include <d3d11.h>
#include <d3dcompiler.h>
#include "Resource.h"
#include "resource.h"

// third Party Libraries

// MACROS

/**
 * @def SAFE_RELEASE(x)
 * @brief Libera de manera segura un recurso COM/DirectX.
 * @details Si el puntero no es nulo, invoca <tt>Release()</tt> y lo pone a <tt>nullptr</tt>
 *          para evitar dobles liberaciones o accesos inválidos.
 * @param x Puntero al recurso a liberar.
 *
 * @warning Debe pasarse un puntero válido a una interfaz COM (e.g., <tt>ID3D11Buffer*</tt>).
 */
#define SAFE_RELEASE(x) if(x != nullptr) x->Release(); x = nullptr;

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
   os_ << classObj << "::" << method << " : " << "[CREATION OF RESOURCE " << ": " << state << "] \n"; \
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

   /**
    * @struct SimpleVertex
    * @brief Vértice mínimo con posición y coordenadas de textura.
    * @note Compatible con layouts típicos <tt>POSITION</tt> y <tt>TEXCOORD</tt>.
    */
struct SimpleVertex {
    XMFLOAT3 Pos;  /**< Coordenadas de posición del vértice (x, y, z). */
    XMFLOAT2 Tex;  /**< Coordenadas de textura (u, v). */
};

/**
 * @struct CBNeverChanges
 * @brief Buffer constante con datos invariables durante la ejecución.
 * @details Usualmente contiene la matriz de vista (cámara).
 */
struct CBNeverChanges {
    XMMATRIX mView; /**< Matriz de vista usada por la cámara. */
};

/**
 * @struct CBChangeOnResize
 * @brief Buffer constante actualizado al cambiar el tamaño de la ventana.
 * @details Incluye la matriz de proyección dependiente de <em>aspect ratio</em>.
 */
struct CBChangeOnResize {
    XMMATRIX mProjection; /**< Matriz de proyección ajustada al tamaño actual. */
};

/**
 * @struct CBChangesEveryFrame
 * @brief Buffer constante actualizado cada frame.
 * @details Contiene la transformación de mundo y un color para la malla.
 */
struct CBChangesEveryFrame {
    XMMATRIX mWorld;      /**< Matriz de mundo para transformar los objetos. */
    XMFLOAT4 vMeshColor;  /**< Color aplicado a la malla (rgba). */
};

/**
 * @enum ExtensionType
 * @brief Tipos de extensiones soportadas para texturas.
 */
enum ExtensionType {
    DDS = 0, /**< Textura en formato DDS (DirectDraw Surface). */
    PNG = 1, /**< Textura en formato PNG (Portable Network Graphics). */
    JPG = 2  /**< Textura en formato JPG (Joint Photographic Experts Group). */
};

/**
 * @enum ShaderType
 * @brief Tipo de shader manejado por el sistema.
 */
enum ShaderType {
    VERTEX_SHADER = 0, /**< Shader de vértices. */
    PIXEL_SHADER = 1  /**< Shader de píxeles (fragment). */
};
