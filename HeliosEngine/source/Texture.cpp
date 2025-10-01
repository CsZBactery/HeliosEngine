// Texture.cpp  (sin DirectXTK: usa WIC puro para PNG/JPG/BMP/etc.)
#include "../include/Prerequisites.h"
#include "../include/Texture.h"
#include "../include/Device.h"
#include "../include/DeviceContext.h"

#include <string>
#include <cctype>
#include <cstring>
#include <wincodec.h>       // WIC
#include <wrl/client.h>     // ComPtr

#pragma comment(lib, "windowscodecs.lib")

using Microsoft::WRL::ComPtr;

// ------------------------------------------------------------
// Helpers
// ------------------------------------------------------------
static DXGI_FORMAT ChooseSrvFormat(DXGI_FORMAT fmt) {
  switch (fmt) {
  case DXGI_FORMAT_R8G8B8A8_TYPELESS: return DXGI_FORMAT_R8G8B8A8_UNORM;
  case DXGI_FORMAT_B8G8R8A8_TYPELESS: return DXGI_FORMAT_B8G8R8A8_UNORM;
  case DXGI_FORMAT_R16_TYPELESS:      return DXGI_FORMAT_R16_UNORM;
  case DXGI_FORMAT_R32_TYPELESS:      return DXGI_FORMAT_R32_FLOAT;
  case DXGI_FORMAT_R24G8_TYPELESS:    return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
  default:                            return fmt;
  }
}

static bool EndsWithCaseInsensitive(const std::string& s, const char* suf) {
  const size_t n = s.size(), m = std::strlen(suf);
  if (m > n) return false;
  for (size_t i = 0; i < m; ++i) {
    char a = (char)std::tolower(s[n - m + i]);
    char b = (char)std::tolower(suf[i]);
    if (a != b) return false;
  }
  return true;
}

// Carga con WIC y crea Texture2D + SRV (RGBA 32 bits)
static HRESULT CreateTextureFromWICFile(
  ID3D11Device* device,
  const wchar_t* filename,
  ID3D11Texture2D** outTex2D,
  ID3D11ShaderResourceView** outSRV)
{
  if (!device || !filename || !outTex2D || !outSRV) return E_INVALIDARG;
  *outTex2D = nullptr;
  *outSRV = nullptr;

  // Asegurar COM
  bool needCoUninit = false;
  HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
  if (SUCCEEDED(hr)) needCoUninit = true;
  else if (hr == RPC_E_CHANGED_MODE) {
    // COM ya estaba init con otro modo; seguimos.
    hr = S_OK;
  }
  if (FAILED(hr)) return hr;

  ComPtr<IWICImagingFactory> factory;
  hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
    IID_PPV_ARGS(&factory));
  if (FAILED(hr)) { if (needCoUninit) CoUninitialize(); return hr; }

  ComPtr<IWICBitmapDecoder> decoder;
  hr = factory->CreateDecoderFromFilename(filename, nullptr, GENERIC_READ,
    WICDecodeMetadataCacheOnDemand, &decoder);
  if (FAILED(hr)) { if (needCoUninit) CoUninitialize(); return hr; }

  ComPtr<IWICBitmapFrameDecode> frame;
  hr = decoder->GetFrame(0, &frame);
  if (FAILED(hr)) { if (needCoUninit) CoUninitialize(); return hr; }

  // Convertir a 32bpp RGBA
  ComPtr<IWICFormatConverter> converter;
  hr = factory->CreateFormatConverter(&converter);
  if (FAILED(hr)) { if (needCoUninit) CoUninitialize(); return hr; }

  hr = converter->Initialize(frame.Get(),
    GUID_WICPixelFormat32bppRGBA,
    WICBitmapDitherTypeNone,
    nullptr, 0.0, WICBitmapPaletteTypeCustom);
  if (FAILED(hr)) { if (needCoUninit) CoUninitialize(); return hr; }

  UINT width = 0, height = 0;
  hr = converter->GetSize(&width, &height);
  if (FAILED(hr)) { if (needCoUninit) CoUninitialize(); return hr; }
  if (width == 0 || height == 0) { if (needCoUninit) CoUninitialize(); return E_FAIL; }

  const UINT bpp = 32;
  const UINT stride = (width * bpp + 7) / 8; // width * 4
  const UINT imageSize = stride * height;

  std::unique_ptr<BYTE[]> pixels(new (std::nothrow) BYTE[imageSize]);
  if (!pixels) { if (needCoUninit) CoUninitialize(); return E_OUTOFMEMORY; }

  hr = converter->CopyPixels(nullptr, stride, imageSize, pixels.get());
  if (FAILED(hr)) { if (needCoUninit) CoUninitialize(); return hr; }

  // Crear textura
  D3D11_TEXTURE2D_DESC td{};
  td.Width = width;
  td.Height = height;
  td.MipLevels = 1;   // (si quieres mips: 0 + GenerateMips)
  td.ArraySize = 1;
  td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  td.SampleDesc.Count = 1;
  td.Usage = D3D11_USAGE_IMMUTABLE;
  td.BindFlags = D3D11_BIND_SHADER_RESOURCE;

  D3D11_SUBRESOURCE_DATA srd{};
  srd.pSysMem = pixels.get();
  srd.SysMemPitch = stride;

  ComPtr<ID3D11Texture2D> tex;
  hr = device->CreateTexture2D(&td, &srd, &tex);
  if (FAILED(hr)) { if (needCoUninit) CoUninitialize(); return hr; }

  // SRV
  D3D11_SHADER_RESOURCE_VIEW_DESC srvd{};
  srvd.Format = td.Format;
  srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
  srvd.Texture2D.MostDetailedMip = 0;
  srvd.Texture2D.MipLevels = 1;

  ComPtr<ID3D11ShaderResourceView> srv;
  hr = device->CreateShaderResourceView(tex.Get(), &srvd, &srv);
  if (FAILED(hr)) { if (needCoUninit) CoUninitialize(); return hr; }

  *outTex2D = tex.Detach();
  *outSRV = srv.Detach();

  if (needCoUninit) CoUninitialize();
  return S_OK;
}

