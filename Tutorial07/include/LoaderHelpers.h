#pragma once

//--------------------------------------------------------------------------------------
// File: LoaderHelpers.h  (versión corregida, sin pch y sin using-namespace)
// Helper functions for texture loaders and screen grabber
//--------------------------------------------------------------------------------------

// === Win32 / SDK ===
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>        // HANDLE, CreateFile2, etc.
#include <wincodec.h>       // IWICStream (auto_delete_file_wic)
#include <wrl/client.h>     // Microsoft::WRL::ComPtr

// === Direct3D / DXGI ===
#include <d3d11.h>
#include <dxgiformat.h>

// === C/C++ runtime ===
#include <cstdint>
#include <memory>
#include <algorithm>
#include <tuple>            // std::ignore
#include <cfloat>           // FLT_MAX
#include <cmath>            // fabsf
#include <cstring>          // memcpy, memcmp

// --- Nuestros headers ---
#include "DDS.h"
#include "DDSTextureLoader.h"
#include "PlatformHelpers.h"

namespace DirectX
{
  namespace LoaderHelpers
  {
    //--------------------------------------------------------------------------------------
    // Return the BPP for a particular format
    //--------------------------------------------------------------------------------------
    inline size_t BitsPerPixel(_In_ DXGI_FORMAT fmt) noexcept
    {
      switch (fmt)
      {
      case DXGI_FORMAT_R32G32B32A32_TYPELESS:
      case DXGI_FORMAT_R32G32B32A32_FLOAT:
      case DXGI_FORMAT_R32G32B32A32_UINT:
      case DXGI_FORMAT_R32G32B32A32_SINT:
        return 128;

      case DXGI_FORMAT_R32G32B32_TYPELESS:
      case DXGI_FORMAT_R32G32B32_FLOAT:
      case DXGI_FORMAT_R32G32B32_UINT:
      case DXGI_FORMAT_R32G32B32_SINT:
        return 96;

      case DXGI_FORMAT_R16G16B16A16_TYPELESS:
      case DXGI_FORMAT_R16G16B16A16_FLOAT:
      case DXGI_FORMAT_R16G16B16A16_UNORM:
      case DXGI_FORMAT_R16G16B16A16_UINT:
      case DXGI_FORMAT_R16G16B16A16_SNORM:
      case DXGI_FORMAT_R16G16B16A16_SINT:
      case DXGI_FORMAT_R32G32_TYPELESS:
      case DXGI_FORMAT_R32G32_FLOAT:
      case DXGI_FORMAT_R32G32_UINT:
      case DXGI_FORMAT_R32G32_SINT:
      case DXGI_FORMAT_R32G8X24_TYPELESS:
      case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
      case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
      case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
      case DXGI_FORMAT_Y416:
      case DXGI_FORMAT_Y210:
      case DXGI_FORMAT_Y216:
        return 64;

      case DXGI_FORMAT_R10G10B10A2_TYPELESS:
      case DXGI_FORMAT_R10G10B10A2_UNORM:
      case DXGI_FORMAT_R10G10B10A2_UINT:
      case DXGI_FORMAT_R11G11B10_FLOAT:
      case DXGI_FORMAT_R8G8B8A8_TYPELESS:
      case DXGI_FORMAT_R8G8B8A8_UNORM:
      case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
      case DXGI_FORMAT_R8G8B8A8_UINT:
      case DXGI_FORMAT_R8G8B8A8_SNORM:
      case DXGI_FORMAT_R8G8B8A8_SINT:
      case DXGI_FORMAT_R16G16_TYPELESS:
      case DXGI_FORMAT_R16G16_FLOAT:
      case DXGI_FORMAT_R16G16_UNORM:
      case DXGI_FORMAT_R16G16_UINT:
      case DXGI_FORMAT_R16G16_SNORM:
      case DXGI_FORMAT_R16G16_SINT:
      case DXGI_FORMAT_R32_TYPELESS:
      case DXGI_FORMAT_D32_FLOAT:
      case DXGI_FORMAT_R32_FLOAT:
      case DXGI_FORMAT_R32_UINT:
      case DXGI_FORMAT_R32_SINT:
      case DXGI_FORMAT_R24G8_TYPELESS:
      case DXGI_FORMAT_D24_UNORM_S8_UINT:
      case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
      case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
      case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
      case DXGI_FORMAT_R8G8_B8G8_UNORM:
      case DXGI_FORMAT_G8R8_G8B8_UNORM:
      case DXGI_FORMAT_B8G8R8A8_UNORM:
      case DXGI_FORMAT_B8G8R8X8_UNORM:
      case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
      case DXGI_FORMAT_B8G8R8A8_TYPELESS:
      case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
      case DXGI_FORMAT_B8G8R8X8_TYPELESS:
      case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
      case DXGI_FORMAT_AYUV:
      case DXGI_FORMAT_Y410:
      case DXGI_FORMAT_YUY2:
#if (defined(_XBOX_ONE) && defined(_TITLE)) || defined(_GAMING_XBOX)
      case DXGI_FORMAT_R10G10B10_7E3_A2_FLOAT:
      case DXGI_FORMAT_R10G10B10_6E4_A2_FLOAT:
      case DXGI_FORMAT_R10G10B10_SNORM_A2_UNORM:
#endif
        return 32;

      case DXGI_FORMAT_P010:
      case DXGI_FORMAT_P016:
#if (_WIN32_WINNT >= _WIN32_WINNT_WIN10)
      case DXGI_FORMAT_V408:
#endif
#if (defined(_XBOX_ONE) && defined(_TITLE)) || defined(_GAMING_XBOX)
      case DXGI_FORMAT_D16_UNORM_S8_UINT:
      case DXGI_FORMAT_R16_UNORM_X8_TYPELESS:
      case DXGI_FORMAT_X16_TYPELESS_G8_UINT:
#endif
        return 24;

      case DXGI_FORMAT_R8G8_TYPELESS:
      case DXGI_FORMAT_R8G8_UNORM:
      case DXGI_FORMAT_R8G8_UINT:
      case DXGI_FORMAT_R8G8_SNORM:
      case DXGI_FORMAT_R8G8_SINT:
      case DXGI_FORMAT_R16_TYPELESS:
      case DXGI_FORMAT_R16_FLOAT:
      case DXGI_FORMAT_D16_UNORM:
      case DXGI_FORMAT_R16_UNORM:
      case DXGI_FORMAT_R16_UINT:
      case DXGI_FORMAT_R16_SNORM:
      case DXGI_FORMAT_R16_SINT:
      case DXGI_FORMAT_B5G6R5_UNORM:
      case DXGI_FORMAT_B5G5R5A1_UNORM:
      case DXGI_FORMAT_A8P8:
      case DXGI_FORMAT_B4G4R4A4_UNORM:
#if (_WIN32_WINNT >= _WIN32_WINNT_WIN10)
      case DXGI_FORMAT_P208:
      case DXGI_FORMAT_V208:
#endif
        return 16;

      case DXGI_FORMAT_NV12:
      case DXGI_FORMAT_420_OPAQUE:
      case DXGI_FORMAT_NV11:
        return 12;

      case DXGI_FORMAT_R8_TYPELESS:
      case DXGI_FORMAT_R8_UNORM:
      case DXGI_FORMAT_R8_UINT:
      case DXGI_FORMAT_R8_SNORM:
      case DXGI_FORMAT_R8_SINT:
      case DXGI_FORMAT_A8_UNORM:
      case DXGI_FORMAT_BC2_TYPELESS:
      case DXGI_FORMAT_BC2_UNORM:
      case DXGI_FORMAT_BC2_UNORM_SRGB:
      case DXGI_FORMAT_BC3_TYPELESS:
      case DXGI_FORMAT_BC3_UNORM:
      case DXGI_FORMAT_BC3_UNORM_SRGB:
      case DXGI_FORMAT_BC5_TYPELESS:
      case DXGI_FORMAT_BC5_UNORM:
      case DXGI_FORMAT_BC5_SNORM:
      case DXGI_FORMAT_BC6H_TYPELESS:
      case DXGI_FORMAT_BC6H_UF16:
      case DXGI_FORMAT_BC6H_SF16:
      case DXGI_FORMAT_BC7_TYPELESS:
      case DXGI_FORMAT_BC7_UNORM:
      case DXGI_FORMAT_BC7_UNORM_SRGB:
      case DXGI_FORMAT_AI44:
      case DXGI_FORMAT_IA44:
      case DXGI_FORMAT_P8:
#if (defined(_XBOX_ONE) && defined(_TITLE)) || defined(_GAMING_XBOX)
      case DXGI_FORMAT_R4G4_UNORM:
#endif
        return 8;

      case DXGI_FORMAT_R1_UNORM:
        return 1;

      case DXGI_FORMAT_BC1_TYPELESS:
      case DXGI_FORMAT_BC1_UNORM:
      case DXGI_FORMAT_BC1_UNORM_SRGB:
      case DXGI_FORMAT_BC4_TYPELESS:
      case DXGI_FORMAT_BC4_UNORM:
      case DXGI_FORMAT_BC4_SNORM:
        return 4;

      case DXGI_FORMAT_UNKNOWN:
      case DXGI_FORMAT_FORCE_UINT:
      default:
        return 0;
      }
    }

