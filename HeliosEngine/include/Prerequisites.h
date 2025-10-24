#pragma once
//
// Prerequisites.h
// Cabecera común con includes de Win32/D3D11/DirectXMath, utilidades y tipos compartidos.
//

// ======================================================
// STL
// ======================================================
#include <string>
#include <sstream>
#include <vector>
#include <thread>
#include <cstring>   // std::memcpy
#include <cstdint>

// ======================================================
// Win32 + Direct3D 11
// ======================================================
#ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#  define NOMINMAX
#endif
#include <Windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>

// ======================================================
// DirectXMath
// ======================================================
#include <DirectXMath.h>

// ======================================================
// Utilidades / macros
// ======================================================

// Algunas cabeceras del SDK definen macros con estos nombres.
// Las quitamos para evitar colisiones con nuestros macros.
#ifdef ERROR
#  undef ERROR
#endif
#ifdef MESSAGE
#  undef MESSAGE
#endif

// Liberación segura de interfaces COM.
#ifndef SAFE_RELEASE
#  define SAFE_RELEASE(x) do { if ((x)) { (x)->Release(); (x) = nullptr; } } while(0)
#endif

// Salida de depuración (Unicode / ANSI)
inline void __helios_logw(const wchar_t* s) { ::OutputDebugStringW(s); }
inline void __helios_loga(const char* s) { ::OutputDebugStringA(s); }

// Macro de mensaje informativo
#define MESSAGE(MOD, FUNC, WMSG) \
  do { std::wostringstream __os; \
       __os << L"[" L#MOD L"][" L#FUNC L"] " << WMSG << L"\n"; \
       __helios_logw(__os.str().c_str()); } while(0)

// Macro de error
#define ERROR(MOD, FUNC, WMSG) \
  do { std::wostringstream __os; \
       __os << L"[ERROR][" L#MOD L"][" L#FUNC L"] " << WMSG << L"\n"; \
       __helios_logw(__os.str().c_str()); } while(0)

// (Opcional) autolink de librerías con MSVC
// #pragma comment(lib, "d3d11.lib")
// #pragma comment(lib, "dxgi.lib")
// #pragma comment(lib, "d3dcompiler.lib")

// ======================================================
// Tipos compartidos por el engine
// ======================================================

// Vértice básico: posición + UV


// Constant buffers usados por los shaders (VS/PS)
//struct CBNeverChanges      // register(b0)
//{
   // DirectX::XMMATRIX mView;
//};

//struct CBChangeOnResize    // register(b1)
//{
   // DirectX::XMMATRIX mProjection;
//};

//struct CBChangesEveryFrame // register(b2)
//{
  //  DirectX::XMMATRIX mWorld;
   // DirectX::XMFLOAT4 vMeshColor;
//};

// Asegurar que los CBs respetan alineación de 16 bytes
//static_assert((sizeof(CBNeverChanges) % 16) == 0, "CBNeverChanges must be 16-byte aligned.");
//static_assert((sizeof(CBChangeOnResize) % 16) == 0, "CBChangeOnResize must be 16-byte aligned.");
//static_assert((sizeof(CBChangesEveryFrame) % 16) == 0, "CBChangesEveryFrame must be 16-byte aligned.");
