#pragma once
/**
 * @file Prerequisites.h
 * @brief Cabecera centralizada de dependencias y utilidades (Win32 + Direct3D 11 + DirectXMath).
 *
 * @details
 * - Reúne includes estándar (STL), Win32 y Direct3D 11, más DirectXMath.
 * - Define macros utilitarias para liberar COM y para logging en la salida de depuración.
 * - Pensado para usarse como “precompiled header” del proyecto o como cabecera común.
 *
 * @note Los macros de logging aquí definidos formatean mensajes a `OutputDebugStringW`.
 *       Asegúrate de tener una ventana de *Output* visible (Debug) para verlos.
 */

 // ==============================
 // Dependencias estándar (STL)
 // ==============================

 /**
  * @defgroup PrereqSTD STL / C++ Standard Library
  * @brief Incluye tipos y utilidades generales.
  * @{
  */
#include <string>      ///< @brief `std::string`, `std::wstring` y utilidades de cadena.
#include <sstream>     ///< @brief `std::ostringstream` / `std::wostringstream` para formateo.
#include <vector>      ///< @brief Contenedor dinámico `std::vector`.
#include <thread>      ///< @brief `std::thread` y utilidades de concurrencia.
  /// @}

  // ==============================
  // Win32 + Direct3D 11 (SDK)
  // ==============================

  /**
   * @defgroup PrereqWin32 Win32 + Direct3D 11
   * @brief Cabeceras base del API de Windows y Direct3D 11.
   * @{
   */
#include <windows.h>    ///< @brief Tipos/funciones Win32 (HWND, HINSTANCE, mensajes, etc.).
#include <d3d11.h>      ///< @brief Interfaces D3D11 (ID3D11Device, ID3D11DeviceContext, etc.).
#include <dxgi.h>       ///< @brief DXGI (swap chain, formatos, adaptadores).
#include <d3dcompiler.h>///< @brief Compilador HLSL (D3DCompile, blobs).
   /// @}

   // ==============================
   // DirectXMath (moderna)
   // ==============================

   /**
    * @defgroup PrereqDXM DirectXMath
    * @brief Tipos y funciones SIMD (XMFLOAT*, XMMATRIX, XMVector*).
    * @details Usa espacios de nombres y funciones inline; no requiere d3dx.
    * @{
    */
#include <DirectXMath.h> ///< @see https://learn.microsoft.com/windows/win32/dxmath/pg-xnamath-migration
    /// @}

    /**
     * @defgroup PrereqMacros Macros utilitarios
     * @brief Macros para liberación segura y logging a la salida de depuración.
     * @{
     */

     /**
      * @brief Libera de forma segura un puntero COM y lo pone a `nullptr`.
      *
      * @param x Puntero COM (por ejemplo, `ID3D11Buffer*`, `ID3D11Device*`, etc.).
      *
      * @warning El parámetro debe ser una **lvalue** (una variable asignable). No pases temporales.
      * @code
      * ID3D11Buffer* vb = nullptr;
      * // ... vb creado ...
      * SAFE_RELEASE(vb); // vb->Release() y luego vb = nullptr
      * @endcode
      */
#define SAFE_RELEASE(x) if((x) != nullptr) { (x)->Release(); (x) = nullptr; }

      /**
       * @brief Escribe un mensaje informativo en la salida de depuración (Unicode).
       *
       * @param classObj Nombre del “módulo/clase” (macro tokenizado).
       * @param method   Nombre del método/función (macro tokenizado).
       * @param state    Texto corto del estado (macro tokenizado).
       *
       * @details Este macro **stringiza** los parámetros `classObj`, `method` y `state`
       * (mediante `#`) y arma un mensaje con el formato:
       * `Class::Method : [ State ]\n`
       *
       * @note Debido al stringizing, si pasas literales con `L"..."`, en el output
       * verás los caracteres de literal (p.ej. `L"MiClase"`). Es normal con esta versión.
       * Si prefieres pasar wstring directamente sin `#`, podemos darte una variante.
       *
       * @code
       * // Ejemplo
       * MESSAGE(Device, Init, OK);
       * // Output: "Device::Init : [ OK ]"
       * @endcode
       */
#define MESSAGE(classObj, method, state) \
{ std::wostringstream os_; os_ << L#classObj L"::" L#method L" : " << L"[ " << L#state << L" ]\n"; \
  OutputDebugStringW(os_.str().c_str()); }

       /**
        * @brief Escribe un mensaje de error en la salida de depuración (Unicode).
        *
        * @param classObj Nombre del “módulo/clase” (macro tokenizado).
        * @param method   Nombre del método/función (macro tokenizado).
        * @param errorMSG Mensaje de error detallado (expresión wide-string, p.ej. `L"...")`.
        *
        * @details Formato de salida:
        * `ERROR : Class::Method : <mensaje>\n`
        *
        * @code
        * // Ejemplo
        * ERROR(Device, CreateBuffer, L"CreateBuffer() falló con E_FAIL");
        * @endcode
        */
#define ERROR(classObj, method, errorMSG) \
{ try { std::wostringstream os_; os_ << L"ERROR : " << L#classObj << L"::" L#method \
  << L" : " << errorMSG << L"\n"; OutputDebugStringW(os_.str().c_str()); } \
  catch (...) { OutputDebugStringW(L"Failed to log error message.\n"); } }

        /// @} // end of group PrereqMacros
