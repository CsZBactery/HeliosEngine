#pragma once

//--------------------------------------------------------------------------------------
// File: DirectXHelpers.h  (versión corregida, sin pch)
//--------------------------------------------------------------------------------------

// === Windows / SDK base ===
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>     // SAL macros, HRESULT, etc.
#include <dxgiformat.h>  // DXGI_FORMAT y WKPDID_D3DDebugObjectName

// === Direct3D 11 ===
#if defined(_XBOX_ONE) && defined(_TITLE)
#include <d3d11_x.h>
#else
#include <d3d11_1.h>
#endif

// NOTA: si quieres, mueve el link de dxguid a un .cpp (recomendado)
// #pragma comment(lib,"dxguid.lib")  // <— mejor en .cpp

// === C/C++ runtime ===
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <exception>

#ifndef DIRECTX_TOOLKIT_API
#ifdef DIRECTX_TOOLKIT_EXPORT
#ifdef __GNUC__
#define DIRECTX_TOOLKIT_API __attribute__((dllexport))
#else
#define DIRECTX_TOOLKIT_API __declspec(dllexport)
#endif
#elif defined(DIRECTX_TOOLKIT_IMPORT)
#ifdef __GNUC__
#define DIRECTX_TOOLKIT_API __attribute__((dllimport))
#else
#define DIRECTX_TOOLKIT_API __declspec(dllimport)
#endif
#else
#define DIRECTX_TOOLKIT_API
#endif
#endif

#ifndef IID_GRAPHICS_PPV_ARGS
#define IID_GRAPHICS_PPV_ARGS(x) IID_PPV_ARGS(x)
#endif

namespace DirectX
{
  inline namespace DX11
  {
    class IEffect;
  }

  // Guard de mapeo tipo lock_guard para recursos D3D
  class DIRECTX_TOOLKIT_API MapGuard : public D3D11_MAPPED_SUBRESOURCE
  {
  public:
    MapGuard(_In_ ID3D11DeviceContext* context,
      _In_ ID3D11Resource* resource,
      _In_ unsigned int subresource,
      _In_ D3D11_MAP mapType,
      _In_ unsigned int mapFlags) noexcept(false)
      : mContext(context), mResource(resource), mSubresource(subresource)
    {
      HRESULT hr = mContext->Map(resource, subresource, mapType, mapFlags, this);
      if (FAILED(hr))
      {
        throw std::exception();
      }
    }

    MapGuard(MapGuard&&) = delete;
    MapGuard& operator=(MapGuard&&) = delete;

    MapGuard(const MapGuard&) = delete;
    MapGuard& operator=(const MapGuard&) = delete;

    ~MapGuard()
    {
      mContext->Unmap(mResource, mSubresource);
    }

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-warning-option"
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
#endif

    uint8_t* get() const noexcept
    {
      return static_cast<uint8_t*>(pData);
    }
    uint8_t* get(size_t slice) const noexcept
    {
      return static_cast<uint8_t*>(pData) + (slice * DepthPitch);
    }

    uint8_t* scanline(size_t row) const noexcept
    {
      return static_cast<uint8_t*>(pData) + (row * RowPitch);
    }
    uint8_t* scanline(size_t slice, size_t row) const noexcept
    {
      return static_cast<uint8_t*>(pData) + (slice * DepthPitch) + (row * RowPitch);
    }

#ifdef __clang__
#pragma clang diagnostic pop
#endif

    template<typename T>
    void copy(_In_reads_(count) const T* data, size_t count) noexcept
    {
      std::memcpy(pData, data, count * sizeof(T));
    }

    template<typename T>
    void copy(const T& data) noexcept
    {
      // Requiere que T tenga .data() y .size()
      std::memcpy(pData, data.data(), data.size() * sizeof(typename T::value_type));
    }

  private:
    ID3D11DeviceContext* mContext;
    ID3D11Resource* mResource;
    unsigned int         mSubresource;
  };

  // Poner nombre de depuración a recursos (PIX / D3D11 debug layer)
#if !defined(NO_D3D11_DEBUG_NAME) && ( defined(_DEBUG) || defined(PROFILE) )
  template<UINT TNameLength>
  inline void SetDebugObjectName(_In_ ID3D11DeviceChild* resource, _In_z_ const char(&name)[TNameLength]) noexcept
  {
#if defined(_XBOX_ONE) && defined(_TITLE)
    wchar_t wname[MAX_PATH];
    int result = MultiByteToWideChar(CP_UTF8, 0, name, TNameLength, wname, MAX_PATH);
    if (result > 0) resource->SetName(wname);
#else
    resource->SetPrivateData(WKPDID_D3DDebugObjectName, TNameLength - 1, name);
#endif
  }
#else
  template<UINT TNameLength>
  inline void SetDebugObjectName(_In_ ID3D11DeviceChild*, _In_z_ const char(&)[TNameLength]) noexcept {}
#endif

#if !defined(NO_D3D11_DEBUG_NAME) && ( defined(_DEBUG) || defined(PROFILE) )
  template<UINT TNameLength>
  inline void SetDebugObjectName(_In_ ID3D11DeviceChild* resource, _In_z_ const wchar_t(&name)[TNameLength])
  {
#if defined(_XBOX_ONE) && defined(_TITLE)
    resource->SetName(name);
#else
    char aname[MAX_PATH];
    int result = WideCharToMultiByte(CP_UTF8, 0, name, TNameLength, aname, MAX_PATH, nullptr, nullptr);
    if (result > 0) resource->SetPrivateData(WKPDID_D3DDebugObjectName, TNameLength - 1, aname);
#endif
  }
#else
  template<UINT TNameLength>
  inline void SetDebugObjectName(_In_ ID3D11DeviceChild*, _In_z_ const wchar_t(&)[TNameLength]) {}
#endif

  inline namespace DX11
  {
    // ¿es potencia de 2?
    template<typename T>
    constexpr bool IsPowerOf2(T x) noexcept { return (x != 0) && !(x & (x - 1)); }

    // Alineaciones por potencias de 2
    template<typename T>
    inline T AlignDown(T size, size_t alignment) noexcept
    {
      if (alignment > 0)
      {
        assert(((alignment - 1) & alignment) == 0);
        auto mask = static_cast<T>(alignment - 1);
        return size & ~mask;
      }
      return size;
    }

    template<typename T>
    inline T AlignUp(T size, size_t alignment) noexcept
    {
      if (alignment > 0)
      {
        assert(((alignment - 1) & alignment) == 0);
        auto mask = static_cast<T>(alignment - 1);
        return (size + mask) & ~mask;
      }
      return size;
    }
  }

  // Crea un input layout a partir de un IEffect
  DIRECTX_TOOLKIT_API HRESULT __cdecl CreateInputLayoutFromEffect(
    _In_ ID3D11Device* device,
    _In_ IEffect* effect,
    _In_reads_(count) const D3D11_INPUT_ELEMENT_DESC* desc,
    size_t count,
    _COM_Outptr_ ID3D11InputLayout** pInputLayout) noexcept;

  template<typename T>
  HRESULT CreateInputLayoutFromEffect(_In_ ID3D11Device* device,
    _In_ IEffect* effect,
    _COM_Outptr_ ID3D11InputLayout** pInputLayout) noexcept
  {
    return CreateInputLayoutFromEffect(device, effect, T::InputElements, T::InputElementCount, pInputLayout);
  }
} // namespace DirectX