    //--------------------------------------------------------------------------------------
    inline DXGI_FORMAT MakeSRGB(_In_ DXGI_FORMAT format) noexcept
    {
      switch (format)
      {
      case DXGI_FORMAT_R8G8B8A8_UNORM:   return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
      case DXGI_FORMAT_BC1_UNORM:        return DXGI_FORMAT_BC1_UNORM_SRGB;
      case DXGI_FORMAT_BC2_UNORM:        return DXGI_FORMAT_BC2_UNORM_SRGB;
      case DXGI_FORMAT_BC3_UNORM:        return DXGI_FORMAT_BC3_UNORM_SRGB;
      case DXGI_FORMAT_B8G8R8A8_UNORM:   return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
      case DXGI_FORMAT_B8G8R8X8_UNORM:   return DXGI_FORMAT_B8G8R8X8_UNORM_SRGB;
      case DXGI_FORMAT_BC7_UNORM:        return DXGI_FORMAT_BC7_UNORM_SRGB;
      default:                           return format;
      }
    }

    //--------------------------------------------------------------------------------------
    inline DXGI_FORMAT MakeLinear(_In_ DXGI_FORMAT format) noexcept
    {
      switch (format)
      {
      case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:  return DXGI_FORMAT_R8G8B8A8_UNORM;
      case DXGI_FORMAT_BC1_UNORM_SRGB:       return DXGI_FORMAT_BC1_UNORM;
      case DXGI_FORMAT_BC2_UNORM_SRGB:       return DXGI_FORMAT_BC2_UNORM;
      case DXGI_FORMAT_BC3_UNORM_SRGB:       return DXGI_FORMAT_BC3_UNORM;
      case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:  return DXGI_FORMAT_B8G8R8A8_UNORM;
      case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:  return DXGI_FORMAT_B8G8R8X8_UNORM;
      case DXGI_FORMAT_BC7_UNORM_SRGB:       return DXGI_FORMAT_BC7_UNORM;
      default:                                return format;
      }
    }

