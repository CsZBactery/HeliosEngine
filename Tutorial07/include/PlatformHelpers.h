#pragma once

//--------------------------------------------------------------------------------------
// File: PlatformHelpers.h (ajustado)
//--------------------------------------------------------------------------------------

// Silencia alineación por _declspec
#ifdef _MSC_VER
#pragma warning(disable : 4324)
#endif

// --- Windows / SDK ---
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <sdkddkver.h>     // NTDDI_*, _WIN32_WINNT_*
#include <Windows.h>       // HANDLE, HRESULT, OutputDebugStringA, SAL macros, etc.
#include <winapifamily.h>  // WINAPI_FAMILY_xxx

// --- C / C++ runtime ---
#include <cstdint>
#include <cstdarg>
#include <cstdio>
#include <exception>
#include <memory>

// MAKEFOURCC si no viene de headers de D3D antiguos
#ifndef MAKEFOURCC
#define MAKEFOURCC(ch0, ch1, ch2, ch3) \
    (static_cast<uint32_t>(static_cast<uint8_t>(ch0)) \
    | (static_cast<uint32_t>(static_cast<uint8_t>(ch1)) << 8) \
    | (static_cast<uint32_t>(static_cast<uint8_t>(ch2)) << 16) \
    | (static_cast<uint32_t>(static_cast<uint8_t>(ch3)) << 24))
#endif

// s/constexpr en entornos antiguos (coincide con los headers de DirectXTK)
#ifndef ENUM_FLAGS_CONSTEXPR
#if defined(NTDDI_WIN10_RS1) && !defined(__MINGW32__)
#define ENUM_FLAGS_CONSTEXPR constexpr
#else
#define ENUM_FLAGS_CONSTEXPR const
#endif
#endif

namespace DirectX
{
  // Excepción COM
  class com_exception : public std::exception
  {
  public:
    explicit com_exception(HRESULT hr) noexcept : result(hr) {}

    const char* what() const noexcept override
    {
      static char s_str[64] = {};
      std::snprintf(s_str, sizeof(s_str), "Failure with HRESULT of %08X",
        static_cast<unsigned int>(result));
      return s_str;
    }

    HRESULT get_result() const noexcept { return result; }

  private:
    HRESULT result;
  };

  // Lanza com_exception si hr es fallo
  inline void ThrowIfFailed(HRESULT hr) noexcept(false)
  {
    if (FAILED(hr))
      throw com_exception(hr);
  }

  // Trace a la ventana de depuración
  inline void DebugTrace(_In_z_ _Printf_format_string_ const char* format, ...) noexcept
  {
#ifdef _DEBUG
    va_list args;
    va_start(args, format);

    char buff[1024] = {};
    std::vsnprintf(buff, sizeof(buff), format, args);
    OutputDebugStringA(buff);

    va_end(args);
#else
    UNREFERENCED_PARAMETER(format);
#endif
  }

  // Smart pointers/handles
#if (_WIN32_WINNT >= _WIN32_WINNT_WIN10) || (defined(_XBOX_ONE) && defined(_TITLE)) \
    || !defined(WINAPI_FAMILY) || (WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP)
  struct virtual_deleter
  {
    void operator()(void* p) noexcept
    {
      if (p) ::VirtualFree(p, 0, MEM_RELEASE);
    }
  };
#endif

  struct handle_closer
  {
    void operator()(HANDLE h) noexcept
    {
      if (h) ::CloseHandle(h);
    }
  };

  using ScopedHandle = std::unique_ptr<void, handle_closer>;

  inline HANDLE safe_handle(HANDLE h) noexcept
  {
    return (h == INVALID_HANDLE_VALUE) ? nullptr : h;
  }
}
