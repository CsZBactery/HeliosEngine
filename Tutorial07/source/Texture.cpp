// Texture.cpp
#include "../include/Prerequisites.h"   // trae SAFE_RELEASE, MESSAGE/ERROR
#include "../include/Texture.h"
#include "../include/Device.h"
#include "../include/DeviceContext.h"

#include <string>
#include <cctype>   // tolower
#include <cstring>  // strlen

// Loaders modernos (DirectXTK). Si los pusiste en otra carpeta, ajusta la ruta.
#include "../include/DDSTextureLoader.h"
#include "WICTextureLoader.h"

// ---- Helper: mapear formatos TYPELESS a uno "typed" para SRV ----
static DXGI_FORMAT ChooseSrvFormat(DXGI_FORMAT fmt) {
  switch (fmt) {
  case DXGI_FORMAT_R8G8B8A8_TYPELESS: return DXGI_FORMAT_R8G8B8A8_UNORM;
  case DXGI_FORMAT_B8G8R8A8_TYPELESS: return DXGI_FORMAT_B8G8R8A8_UNORM;
  case DXGI_FORMAT_R16_TYPELESS:      return DXGI_FORMAT_R16_UNORM;
  case DXGI_FORMAT_R32_TYPELESS:      return DXGI_FORMAT_R32_FLOAT;
  case DXGI_FORMAT_R24G8_TYPELESS:    return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
  default:                             return fmt;
  }
}

// ------------------------------------------------------------
// init(): cargar desde archivo (DDS/WIC) usando DirectXTK
// ------------------------------------------------------------
HRESULT Texture::init(Device& device,
  const std::string& textureName,
  ExtensionType /*extensionType*/)
{
  if (!device.m_device) {
    ERROR(Texture, init_file, L"Device is null.");
    return E_POINTER;
  }

  destroy();
  m_textureName = textureName;

  // Path en wide-string
  std::wstring wpath(textureName.begin(), textureName.end());

  // Elegir loader según extensión (muy simple)
  auto endsWith = [](const std::string& s, const char* suf) {
    const size_t n = s.size(), m = std::strlen(suf);
    if (m > n) return false;
    for (size_t i = 0; i < m; ++i) {
      char a = (char)std::tolower(s[n - m + i]);
      char b = (char)std::tolower(suf[i]);
      if (a != b) return false;
    }
    return true;
    };

  ID3D11Resource* res = nullptr;
  HRESULT hr = E_FAIL;

  if (endsWith(textureName, ".dds")) {
    hr = DirectX::CreateDDSTextureFromFile(
      device.m_device, wpath.c_str(),
      &res, &m_textureFromImg);   // crea SRV
  }
  else {
    hr = DirectX::CreateWICTextureFromFile(
      device.m_device, wpath.c_str(),
      &res, &m_textureFromImg);   // crea SRV
  }

  if (FAILED(hr)) {
    ERROR(Texture, init_file, L"Failed to load texture with DirectXTK loader.");
    return hr;
  }

  // Guardar el ID3D11Texture2D subyacente (opcional)
  if (res) {
    res->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&m_texture));
    res->Release();
  }

  return S_OK;
}

