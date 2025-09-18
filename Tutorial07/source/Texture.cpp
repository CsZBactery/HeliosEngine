// Texture.cpp
#include "../include/Texture.h"
#include "../include/Device.h"
#include "../include/DeviceContext.h"

#include <d3d11.h>
#include <d3dx11.h>
#include <cassert>

#ifndef ERROR
// Si ya tienes tu macro, elimina esto.
#define ERROR(MOD, FUNC, MSG) OutputDebugStringA(("[ERROR][" MOD "][" FUNC "] " MSG "\n"))
#endif

template<typename T>
static void SafeRelease(T*& p) { if (p) { p->Release(); p = nullptr; } }

// ==============================
// 1) init() desde archivo
// ==============================
HRESULT Texture::init(Device& device,
  const std::string& textureName,
  ExtensionType /*extensionType*/)
{
  if (!device.m_device) { // <-- cambia si tu Device usa otro nombre
    ERROR("Texture", "init(file)", "Device is null.");
    return E_POINTER;
  }

  // Limpieza previa
  destroy();
  m_textureName = textureName;

  // Crea SRV directamente desde archivo
  D3DX11_IMAGE_LOAD_INFO loadInfo = {};
  loadInfo.Format = DXGI_FORMAT_FROM_FILE;
  loadInfo.MipLevels = 0;                        // permitir cadena de mips si aplica
  loadInfo.BindFlags = D3D11_BIND_SHADER_RESOURCE;

  HRESULT hr = D3DX11CreateShaderResourceViewFromFileA(
    device.m_device,                           // <-- cambia si tu Device usa otro nombre
    textureName.c_str(),
    &loadInfo,
    nullptr,
    &m_textureFromImg,
    nullptr
  );
  if (FAILED(hr)) {
    ERROR("Texture", "init(file)", "D3DX11CreateShaderResourceViewFromFileA failed.");
    return hr;
  }

  // (Opcional) obtener el ID3D11Texture2D real detrás del SRV y guardarlo en m_texture
  {
    ID3D11Resource* res = nullptr;
    m_textureFromImg->GetResource(&res);
    if (res) {
      res->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&m_texture));
      res->Release();
    }
  }

  return S_OK;
}

// ===========================================
// 2) init() creando textura vacía con params
// ===========================================
HRESULT Texture::init(Device& device,
  unsigned int width,
  unsigned int height,
  DXGI_FORMAT format,
  unsigned int BindFlags,
  unsigned int sampleCount,
  unsigned int qualityLevels)
{
  if (!device.m_device) {
    ERROR("Texture", "init(params)", "Device is null.");
    return E_POINTER;
  }
  if (width == 0 || height == 0) {
    ERROR("Texture", "init(params)", "Width/Height must be > 0.");
    return E_INVALIDARG;
  }

  // Limpieza previa
  destroy();
  m_textureName.clear();

  D3D11_TEXTURE2D_DESC td = {};
  td.Width = width;
  td.Height = height;
  td.MipLevels = 1;                      // 1 mip (ajusta si necesitas más)
  td.ArraySize = 1;
  td.Format = format;
  td.SampleDesc.Count = sampleCount == 0 ? 1u : sampleCount;
  td.SampleDesc.Quality = qualityLevels;
  td.Usage = D3D11_USAGE_DEFAULT;
  td.BindFlags = BindFlags;              // p.ej. D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET
  td.CPUAccessFlags = 0;
  td.MiscFlags = 0;

  HRESULT hr = device.m_device->CreateTexture2D(&td, nullptr, &m_texture);
  if (FAILED(hr)) {
    ERROR("Texture", "init(params)", "CreateTexture2D failed.");
    return hr;
  }

  // Si se desea muestrear en shader, creamos su SRV
  if (BindFlags & D3D11_BIND_SHADER_RESOURCE) {
    D3D11_SHADER_RESOURCE_VIEW_DESC srvd = {};
    srvd.Format = format;

    if (td.SampleDesc.Count > 1) {
      srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;
    }
    else {
      srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
      srvd.Texture2D.MostDetailedMip = 0;
      srvd.Texture2D.MipLevels = 1;
    }

    hr = device.m_device->CreateShaderResourceView(m_texture, &srvd, &m_textureFromImg);
    if (FAILED(hr)) {
      ERROR("Texture", "init(params)", "CreateShaderResourceView failed.");
      return hr;
    }
  }

  return S_OK;
}

// ========================================================
// 3) init() envolviendo una textura existente (crear SRV)
// ========================================================
HRESULT Texture::init(Device& device,
  Texture& textureRef,
  DXGI_FORMAT format)
{
  if (!device.m_device) {
    ERROR("Texture", "init(ref)", "Device is null.");
    return E_POINTER;
  }
  if (!textureRef.m_texture) {
    ERROR("Texture", "init(ref)", "textureRef.m_texture is null.");
    return E_POINTER;
  }

  // Limpieza previa
  destroy();
  m_textureName.clear();

  // No clonamos el recurso; lo referenciamos (AddRef implícito en SRV)
  m_texture = textureRef.m_texture;
  if (m_texture) m_texture->AddRef();

  D3D11_TEXTURE2D_DESC td = {};
  m_texture->GetDesc(&td);

  D3D11_SHADER_RESOURCE_VIEW_DESC srvd = {};
  srvd.Format = format;

  if (td.SampleDesc.Count > 1) {
    srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;
  }
  else {
    srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvd.Texture2D.MostDetailedMip = 0;
    srvd.Texture2D.MipLevels = td.MipLevels;
  }

  HRESULT hr = device.m_device->CreateShaderResourceView(m_texture, &srvd, &m_textureFromImg);
  if (FAILED(hr)) {
    ERROR("Texture", "init(ref)", "CreateShaderResourceView failed.");
    return hr;
  }

  return S_OK;
}

// ==============================
// update() (placeholder)
// ==============================
void Texture::update()
{
  // vacío por ahora; útil si haces streaming/animación, etc.
}

// ==============================
// render(): bind de SRV al PS
// ==============================
void Texture::render(DeviceContext& ctx, unsigned StartSlot, unsigned NumViews) {
  if (!m_textureFromImg) return;
  ID3D11ShaderResourceView* views[1] = { m_textureFromImg };
  ctx.PSSetShaderResources(StartSlot, NumViews, views);
}


// ==============================
// destroy(): liberar recursos
// ==============================
void Texture::destroy()
{
  SafeRelease(m_textureFromImg);
  SafeRelease(m_texture);
}