    //--------------------------------------------------------------------------------------
    inline bool IsCompressed(_In_ DXGI_FORMAT fmt) noexcept
    {
      switch (fmt)
      {
      case DXGI_FORMAT_BC1_TYPELESS:
      case DXGI_FORMAT_BC1_UNORM:
      case DXGI_FORMAT_BC1_UNORM_SRGB:
      case DXGI_FORMAT_BC2_TYPELESS:
      case DXGI_FORMAT_BC2_UNORM:
      case DXGI_FORMAT_BC2_UNORM_SRGB:
      case DXGI_FORMAT_BC3_TYPELESS:
      case DXGI_FORMAT_BC3_UNORM:
      case DXGI_FORMAT_BC3_UNORM_SRGB:
      case DXGI_FORMAT_BC4_TYPELESS:
      case DXGI_FORMAT_BC4_UNORM:
      case DXGI_FORMAT_BC4_SNORM:
      case DXGI_FORMAT_BC5_TYPELESS:
      case DXGI_FORMAT_BC5_UNORM:
      case DXGI_FORMAT_BC5_SNORM:
      case DXGI_FORMAT_BC6H_TYPELESS:
      case DXGI_FORMAT_BC6H_UF16:
      case DXGI_FORMAT_BC6H_SF16:
      case DXGI_FORMAT_BC7_TYPELESS:
      case DXGI_FORMAT_BC7_UNORM:
      case DXGI_FORMAT_BC7_UNORM_SRGB:
        return true;
      default:
        return false;
      }
    }