// ------------------------------------------------------------
// init(): crear textura vacía con parámetros
// ------------------------------------------------------------
HRESULT Texture::init(Device& device,
  unsigned int width,
  unsigned int height,
  DXGI_FORMAT Format,
  unsigned int BindFlags,
  unsigned int sampleCount,
  unsigned int qualityLevels)
{
  if (!device.m_device) {
    ERROR(Texture, init_params, L"Device is null.");
    return E_POINTER;
  }
  if (width == 0 || height == 0) {
    ERROR(Texture, init_params, L"Width/Height must be > 0.");
    return E_INVALIDARG;
  }

  destroy();
  m_textureName.clear();

  D3D11_TEXTURE2D_DESC desc = {};
  desc.Width = width;
  desc.Height = height;
  desc.MipLevels = 1;         // si quieres mips: 0 + GenerateMips
  desc.ArraySize = 1;
  desc.Format = Format;
  desc.SampleDesc.Count = (sampleCount == 0) ? 1u : sampleCount;
  desc.SampleDesc.Quality = qualityLevels;
  desc.Usage = D3D11_USAGE_DEFAULT;
  desc.BindFlags = BindFlags; // p.ej. D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET
  desc.CPUAccessFlags = 0;
  desc.MiscFlags = 0;

  HRESULT hr = device.m_device->CreateTexture2D(&desc, nullptr, &m_texture);
  if (FAILED(hr)) {
    ERROR(Texture, init_params, L"CreateTexture2D failed.");
    return hr;
  }

  // Crear SRV si se va a samplear en shaders
  if (BindFlags & D3D11_BIND_SHADER_RESOURCE) {
    D3D11_SHADER_RESOURCE_VIEW_DESC srvd = {};
    srvd.Format = ChooseSrvFormat(Format);
    if (desc.SampleDesc.Count > 1) {
      srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;
    }
    else {
      srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
      srvd.Texture2D.MostDetailedMip = 0;
      srvd.Texture2D.MipLevels = 1;
    }
    hr = device.m_device->CreateShaderResourceView(m_texture, &srvd, &m_textureFromImg);
    if (FAILED(hr)) {
      ERROR(Texture, init_params, L"CreateShaderResourceView failed.");
      return hr;
    }
  }

  return S_OK;
}

// ------------------------------------------------------------
// init(): envolver textura existente y crear SRV compatible
// ------------------------------------------------------------
HRESULT Texture::init(Device& device,
  Texture& textureRef,
  DXGI_FORMAT format)
{
  if (!device.m_device) {
    ERROR(Texture, init_ref, L"Device is null.");
    return E_POINTER;
  }
  if (!textureRef.m_texture) {
    ERROR(Texture, init_ref, L"textureRef.m_texture is null.");
    return E_POINTER;
  }

  destroy();
  m_textureName.clear();

  // Tomar referencia del recurso existente
  m_texture = textureRef.m_texture;
  if (m_texture) m_texture->AddRef();

  D3D11_TEXTURE2D_DESC td = {};
  m_texture->GetDesc(&td);

  if ((td.BindFlags & D3D11_BIND_SHADER_RESOURCE) == 0) {
    ERROR(Texture, init_ref, L"Source texture lacks D3D11_BIND_SHADER_RESOURCE.");
    return E_FAIL;
  }

  DXGI_FORMAT srvFormat = (format == DXGI_FORMAT_UNKNOWN)
    ? ChooseSrvFormat(td.Format) : format;

  D3D11_SHADER_RESOURCE_VIEW_DESC srvd = {};
  srvd.Format = srvFormat;
  if (td.SampleDesc.Count > 1) {
    srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;
  }
  else {
    srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvd.Texture2D.MostDetailedMip = 0;
    srvd.Texture2D.MipLevels = (td.MipLevels && td.MipLevels != 0xFFFFFFFF)
      ? td.MipLevels : UINT(-1);
  }

  HRESULT hr = device.m_device->CreateShaderResourceView(m_texture, &srvd, &m_textureFromImg);
  if (FAILED(hr)) {
    ERROR(Texture, init_ref, L"CreateShaderResourceView failed.");
    return hr;
  }

  return S_OK;
}

// ------------------------------------------------------------
// render(): bindea SRV al Pixel Shader
// ------------------------------------------------------------
void Texture::render(DeviceContext& ctx,
  unsigned int StartSlot,
  unsigned int NumViews)
{
  if (!m_textureFromImg) {
    ERROR(Texture, render, L"SRV is null.");
    return;
  }
  if (NumViews == 0) NumViews = 1;

  ID3D11ShaderResourceView* views[1] = { m_textureFromImg };
  ctx.PSSetShaderResources(StartSlot, NumViews, views);
}

// ------------------------------------------------------------
// destroy(): libera recursos
// ------------------------------------------------------------
void Texture::destroy()
{
  SAFE_RELEASE(m_textureFromImg);
  SAFE_RELEASE(m_texture);
}