// Crea 1×1 blanca (fallback)
static HRESULT CreateWhite1x1(ID3D11Device* device,
  ID3D11Texture2D** outTex2D,
  ID3D11ShaderResourceView** outSRV)
{
  if (!device || !outTex2D || !outSRV) return E_INVALIDARG;
  *outTex2D = nullptr; *outSRV = nullptr;

  UINT32 white = 0xFFFFFFFF;
  D3D11_TEXTURE2D_DESC td{};
  td.Width = 1; td.Height = 1; td.MipLevels = 1; td.ArraySize = 1;
  td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  td.SampleDesc.Count = 1;
  td.Usage = D3D11_USAGE_IMMUTABLE;
  td.BindFlags = D3D11_BIND_SHADER_RESOURCE;

  D3D11_SUBRESOURCE_DATA srd{};
  srd.pSysMem = &white;
  srd.SysMemPitch = sizeof(white);

  ComPtr<ID3D11Texture2D> tex;
  HRESULT hr = device->CreateTexture2D(&td, &srd, &tex);
  if (FAILED(hr)) return hr;

  D3D11_SHADER_RESOURCE_VIEW_DESC srvd{};
  srvd.Format = td.Format;
  srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
  srvd.Texture2D.MipLevels = 1;
  srvd.Texture2D.MostDetailedMip = 0;

  ComPtr<ID3D11ShaderResourceView> srv;
  hr = device->CreateShaderResourceView(tex.Get(), &srvd, &srv);
  if (FAILED(hr)) return hr;

  *outTex2D = tex.Detach();
  *outSRV = srv.Detach();
  return S_OK;
}

// ------------------------------------------------------------
// init(): cargar desde archivo (WIC). DDS no soportado sin DirectXTK.
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

  // .dds no soportado aquí (sin DirectXTK)
  if (EndsWithCaseInsensitive(textureName, ".dds")) {
    ERROR(Texture, init_file, L"DDS no soportado sin DirectXTK. Usa PNG/JPG/BMP, etc.");
    // Fallback: blanca 1×1 para no romper el pipeline
    return CreateWhite1x1(device.m_device, &m_texture, &m_textureFromImg);
  }

  // Carga WIC
  std::wstring wpath(textureName.begin(), textureName.end());
  HRESULT hr = CreateTextureFromWICFile(device.m_device, wpath.c_str(),
    &m_texture, &m_textureFromImg);
  if (FAILED(hr)) {
    ERROR(Texture, init_file, L"Fallo al cargar imagen con WIC. Se usará textura blanca 1×1.");
    return CreateWhite1x1(device.m_device, &m_texture, &m_textureFromImg);
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

  D3D11_TEXTURE2D_DESC desc{};
  desc.Width = width;
  desc.Height = height;
  desc.MipLevels = 1;
  desc.ArraySize = 1;
  desc.Format = Format;
  desc.SampleDesc.Count = (sampleCount == 0) ? 1u : sampleCount;
  desc.SampleDesc.Quality = qualityLevels;
  desc.Usage = D3D11_USAGE_DEFAULT;
  desc.BindFlags = BindFlags;
  desc.CPUAccessFlags = 0;
  desc.MiscFlags = 0;

  HRESULT hr = device.m_device->CreateTexture2D(&desc, nullptr, &m_texture);
  if (FAILED(hr)) {
    ERROR(Texture, init_params, L"CreateTexture2D failed.");
    return hr;
  }

  if (BindFlags & D3D11_BIND_SHADER_RESOURCE) {
    D3D11_SHADER_RESOURCE_VIEW_DESC srvd{};
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

  m_texture = textureRef.m_texture;
  if (m_texture) m_texture->AddRef();

  D3D11_TEXTURE2D_DESC td{};
  m_texture->GetDesc(&td);

  if ((td.BindFlags & D3D11_BIND_SHADER_RESOURCE) == 0) {
    ERROR(Texture, init_ref, L"Source texture lacks D3D11_BIND_SHADER_RESOURCE.");
    return E_FAIL;
  }

  DXGI_FORMAT srvFormat = (format == DXGI_FORMAT_UNKNOWN)
    ? ChooseSrvFormat(td.Format)
    : format;

  D3D11_SHADER_RESOURCE_VIEW_DESC srvd{};
  srvd.Format = srvFormat;
  if (td.SampleDesc.Count > 1) {
    srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;
  }
  else {
    srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvd.Texture2D.MostDetailedMip = 0;
    srvd.Texture2D.MipLevels = (td.MipLevels && td.MipLevels != 0xFFFFFFFF) ? td.MipLevels : UINT(-1);
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