    //--------------------------------------------------------------------------------------
    inline DXGI_FORMAT EnsureNotTypeless(DXGI_FORMAT fmt) noexcept
    {
      // Assumes UNORM or FLOAT; doesn't use UINT or SINT
      switch (fmt)
      {
      case DXGI_FORMAT_R32G32B32A32_TYPELESS: return DXGI_FORMAT_R32G32B32A32_FLOAT;
      case DXGI_FORMAT_R32G32B32_TYPELESS:    return DXGI_FORMAT_R32G32B32_FLOAT;
      case DXGI_FORMAT_R16G16B16A16_TYPELESS: return DXGI_FORMAT_R16G16B16A16_UNORM;
      case DXGI_FORMAT_R32G32_TYPELESS:       return DXGI_FORMAT_R32G32_FLOAT;
      case DXGI_FORMAT_R10G10B10A2_TYPELESS:  return DXGI_FORMAT_R10G10B10A2_UNORM;
      case DXGI_FORMAT_R8G8B8A8_TYPELESS:     return DXGI_FORMAT_R8G8B8A8_UNORM;
      case DXGI_FORMAT_R16G16_TYPELESS:       return DXGI_FORMAT_R16G16_UNORM;
      case DXGI_FORMAT_R32_TYPELESS:          return DXGI_FORMAT_R32_FLOAT;
      case DXGI_FORMAT_R8G8_TYPELESS:         return DXGI_FORMAT_R8G8_UNORM;
      case DXGI_FORMAT_R16_TYPELESS:          return DXGI_FORMAT_R16_UNORM;
      case DXGI_FORMAT_R8_TYPELESS:           return DXGI_FORMAT_R8_UNORM;
      case DXGI_FORMAT_BC1_TYPELESS:          return DXGI_FORMAT_BC1_UNORM;
      case DXGI_FORMAT_BC2_TYPELESS:          return DXGI_FORMAT_BC2_UNORM;
      case DXGI_FORMAT_BC3_TYPELESS:          return DXGI_FORMAT_BC3_UNORM;
      case DXGI_FORMAT_BC4_TYPELESS:          return DXGI_FORMAT_BC4_UNORM;
      case DXGI_FORMAT_BC5_TYPELESS:          return DXGI_FORMAT_BC5_UNORM;
      case DXGI_FORMAT_B8G8R8A8_TYPELESS:     return DXGI_FORMAT_B8G8R8A8_UNORM;
      case DXGI_FORMAT_B8G8R8X8_TYPELESS:     return DXGI_FORMAT_B8G8R8X8_UNORM;
      case DXGI_FORMAT_BC7_TYPELESS:          return DXGI_FORMAT_BC7_UNORM;
      default:                                return fmt;
      }
    }

    //--------------------------------------------------------------------------------------
    inline HRESULT LoadTextureDataFromMemory(
      _In_reads_(ddsDataSize) const uint8_t* ddsData,
      size_t ddsDataSize,
      const DDS_HEADER** header,
      const uint8_t** bitData,
      size_t* bitSize) noexcept
    {
      if (!header || !bitData || !bitSize)
        return E_POINTER;

      *bitSize = 0;

      if (ddsDataSize > UINT32_MAX || ddsDataSize < DDS_MIN_HEADER_SIZE)
        return E_FAIL;

      // Magic "DDS "
      const auto dwMagicNumber = *reinterpret_cast<const uint32_t*>(ddsData);
      if (dwMagicNumber != DDS_MAGIC)
        return E_FAIL;

      auto hdr = reinterpret_cast<const DDS_HEADER*>(ddsData + sizeof(uint32_t));

      // Validate header
      if (hdr->size != sizeof(DDS_HEADER) || hdr->ddspf.size != sizeof(DDS_PIXELFORMAT))
        return E_FAIL;

      // DX10 extension?
      bool bDXT10Header = false;
      if ((hdr->ddspf.flags & DDS_FOURCC) &&
        (MAKEFOURCC('D', 'X', '1', '0') == hdr->ddspf.fourCC))
      {
        if (ddsDataSize < DDS_DX10_HEADER_SIZE)
          return E_FAIL;

        bDXT10Header = true;
      }

      *header = hdr;
      auto offset = DDS_MIN_HEADER_SIZE + (bDXT10Header ? sizeof(DDS_HEADER_DXT10) : 0u);
      *bitData = ddsData + offset;
      *bitSize = ddsDataSize - offset;

      return S_OK;
    }

