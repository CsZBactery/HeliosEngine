#pragma once
/**
 * @file Prerequisites.h
 * @brief Cabecera centralizada de dependencias y utilidades (Win32 + Direct3D 11 + DirectXMath).
 *
 * @details
 * - Re�ne includes est�ndar (STL), Win32 y Direct3D 11, m�s DirectXMath.
 * - Define macros utilitarios para liberar COM y para logging en la salida de depuraci�n.
 * - Pensado para usarse como �precompiled header� del proyecto o como cabecera com�n.
 *
 * @note Los macros de logging formatean mensajes a `OutputDebugStringW`.
 *       Abre la ventana *Output* (Debug) para verlos.
 */

 // ==============================
 // Dependencias est�ndar (STL)
 // ==============================

 /**
  * @defgroup PrereqSTD STL / C++ Standard Library
  * @{
  */
#include <string>      ///< `std::string`, `std::wstring`.
#include <sstream>     ///< `std::wostringstream` para logs formateados.
#include <vector>      ///< `std::vector`.
#include <thread>      ///< `std::thread`, utilidades de concurrencia.
  /// @}

  // ==============================
  // Win32 + Direct3D 11 (SDK)
  // ==============================

  /**
   * @defgroup PrereqWin32 Win32 + Direct3D 11
   * @brief Cabeceras base del API de Windows y Direct3D 11.
   * @{
   */
#include <Windows.h>     ///< Tipos/funciones Win32 (HWND, HINSTANCE, etc.).
#include <d3d11.h>       ///< Interfaces D3D11 (ID3D11Device, ID3D11DeviceContext, ...).
#include <dxgi.h>        ///< DXGI (swap chain, formatos, adaptadores).
#include <d3dcompiler.h> ///< Compilador HLSL (D3DCompile, blobs).
   /// @}

   // ==============================
   // DirectXMath (moderna)
   // ==============================

   /**
    * @defgroup PrereqDXM DirectXMath
    * @brief Tipos y funciones SIMD (XMFLOAT*, XMMATRIX, XMVector*).
    * @details Usa espacios de nombres y funciones inline; no requiere D3DX.
    * @{
    */
#include <DirectXMath.h>
    /// @}

    // ==============================
    // Macros utilitarios (seguros)
    // ==============================

    /**
     * @defgroup PrereqMacros Macros utilitarios
     * @brief Macros para liberaci�n segura y logging a la salida de depuraci�n.
    //  * @{
     */

     // Algunas cabeceras del SDK definen macros con estos nombres.
     // Los �desarmamos� para evitar colisiones.
#ifdef ERROR
#  undef ERROR
#endif
#ifdef MESSAGE
#  undef MESSAGE
#endif

/**
 * @brief Libera de forma segura un puntero COM y lo pone a `nullptr`.
 * @param x Puntero COM (lvalue).
 * @code
 * ID3D11Buffer* vb = nullptr;
 * // ... crear vb ...
 * SAFE_RELEASE(vb); // vb->Release(); vb = nullptr;
 * @endcode
 */
#ifndef SAFE_RELEASE
#  define SAFE_RELEASE(x) do { if ((x)) { (x)->Release(); (x) = nullptr; } } while(0)
#endif

 /// Funciones helper para escribir a la salida de depuraci�n (Unicode/ANSI).
inline void __helios_logw(const wchar_t* s) { ::OutputDebugStringW(s); }
inline void __helios_loga(const char* s) { ::OutputDebugStringA(s); }

/**
 * @brief Escribe un mensaje informativo en la salida de depuraci�n (Unicode).
 * @param MOD   Nombre del m�dulo/clase (token, no string).
 * @param FUNC  Nombre del m�todo/funci�n (token, no string).
 * @param WMSG  Mensaje wide-string `L"..."`.
 * @code
 * MESSAGE(Device, Init, L"OK");
 * @endcode
 */
#define MESSAGE(MOD, FUNC, WMSG) \
  do { std::wostringstream __os; \
       __os << L"[" L#MOD L"][" L#FUNC L"] " << WMSG << L"\n"; \
       __helios_logw(__os.str().c_str()); } while(0)

 /**
  * @brief Escribe un mensaje de error en la salida de depuraci�n (Unicode).
  * @param MOD   Nombre del m�dulo/clase (token).
  * @param FUNC  Nombre del m�todo/funci�n (token).
  * @param WMSG  Mensaje wide-string `L"..."`.
  * @code
  * ERROR(Device, CreateBuffer, L"CreateBuffer() fall� con E_FAIL");
  * @endcode
  */
#define ERROR(MOD, FUNC, WMSG) \
  do { std::wostringstream __os; \
       __os << L"[ERROR][" L#MOD L"][" L#FUNC L"] " << WMSG << L"\n"; \
       __helios_logw(__os.str().c_str()); } while(0)

  // (Opcional) Autolink en MSVC. Si no te gusta, elim�nalas y agrega libs en el Linker.
  // #pragma comment(lib, "d3d11.lib")
  // #pragma comment(lib, "dxgi.lib")
  // #pragma comment(lib, "d3dcompiler.lib")

  /** @} */ // end of group PrereqMacros