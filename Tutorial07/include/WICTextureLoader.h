//--------------------------------------------------------------------------------------
// File: WICTextureLoader.h  (versión simplificada/segura para Win32 + D3D11)
//--------------------------------------------------------------------------------------
#pragma once

// Evita macros molestas de Windows
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>      // SAL annotations (_In_, etc.)
#include <d3d11.h>
#include <dxgiformat.h>
#include <cstddef>
#include <cstdint>

#ifndef DIRECTX_TOOLKIT_API
#define DIRECTX_TOOLKIT_API
#endif

namespace DirectX
{
  // Flags del cargador WIC (coinciden con DirectXTK)
  enum WIC_LOADER_FLAGS : uint32_t
  {
    WIC_LOADER_DEFAULT = 0,
    WIC_LOADER_FORCE_SRGB = 0x1,
    WIC_LOADER_IGNORE_SRGB = 0x2,
    WIC_LOADER_SRGB_DEFAULT = 0x4,
    WIC_LOADER_FIT_POW2 = 0x20,
    WIC_LOADER_MAKE_SQUARE = 0x40,
    WIC_LOADER_FORCE_RGBA32 = 0x80,
  };

  // --- Versiones "estándar" (sin autogen mips) ---
  DIRECTX_TOOLKIT_API HRESULT __cdecl CreateWICTextureFromMemory(
    _In_ ID3D11Device* d3dDevice,
    _In_reads_bytes_(wicDataSize) const uint8_t* wicData,
    _In_ size_t wicDataSize,
    _Outptr_opt_ ID3D11Resource** texture,
    _Outptr_opt_ ID3D11ShaderResourceView** textureView,
    _In_ size_t maxsize = 0) noexcept;

  DIRECTX_TOOLKIT_API HRESULT __cdecl CreateWICTextureFromFile(
    _In_ ID3D11Device* d3dDevice,
    _In_z_ const wchar_t* szFileName,
    _Outptr_opt_ ID3D11Resource** texture,
    _Outptr_opt_ ID3D11ShaderResourceView** textureView,
    _In_ size_t maxsize = 0) noexcept;

  // --- Versiones con soporte opcional de auto-gen de mipmaps (requiere d3dContext) ---
  DIRECTX_TOOLKIT_API HRESULT __cdecl CreateWICTextureFromMemory(
    _In_ ID3D11Device* d3dDevice,
    _In_opt_ ID3D11DeviceContext* d3dContext,
    _In_reads_bytes_(wicDataSize) const uint8_t* wicData,
    _In_ size_t wicDataSize,
    _Outptr_opt_ ID3D11Resource** texture,
    _Outptr_opt_ ID3D11ShaderResourceView** textureView,
    _In_ size_t maxsize = 0) noexcept;

  DIRECTX_TOOLKIT_API HRESULT __cdecl CreateWICTextureFromFile(
    _In_ ID3D11Device* d3dDevice,
    _In_opt_ ID3D11DeviceContext* d3dContext,
    _In_z_ const wchar_t* szFileName,
    _Outptr_opt_ ID3D11Resource** texture,
    _Outptr_opt_ ID3D11ShaderResourceView** textureView,
    _In_ size_t maxsize = 0) noexcept;

  // --- Versiones extendidas ---
  DIRECTX_TOOLKIT_API HRESULT __cdecl CreateWICTextureFromMemoryEx(
    _In_ ID3D11Device* d3dDevice,
    _In_reads_bytes_(wicDataSize) const uint8_t* wicData,
    _In_ size_t wicDataSize,
    _In_ size_t maxsize,
    _In_ D3D11_USAGE usage,
    _In_ unsigned int bindFlags,
    _In_ unsigned int cpuAccessFlags,
    _In_ unsigned int miscFlags,
    _In_ WIC_LOADER_FLAGS loadFlags,
    _Outptr_opt_ ID3D11Resource** texture,
    _Outptr_opt_ ID3D11ShaderResourceView** textureView) noexcept;

  DIRECTX_TOOLKIT_API HRESULT __cdecl CreateWICTextureFromFileEx(
    _In_ ID3D11Device* d3dDevice,
    _In_z_ const wchar_t* szFileName,
    _In_ size_t maxsize,
    _In_ D3D11_USAGE usage,
    _In_ unsigned int bindFlags,
    _In_ unsigned int cpuAccessFlags,
    _In_ unsigned int miscFlags,
    _In_ WIC_LOADER_FLAGS loadFlags,
    _Outptr_opt_ ID3D11Resource** texture,
    _Outptr_opt_ ID3D11ShaderResourceView** textureView) noexcept;

  DIRECTX_TOOLKIT_API HRESULT __cdecl CreateWICTextureFromMemoryEx(
    _In_ ID3D11Device* d3dDevice,
    _In_opt_ ID3D11DeviceContext* d3dContext,
    _In_reads_bytes_(wicDataSize) const uint8_t* wicData,
    _In_ size_t wicDataSize,
    _In_ size_t maxsize,
    _In_ D3D11_USAGE usage,
    _In_ unsigned int bindFlags,
    _In_ unsigned int cpuAccessFlags,
    _In_ unsigned int miscFlags,
    _In_ WIC_LOADER_FLAGS loadFlags,
    _Outptr_opt_ ID3D11Resource** texture,
    _Outptr_opt_ ID3D11ShaderResourceView** textureView) noexcept;

  DIRECTX_TOOLKIT_API HRESULT __cdecl CreateWICTextureFromFileEx(
    _In_ ID3D11Device* d3dDevice,
    _In_opt_ ID3D11DeviceContext* d3dContext,
    _In_z_ const wchar_t* szFileName,
    _In_ size_t maxsize,
    _In_ D3D11_USAGE usage,
    _In_ unsigned int bindFlags,
    _In_ unsigned int cpuAccessFlags,
    _In_ unsigned int miscFlags,
    _In_ WIC_LOADER_FLAGS loadFlags,
    _Outptr_opt_ ID3D11Resource** texture,
    _Outptr_opt_ ID3D11ShaderResourceView** textureView) noexcept;
} // namespace DirectX