    //--------------------------------------------------------------------------------------
    inline HRESULT LoadTextureDataFromFile(
      _In_z_ const wchar_t* fileName,
      std::unique_ptr<uint8_t[]>& ddsData,
      const DDS_HEADER** header,
      const uint8_t** bitData,
      size_t* bitSize) noexcept
    {
      if (!header || !bitData || !bitSize)
        return E_POINTER;

      *bitSize = 0;

      // open the file
      ScopedHandle hFile(safe_handle(CreateFile2(
        fileName, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING, nullptr)));
      if (!hFile)
        return HRESULT_FROM_WIN32(GetLastError());

      // size
      FILE_STANDARD_INFO fileInfo;
      if (!GetFileInformationByHandleEx(hFile.get(), FileStandardInfo, &fileInfo, sizeof(fileInfo)))
        return HRESULT_FROM_WIN32(GetLastError());

      if (fileInfo.EndOfFile.HighPart > 0 || fileInfo.EndOfFile.LowPart < DDS_MIN_HEADER_SIZE)
        return E_FAIL;

      // read
      ddsData.reset(new (std::nothrow) uint8_t[fileInfo.EndOfFile.LowPart]);
      if (!ddsData)
        return E_OUTOFMEMORY;

      DWORD bytesRead = 0;
      if (!ReadFile(hFile.get(), ddsData.get(), fileInfo.EndOfFile.LowPart, &bytesRead, nullptr))
      {
        ddsData.reset();
        return HRESULT_FROM_WIN32(GetLastError());
      }
      if (bytesRead < fileInfo.EndOfFile.LowPart)
      {
        ddsData.reset();
        return E_FAIL;
      }

      // Magic "DDS "
      const auto dwMagicNumber = *reinterpret_cast<const uint32_t*>(ddsData.get());
      if (dwMagicNumber != DDS_MAGIC)
      {
        ddsData.reset();
        return E_FAIL;
      }

      auto hdr = reinterpret_cast<const DDS_HEADER*>(ddsData.get() + sizeof(uint32_t));

      if (hdr->size != sizeof(DDS_HEADER) || hdr->ddspf.size != sizeof(DDS_PIXELFORMAT))
      {
        ddsData.reset();
        return E_FAIL;
      }

      bool bDXT10Header = false;
      if ((hdr->ddspf.flags & DDS_FOURCC) &&
        (MAKEFOURCC('D', 'X', '1', '0') == hdr->ddspf.fourCC))
      {
        if (fileInfo.EndOfFile.LowPart < DDS_DX10_HEADER_SIZE)
        {
          ddsData.reset();
          return E_FAIL;
        }
        bDXT10Header = true;
      }

      *header = hdr;
      auto offset = DDS_MIN_HEADER_SIZE + (bDXT10Header ? sizeof(DDS_HEADER_DXT10) : 0u);
      *bitData = ddsData.get() + offset;
      *bitSize = fileInfo.EndOfFile.LowPart - offset;

      return S_OK;
    }

    //--------------------------------------------------------------------------------------
    // Get surface information for a particular format
    //--------------------------------------------------------------------------------------
    inline HRESULT GetSurfaceInfo(
      _In_ size_t width,
      _In_ size_t height,
      _In_ DXGI_FORMAT fmt,
      _Out_opt_ size_t* outNumBytes,
      _Out_opt_ size_t* outRowBytes,
      _Out_opt_ size_t* outNumRows) noexcept
    {
      uint64_t numBytes = 0;
      uint64_t rowBytes = 0;
      uint64_t numRows = 0;

      bool bc = false;
      bool packed = false;
      bool planar = false;
      size_t bpe = 0;

      switch (fmt)
      {
      case DXGI_FORMAT_UNKNOWN:
        return E_INVALIDARG;

      case DXGI_FORMAT_BC1_TYPELESS:
      case DXGI_FORMAT_BC1_UNORM:
      case DXGI_FORMAT_BC1_UNORM_SRGB:
      case DXGI_FORMAT_BC4_TYPELESS:
      case DXGI_FORMAT_BC4_UNORM:
      case DXGI_FORMAT_BC4_SNORM:
        bc = true; bpe = 8; break;

      case DXGI_FORMAT_BC2_TYPELESS:
      case DXGI_FORMAT_BC2_UNORM:
      case DXGI_FORMAT_BC2_UNORM_SRGB:
      case DXGI_FORMAT_BC3_TYPELESS:
      case DXGI_FORMAT_BC3_UNORM:
      case DXGI_FORMAT_BC3_UNORM_SRGB:
      case DXGI_FORMAT_BC5_TYPELESS:
      case DXGI_FORMAT_BC5_UNORM:
      case DXGI_FORMAT_BC5_SNORM:
      case DXGI_FORMAT_BC6H_TYPELESS:
      case DXGI_FORMAT_BC6H_UF16:
      case DXGI_FORMAT_BC6H_SF16:
      case DXGI_FORMAT_BC7_TYPELESS:
      case DXGI_FORMAT_BC7_UNORM:
      case DXGI_FORMAT_BC7_UNORM_SRGB:
        bc = true; bpe = 16; break;

      case DXGI_FORMAT_R8G8_B8G8_UNORM:
      case DXGI_FORMAT_G8R8_G8B8_UNORM:
      case DXGI_FORMAT_YUY2:
        packed = true; bpe = 4; break;

      case DXGI_FORMAT_Y210:
      case DXGI_FORMAT_Y216:
        packed = true; bpe = 8; break;

      case DXGI_FORMAT_NV12:
      case DXGI_FORMAT_420_OPAQUE:
        if ((height % 2) != 0) return E_INVALIDARG;
        planar = true; bpe = 2; break;

#if (_WIN32_WINNT >= _WIN32_WINNT_WIN10)
      case DXGI_FORMAT_P208:
        planar = true; bpe = 2; break;
#endif

      case DXGI_FORMAT_P010:
      case DXGI_FORMAT_P016:
        if ((height % 2) != 0) return E_INVALIDARG;
        planar = true; bpe = 4; break;

      default:
        break;
      }

      if (bc)
      {
        uint64_t numBlocksWide = (width > 0) ? std::max<uint64_t>(1u, (uint64_t(width) + 3u) / 4u) : 0;
        uint64_t numBlocksHigh = (height > 0) ? std::max<uint64_t>(1u, (uint64_t(height) + 3u) / 4u) : 0;
        rowBytes = numBlocksWide * bpe;
        numRows = numBlocksHigh;
        numBytes = rowBytes * numBlocksHigh;
      }
      else if (packed)
      {
        rowBytes = ((uint64_t(width) + 1u) >> 1) * bpe;
        numRows = uint64_t(height);
        numBytes = rowBytes * height;
      }
      else if (fmt == DXGI_FORMAT_NV11)
      {
        rowBytes = ((uint64_t(width) + 3u) >> 2) * 4u;
        numRows = uint64_t(height) * 2u; // simplificación de D3D
        numBytes = rowBytes * numRows;
      }
      else if (planar)
      {
        rowBytes = ((uint64_t(width) + 1u) >> 1) * bpe;
        numBytes = (rowBytes * uint64_t(height)) + ((rowBytes * uint64_t(height) + 1u) >> 1);
        numRows = height + ((uint64_t(height) + 1u) >> 1);
      }
      else
      {
        const size_t bpp = BitsPerPixel(fmt);
        if (!bpp) return E_INVALIDARG;

        rowBytes = (uint64_t(width) * bpp + 7u) / 8u; // redondeo a byte
        numRows = uint64_t(height);
        numBytes = rowBytes * height;
      }

#if defined(_M_IX86) || defined(_M_ARM) || defined(_M_HYBRID_X86_ARM64)
      static_assert(sizeof(size_t) == 4, "Not a 32-bit platform!");
      if (numBytes > UINT32_MAX || rowBytes > UINT32_MAX || numRows > UINT32_MAX)
        return HRESULT_FROM_WIN32(ERROR_ARITHMETIC_OVERFLOW);
#else
      static_assert(sizeof(size_t) == 8, "Not a 64-bit platform!");
#endif

      if (outNumBytes) *outNumBytes = static_cast<size_t>(numBytes);
      if (outRowBytes) *outRowBytes = static_cast<size_t>(rowBytes);
      if (outNumRows)  *outNumRows = static_cast<size_t>(numRows);

      return S_OK;
    }

