#pragma once
#include <d3d11.h>
#include <wincodec.h> // IWIC*
#include <cstdint>

// Simple helper para liberar COM
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p) do { if(p){ (p)->Release(); (p)=nullptr; } } while(0)
#endif

// Carga una textura desde PNG/JPG/etc. con WIC y crea su SRV.
// Usa DXGI_FORMAT_B8G8R8A8_UNORM por compatibilidad directa con WIC (BGRA).
// Si generateMips=true, crea el tex con MipLevels automáticos y llama a GenerateMips.
// Requiere que el device context NO sea nulo cuando generateMips=true.
//
// Parámetros de salida opcionales: width/height.
HRESULT LoadTextureWIC(
  ID3D11Device* device,
  ID3D11DeviceContext* context,
  const wchar_t* filePath,
  ID3D11ShaderResourceView** outSRV,
  UINT* outWidth = nullptr,
  UINT* outHeight = nullptr,
  bool generateMips = true
);
