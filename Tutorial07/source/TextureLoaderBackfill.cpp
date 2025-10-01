// TextureLoaderBackfill.cpp – define las variantes "FromFile" simples
#include <d3d11.h>
#include "DDSTextureLoader.h"
#include "WICTextureLoader.h"

using namespace DirectX;

HRESULT __cdecl DirectX::CreateDDSTextureFromFile(
  ID3D11Device* d3dDevice,
  const wchar_t* szFileName,
  ID3D11Resource** texture,
  ID3D11ShaderResourceView** textureView,
  size_t maxsize,
  DDS_ALPHA_MODE* alphaMode) noexcept
{
  return CreateDDSTextureFromFileEx(
    d3dDevice,
    szFileName,
    maxsize,
    D3D11_USAGE_DEFAULT,
    D3D11_BIND_SHADER_RESOURCE,
    0,      // cpuAccessFlags
    0,      // miscFlags
    DX11::DDS_LOADER_DEFAULT,
    texture,
    textureView,
    alphaMode);
}

HRESULT __cdecl DirectX::CreateWICTextureFromFile(
  ID3D11Device* d3dDevice,
  const wchar_t* szFileName,
  ID3D11Resource** texture,
  ID3D11ShaderResourceView** textureView,
  size_t maxsize) noexcept
{
  return CreateWICTextureFromFileEx(
    d3dDevice,
    szFileName,
    maxsize,
    D3D11_USAGE_DEFAULT,
    D3D11_BIND_SHADER_RESOURCE,
    0,      // cpuAccessFlags
    0,      // miscFlags
    WIC_LOADER_DEFAULT,
    texture,
    textureView);
}