    //--------------------------------------------------------------------------------------
#define ISBITMASK( r,g,b,a ) ( ddpf.RBitMask == r && ddpf.GBitMask == g && ddpf.BBitMask == b && ddpf.ABitMask == a )

    inline DXGI_FORMAT GetDXGIFormat(const DDS_PIXELFORMAT& ddpf) noexcept
    {
      if (ddpf.flags & DDS_RGB)
      {
        switch (ddpf.RGBBitCount)
        {
        case 32:
          if (ISBITMASK(0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000)) return DXGI_FORMAT_R8G8B8A8_UNORM;
          if (ISBITMASK(0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000)) return DXGI_FORMAT_B8G8R8A8_UNORM;
          if (ISBITMASK(0x00ff0000, 0x0000ff00, 0x000000ff, 0))          return DXGI_FORMAT_B8G8R8X8_UNORM;
          if (ISBITMASK(0x3ff00000, 0x000ffc00, 0x000003ff, 0xc0000000)) return DXGI_FORMAT_R10G10B10A2_UNORM;
          if (ISBITMASK(0x0000ffff, 0xffff0000, 0, 0))                   return DXGI_FORMAT_R16G16_UNORM;
          if (ISBITMASK(0xffffffff, 0, 0, 0))                            return DXGI_FORMAT_R32_FLOAT;
          break;

        case 24:
          break; // no 24bpp DXGI

        case 16:
          if (ISBITMASK(0x7c00, 0x03e0, 0x001f, 0x8000)) return DXGI_FORMAT_B5G5R5A1_UNORM;
          if (ISBITMASK(0xf800, 0x07e0, 0x001f, 0))      return DXGI_FORMAT_B5G6R5_UNORM;
          if (ISBITMASK(0x0f00, 0x00f0, 0x000f, 0xf000)) return DXGI_FORMAT_B4G4R4A4_UNORM;
          if (ISBITMASK(0x00ff, 0, 0, 0xff00))           return DXGI_FORMAT_R8G8_UNORM;
          if (ISBITMASK(0xffff, 0, 0, 0))                return DXGI_FORMAT_R16_UNORM;
          break;

        case 8:
          if (ISBITMASK(0xff, 0, 0, 0)) return DXGI_FORMAT_R8_UNORM;
          break;

        default:
          return DXGI_FORMAT_UNKNOWN;
        }
      }
      else if (ddpf.flags & DDS_LUMINANCE)
      {
        switch (ddpf.RGBBitCount)
        {
        case 16:
          if (ISBITMASK(0xffff, 0, 0, 0))          return DXGI_FORMAT_R16_UNORM;
          if (ISBITMASK(0x00ff, 0, 0, 0xff00))      return DXGI_FORMAT_R8G8_UNORM;
          break;
        case 8:
          if (ISBITMASK(0xff, 0, 0, 0))            return DXGI_FORMAT_R8_UNORM;
          if (ISBITMASK(0x00ff, 0, 0, 0xff00))      return DXGI_FORMAT_R8G8_UNORM;
          break;
        default:
          return DXGI_FORMAT_UNKNOWN;
        }
      }
      else if (ddpf.flags & DDS_ALPHA)
      {
        if (ddpf.RGBBitCount == 8) return DXGI_FORMAT_A8_UNORM;
      }
      else if (ddpf.flags & DDS_BUMPDUDV)
      {
        switch (ddpf.RGBBitCount)
        {
        case 32:
          if (ISBITMASK(0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000)) return DXGI_FORMAT_R8G8B8A8_SNORM;
          if (ISBITMASK(0x0000ffff, 0xffff0000, 0, 0))                   return DXGI_FORMAT_R16G16_SNORM;
          break;
        case 16:
          if (ISBITMASK(0x00ff, 0xff00, 0, 0))                           return DXGI_FORMAT_R8G8_SNORM;
          break;
        default:
          return DXGI_FORMAT_UNKNOWN;
        }
      }
      else if (ddpf.flags & DDS_FOURCC)
      {
        if (MAKEFOURCC('D', 'X', 'T', '1') == ddpf.fourCC) return DXGI_FORMAT_BC1_UNORM;
        if (MAKEFOURCC('D', 'X', 'T', '3') == ddpf.fourCC) return DXGI_FORMAT_BC2_UNORM;
        if (MAKEFOURCC('D', 'X', 'T', '5') == ddpf.fourCC) return DXGI_FORMAT_BC3_UNORM;
        if (MAKEFOURCC('D', 'X', 'T', '2') == ddpf.fourCC) return DXGI_FORMAT_BC2_UNORM;
        if (MAKEFOURCC('D', 'X', 'T', '4') == ddpf.fourCC) return DXGI_FORMAT_BC3_UNORM;

        if (MAKEFOURCC('A', 'T', 'I', '1') == ddpf.fourCC) return DXGI_FORMAT_BC4_UNORM;
        if (MAKEFOURCC('B', 'C', '4', 'U') == ddpf.fourCC) return DXGI_FORMAT_BC4_UNORM;
        if (MAKEFOURCC('B', 'C', '4', 'S') == ddpf.fourCC) return DXGI_FORMAT_BC4_SNORM;

        if (MAKEFOURCC('A', 'T', 'I', '2') == ddpf.fourCC) return DXGI_FORMAT_BC5_UNORM;
        if (MAKEFOURCC('B', 'C', '5', 'U') == ddpf.fourCC) return DXGI_FORMAT_BC5_UNORM;
        if (MAKEFOURCC('B', 'C', '5', 'S') == ddpf.fourCC) return DXGI_FORMAT_BC5_SNORM;

        if (MAKEFOURCC('R', 'G', 'B', 'G') == ddpf.fourCC) return DXGI_FORMAT_R8G8_B8G8_UNORM;
        if (MAKEFOURCC('G', 'R', 'G', 'B') == ddpf.fourCC) return DXGI_FORMAT_G8R8_G8B8_UNORM;

        if (MAKEFOURCC('Y', 'U', 'Y', '2') == ddpf.fourCC) return DXGI_FORMAT_YUY2;

        switch (ddpf.fourCC)
        {
        case 36:  return DXGI_FORMAT_R16G16B16A16_UNORM; // D3DFMT_A16B16G16R16
        case 110: return DXGI_FORMAT_R16G16B16A16_SNORM; // D3DFMT_Q16W16V16U16
        case 111: return DXGI_FORMAT_R16_FLOAT;          // D3DFMT_R16F
        case 112: return DXGI_FORMAT_R16G16_FLOAT;       // D3DFMT_G16R16F
        case 113: return DXGI_FORMAT_R16G16B16A16_FLOAT; // D3DFMT_A16B16G16R16F
        case 114: return DXGI_FORMAT_R32_FLOAT;          // D3DFMT_R32F
        case 115: return DXGI_FORMAT_R32G32_FLOAT;       // D3DFMT_G32R32F
        case 116: return DXGI_FORMAT_R32G32B32A32_FLOAT; // D3DFMT_A32B32G32R32F
        default:  return DXGI_FORMAT_UNKNOWN;
        }
      }

      return DXGI_FORMAT_UNKNOWN;
    }

#undef ISBITMASK

