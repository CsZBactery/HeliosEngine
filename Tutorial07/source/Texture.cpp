// Texture.cpp
#include "../include/Texture.h"
#include "../include/Device.h"
#include "../include/DeviceContext.h"

#include <d3d11.h>
#include <d3dx11.h>
#include <string>

#ifndef ERROR
// Usa tu propio logger si ya tienes uno
#define ERROR(MOD, FUNC, MSG) OutputDebugStringA(("[ERROR][" MOD "][" FUNC "] " MSG "\n"))
#endif

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(x) do { if (x) { (x)->Release(); (x) = nullptr; } } while(0)
#endif

// ------------------------------------------------------------
// Helpers internos
// ------------------------------------------------------------
static DXGI_FORMAT ToSrvFormatForMsaa(DXGI_FORMAT fmt) {
  // Para la mayoría de formatos no necesitas convertir; si alguna vez usas
  // typeless para RTV/DSV, ajusta aquí a la variante UNORM/TYPELESS adecuada.
  return fmt;
}

// ------------------------------------------------------------
// init(): cargar desde archivo (DDS/WIC)
// ------------------------------------------------------------
HRESULT Texture::init(Device& device,
  const std::string& textureName,
  ExtensionType /*extensionType*/)
{
  if (!device.m_device) { // ajusta si tu Device tiene otro nombre
    ERROR("Texture", "init(file)", "Device is null.");
    return E_POINTER;
  }

  // Limpieza previa
  destroy();
  m_textureName = textureName;

  // Carga SRV directamente desde archivo (D3DX11 soporta DDS y WIC)
  D3DX11_IMAGE_LOAD_INFO loadInfo = {};
  loadInfo.Format = DXGI_FORMAT_FROM_FILE;              // autodetect
  loadInfo.MipLevels = 0;                                  // genera cadena si aplica
  loadInfo.BindFlags = D3D11_BIND_SHADER_RESOURCE;

  HRESULT hr = D3DX11CreateShaderResourceViewFromFileA(
    device.m_device,
    textureName.c_str(),
    &loadInfo,
    nullptr,                 // no necesitamos device context aquí
    &m_textureFromImg,
    nullptr
  );
  if (FAILED(hr)) {
    ERROR("Texture", "init(file)", "D3DX11CreateShaderResourceViewFromFileA failed.");
    return hr;
  }

  // Si quieres guardar también el ID3D11Texture2D subyacente, lo consultamos:
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
  desc.MipLevels = 1; // si quieres cadena de mips, cambia a 0 + GenerateMips
  desc.ArraySize = 1;
  desc.Format = Format;
  desc.SampleDesc.Count = (sampleCount == 0) ? 1u : sampleCount;
  desc.SampleDesc.Quality = qualityLevels;
  desc.Usage = D3D11_USAGE_DEFAULT;
  desc.BindFlags = BindFlags;  // p.ej. D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET
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
    srvd.Format = ToSrvFormatForMsaa(Format);

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

  // Creamos SRV si el formato y flags lo permiten
  D3D11_SHADER_RESOURCE_VIEW_DESC srvd = {};
  srvd.Format = (format == DXGI_FORMAT_UNKNOWN) ? td.Format : format;

  if (td.SampleDesc.Count > 1) {
    srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;
  }
  else {
    srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvd.Texture2D.MostDetailedMip = 0;
    srvd.Texture2D.MipLevels = td.MipLevels ? td.MipLevels : 1;
  }

  HRESULT hr = device.m_device->CreateShaderResourceView(m_texture, &srvd, &m_textureFromImg);
  if (FAILED(hr)) {
    ERROR("Texture", "init(ref)", "CreateShaderResourceView failed.");
    return hr;
  }

  return S_OK;
}

// ------------------------------------------------------------
// update(): placeholder por si haces streaming/anim
// ------------------------------------------------------------
void Texture::update()
{
  // Intencionalmente vacío
}

// ------------------------------------------------------------
// render(): bindea SRV al Pixel Shader (usa wrappers públicos)
// ------------------------------------------------------------
void Texture::render(DeviceContext& deviceContext,
  unsigned int StartSlot,
  unsigned int NumViews)
{
  if (!m_textureFromImg) return;

  ID3D11ShaderResourceView* views[1] = { m_textureFromImg };
  deviceContext.PSSetShaderResources(StartSlot, NumViews, views);
  // Si gestionas samplers en otra clase, aquí también puedes llamar:
  // deviceContext.PSSetSamplers(StartSlot, NumSamplers, ppSamplers);
}

// ------------------------------------------------------------
// destroy(): libera recursos
// ------------------------------------------------------------
void Texture::destroy()
{
  SAFE_RELEASE(m_textureFromImg);
  SAFE_RELEASE(m_texture);
}
