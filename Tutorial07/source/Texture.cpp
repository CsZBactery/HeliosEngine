// Texture.cpp
#include "../include/Texture.h"
#include "../include/Device.h"
#include "../include/DeviceContext.h"

#include <d3d11.h>
#include <d3dx11.h>
#include <string>

#ifndef ERROR
#define ERROR(MOD, FUNC, MSG) OutputDebugStringA(("[ERROR][" MOD "][" FUNC "] " MSG "\n"))
#endif

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(x) do { if (x) { (x)->Release(); (x) = nullptr; } } while(0)
#endif

// ------------------------------------------------------------
// Helpers internos
// ------------------------------------------------------------

// Mapea formatos TYPELESS al equivalente tipado para usar en SRV.
// Agrega aquí los que uses en tu engine.
static DXGI_FORMAT ChooseSrvFormat(DXGI_FORMAT f) {
  switch (f) {
  case DXGI_FORMAT_R8G8B8A8_TYPELESS:   return DXGI_FORMAT_R8G8B8A8_UNORM;
  case DXGI_FORMAT_B8G8R8A8_TYPELESS:   return DXGI_FORMAT_B8G8R8A8_UNORM;
  case DXGI_FORMAT_R16_TYPELESS:        return DXGI_FORMAT_R16_UNORM;
  case DXGI_FORMAT_R24G8_TYPELESS:      return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
  case DXGI_FORMAT_R32_TYPELESS:        return DXGI_FORMAT_R32_FLOAT;
  default:                              return f; // ya es tipado usable
  }
}

// ------------------------------------------------------------
// init(): cargar desde archivo (DDS/WIC)
// ------------------------------------------------------------
HRESULT Texture::init(Device& device,
  const std::string& textureName,
  ExtensionType /*extensionType*/)
{
  if (!device.m_device) {
    ERROR("Texture", "init(file)", "Device is null.");
    return E_POINTER;
  }

  // Limpieza previa
  destroy();
  m_textureName = textureName;

  // Carga SRV directamente desde archivo (D3DX11 soporta DDS y WIC)
  D3DX11_IMAGE_LOAD_INFO loadInfo = {};
  loadInfo.Format = DXGI_FORMAT_FROM_FILE;     // autodetect
  loadInfo.MipLevels = 0;                         // generar cadena si aplica
  loadInfo.BindFlags = D3D11_BIND_SHADER_RESOURCE;

  HRESULT hr = D3DX11CreateShaderResourceViewFromFileA(
    device.m_device,
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

  // Guardar el ID3D11Texture2D subyacente (opcional pero útil)
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

  // Descripción de la textura
  D3D11_TEXTURE2D_DESC desc = {};
  desc.Width = width;
  desc.Height = height;
  desc.MipLevels = 1; // si quieres cadena de mips, usa 0 + GenerateMips
  desc.ArraySize = 1;
  desc.Format = Format;
  desc.SampleDesc.Count = (sampleCount == 0) ? 1u : sampleCount;
  desc.SampleDesc.Quality = qualityLevels;
  desc.Usage = D3D11_USAGE_DEFAULT;
  desc.BindFlags = BindFlags;  // ej: D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET
  desc.CPUAccessFlags = 0;
  desc.MiscFlags = 0;

  HRESULT hr = device.m_device->CreateTexture2D(&desc, nullptr, &m_texture);
  if (FAILED(hr)) {
    ERROR("Texture", "init(params)", "CreateTexture2D failed.");
    return hr;
  }

  // Crear SRV si la textura será muestreada en shaders
  if (BindFlags & D3D11_BIND_SHADER_RESOURCE) {
    D3D11_SHADER_RESOURCE_VIEW_DESC srvd = {};
    srvd.Format = ChooseSrvFormat(Format);

    if (desc.SampleDesc.Count > 1) {
      // Multisample
      srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;
    }
    else {
      srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
      srvd.Texture2D.MostDetailedMip = 0;
      srvd.Texture2D.MipLevels = 1; // si desc.MipLevels==0, podrías usar UINT(-1) y luego GenerateMips
    }

    hr = device.m_device->CreateShaderResourceView(m_texture, &srvd, &m_textureFromImg);
    if (FAILED(hr)) {
      ERROR("Texture", "init(params)", "CreateShaderResourceView failed.");
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

  // Referenciamos el recurso existente
  m_texture = textureRef.m_texture;
  if (m_texture) m_texture->AddRef();

  D3D11_TEXTURE2D_DESC td = {};
  m_texture->GetDesc(&td);

  // 1) Validar que la textura soporte SRV
  if ((td.BindFlags & D3D11_BIND_SHADER_RESOURCE) == 0) {
    ERROR("Texture", "init(ref)", "Source texture lacks D3D11_BIND_SHADER_RESOURCE.");
    return E_FAIL;
  }

  // 2) Elegir formato SRV correcto (resolver TYPELESS si aplica)
  DXGI_FORMAT srvFormat = (format == DXGI_FORMAT_UNKNOWN)
    ? ChooseSrvFormat(td.Format)
    : format;

  // 3) Describir la SRV (usar todos los mips si hay cadena disponible)
  D3D11_SHADER_RESOURCE_VIEW_DESC srvd = {};
  srvd.Format = srvFormat;

  if (td.SampleDesc.Count > 1) {
    srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;
  }
  else {
    srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvd.Texture2D.MostDetailedMip = 0;

    // Si td.MipLevels == 0 (algunas cargas) o quieres exponer todos:
    srvd.Texture2D.MipLevels = (td.MipLevels && td.MipLevels != 0xFFFFFFFF)
      ? td.MipLevels
      : UINT(-1); // "todos los mips disponibles"
  }

  HRESULT hr = device.m_device->CreateShaderResourceView(m_texture, &srvd, &m_textureFromImg);
  if (FAILED(hr)) {
    ERROR("Texture", "init(ref)", "CreateShaderResourceView failed.");
    return hr;
  }

  return S_OK;
}

// ------------------------------------------------------------
// render(): bindea SRV al Pixel Shader (usa wrappers públicos)
// ------------------------------------------------------------
void Texture::render(DeviceContext& ctx,
  unsigned int StartSlot,
  unsigned int NumViews)
{
  if (!m_textureFromImg) {
    ERROR("Texture", "render", "SRV is null.");
    return;
  }

  // La API espera un array; para 1 vista, arma el arreglo local:
  if (NumViews == 0) NumViews = 1;     // sanidad
  ID3D11ShaderResourceView* views[1] = { m_textureFromImg };
  ctx.PSSetShaderResources(StartSlot, NumViews, views);

  // Si gestionas samplers en otra clase, podrías también:
  // ctx.PSSetSamplers(StartSlot, NumSamplers, ppSamplers);
}


// ------------------------------------------------------------
// destroy(): libera recursos
// ------------------------------------------------------------
void Texture::destroy()
{
  // Libera primero la SRV y luego el recurso base
  SAFE_RELEASE(m_textureFromImg);
  SAFE_RELEASE(m_texture);
}