    //--------------------------------------------------------------------------------------
    inline DirectX::DDS_ALPHA_MODE GetAlphaMode(_In_ const DDS_HEADER* header) noexcept
    {
      if (header->ddspf.flags & DDS_FOURCC)
      {
        if (MAKEFOURCC('D', 'X', '1', '0') == header->ddspf.fourCC)
        {
          auto d3d10ext = reinterpret_cast<const DDS_HEADER_DXT10*>(reinterpret_cast<const uint8_t*>(header) + sizeof(DDS_HEADER));
          const auto mode = static_cast<DDS_ALPHA_MODE>(d3d10ext->miscFlags2 & DDS_MISC_FLAGS2_ALPHA_MODE_MASK);
          switch (mode)
          {
          case DDS_ALPHA_MODE_STRAIGHT:
          case DDS_ALPHA_MODE_PREMULTIPLIED:
          case DDS_ALPHA_MODE_OPAQUE:
          case DDS_ALPHA_MODE_CUSTOM:
            return mode;
          default:
            break;
          }
        }
        else if ((MAKEFOURCC('D', 'X', 'T', '2') == header->ddspf.fourCC) ||
          (MAKEFOURCC('D', 'X', 'T', '4') == header->ddspf.fourCC))
        {
          return DDS_ALPHA_MODE_PREMULTIPLIED;
        }
      }
      return DDS_ALPHA_MODE_UNKNOWN;
    }

    //--------------------------------------------------------------------------------------
    class auto_delete_file
    {
    public:
      explicit auto_delete_file(HANDLE hFile) noexcept : m_handle(hFile) {}

      auto_delete_file(const auto_delete_file&) = delete;
      auto_delete_file& operator=(const auto_delete_file&) = delete;
      auto_delete_file(const auto_delete_file&&) = delete;
      auto_delete_file& operator=(const auto_delete_file&&) = delete;

      ~auto_delete_file()
      {
        if (m_handle)
        {
          FILE_DISPOSITION_INFO info = {};
          info.DeleteFile = TRUE;
          std::ignore = SetFileInformationByHandle(m_handle, FileDispositionInfo, &info, sizeof(info));
        }
      }

      void clear() noexcept { m_handle = nullptr; }

    private:
      HANDLE m_handle;
    };

    class auto_delete_file_wic
    {
    public:
      auto_delete_file_wic(Microsoft::WRL::ComPtr<IWICStream>& hFile, LPCWSTR szFile) noexcept
        : m_filename(szFile), m_handle(hFile) {
      }

      auto_delete_file_wic(const auto_delete_file_wic&) = delete;
      auto_delete_file_wic& operator=(const auto_delete_file_wic&) = delete;
      auto_delete_file_wic(const auto_delete_file_wic&&) = delete;
      auto_delete_file_wic& operator=(const auto_delete_file_wic&&) = delete;

      ~auto_delete_file_wic()
      {
        if (m_filename)
        {
          m_handle.Reset();
          DeleteFileW(m_filename);
        }
      }

      void clear() noexcept { m_filename = nullptr; }

    private:
      LPCWSTR m_filename;
      Microsoft::WRL::ComPtr<IWICStream>& m_handle;
    };

    inline uint32_t CountMips(uint32_t width, uint32_t height) noexcept
    {
      if (width == 0 || height == 0) return 0;
      uint32_t count = 1;
      while (width > 1 || height > 1) { width >>= 1; height >>= 1; ++count; }
      return count;
    }

    inline void FitPowerOf2(UINT origx, UINT origy, _Inout_ UINT& targetx, _Inout_ UINT& targety, size_t maxsize)
    {
      const float origAR = float(origx) / float(origy);

      if (origx > origy)
      {
        size_t x;
        for (x = maxsize; x > 1; x >>= 1) { if (x <= targetx) break; }
        targetx = UINT(x);

        float bestScore = FLT_MAX;
        for (size_t y = maxsize; y > 0; y >>= 1)
        {
          const float score = std::fabs((float(x) / float(y)) - origAR);
          if (score < bestScore) { bestScore = score; targety = UINT(y); }
        }
      }
      else
      {
        size_t y;
        for (y = maxsize; y > 1; y >>= 1) { if (y <= targety) break; }
        targety = UINT(y);

        float bestScore = FLT_MAX;
        for (size_t x = maxsize; x > 0; x >>= 1)
        {
          const float score = std::fabs((float(x) / float(y)) - origAR);
          if (score < bestScore) { bestScore = score; targetx = UINT(x); }
        }
      }
    }
  } // namespace LoaderHelpers
} // namespace DirectX
