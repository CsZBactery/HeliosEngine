#include "../include/TextureLoaderWIC.h"
#include <vector>
#include <stdexcept>

// Helper local: crea (o reutiliza) la IWICImagingFactory
static HRESULT GetWICFactory(IWICImagingFactory** outFactory)
{
  static IWICImagingFactory* s_factory = nullptr;
  if (!outFactory) return E_POINTER;

  if (s_factory)
  {
    s_factory->AddRef();
    *outFactory = s_factory;
    return S_OK;
  }

  // CoInitializeEx debe haber sido llamado en la app principal
  HRESULT hr = CoCreateInstance(
    CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
    IID_PPV_ARGS(&s_factory)
  );
  if (FAILED(hr)) return hr;

  *outFactory = s_factory; // ref 1 para el caller
  s_factory->AddRef();     // y dejamos una ref interna viva
  return S_OK;
}

HRESULT LoadTextureWIC(
  ID3D11Device* device,
  ID3D11DeviceContext* context,
  const wchar_t* filePath,
  ID3D11ShaderResourceView** outSRV,
  UINT* outWidth,
  UINT* outHeight,
  bool generateMips)
{
  if (!device || !filePath || !outSRV) return E_INVALIDARG;
  *outSRV = nullptr;

  HRESULT hr = S_OK;

  // Declarar aquí para evitar warning de "init skipped by goto"
  std::vector<uint8_t> pixels;

  // 1) Abrir imagen con WIC, convertir a 32bpp BGRA
  IWICImagingFactory* factory = nullptr;
  IWICBitmapDecoder* decoder = nullptr;
  IWICBitmapFrameDecode* frame = nullptr;
  IWICFormatConverter* converter = nullptr;

  hr = GetWICFactory(&factory);
  if (FAILED(hr)) goto Cleanup;

  hr = factory->CreateDecoderFromFilename(
    filePath, nullptr, GENERIC_READ,
    WICDecodeMetadataCacheOnDemand, &decoder);
  if (FAILED(hr)) goto Cleanup;

  hr = decoder->GetFrame(0, &frame);
  if (FAILED(hr)) goto Cleanup;

  hr = factory->CreateFormatConverter(&converter);
  if (FAILED(hr)) goto Cleanup;

  // Convertimos a 32 bits BGRA
  hr = converter->Initialize(
    frame,
    GUID_WICPixelFormat32bppBGRA,
    WICBitmapDitherTypeNone,
    nullptr, 0.0, WICBitmapPaletteTypeCustom);
  if (FAILED(hr)) goto Cleanup;

  UINT w = 0, h = 0;
  hr = converter->GetSize(&w, &h);
  if (FAILED(hr)) goto Cleanup;

  // 2) Reservar buffer y copiar los píxeles desde WIC
  size_t rowPitch = static_cast<size_t>(w) * 4;        // 4 bytes por pixel (BGRA)
  size_t imageSz = rowPitch * static_cast<size_t>(h);
  pixels.resize(imageSz);                               // <-- corrección: sin init antes del goto

  hr = converter->CopyPixels(
    nullptr, static_cast<UINT>(rowPitch),
    static_cast<UINT>(imageSz), pixels.data());
  if (FAILED(hr)) goto Cleanup;

  // 3) Crear textura D3D11 (BGRA8)
  D3D11_TEXTURE2D_DESC texDesc = {};
  texDesc.Width = w;
  texDesc.Height = h;
  texDesc.MipLevels = generateMips ? 0 : 1; // 0 = autogenerar mipchain
  texDesc.ArraySize = 1;
  texDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
  texDesc.SampleDesc.Count = 1;
  texDesc.SampleDesc.Quality = 0;
  texDesc.Usage = D3D11_USAGE_DEFAULT;
  texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | (generateMips ? D3D11_BIND_RENDER_TARGET : 0);
  texDesc.CPUAccessFlags = 0;
  texDesc.MiscFlags = generateMips ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0;

  ID3D11Texture2D* texture = nullptr;

  if (!generateMips)
  {
    // Modo simple: una sola mip con datos iniciales
    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = pixels.data();
    initData.SysMemPitch = static_cast<UINT>(rowPitch);

    hr = device->CreateTexture2D(&texDesc, &initData, &texture);
    if (FAILED(hr)) goto Cleanup;
  }
  else
  {
    // Modo mips: creamos sin datos iniciales (MipLevels=0), subimos nivel 0 y generamos.
    hr = device->CreateTexture2D(&texDesc, nullptr, &texture);
    if (FAILED(hr)) goto Cleanup;

    // Subir nivel 0
    D3D11_BOX box = {};
    box.left = 0; box.top = 0; box.front = 0;
    box.right = w; box.bottom = h; box.back = 1;

    // Se asume que 'context' != nullptr cuando generateMips=true
    if (!context) { hr = E_POINTER; goto CleanupTex; }

    context->UpdateSubresource(
      texture, 0, &box, pixels.data(),
      static_cast<UINT>(rowPitch), 0);
  }

  // 4) Crear SRV
  ID3D11ShaderResourceView* srv = nullptr;
  D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
  srvDesc.Format = texDesc.Format;
  srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
  srvDesc.Texture2D.MostDetailedMip = 0;
  srvDesc.Texture2D.MipLevels = (generateMips ? -1 : 1);

  hr = device->CreateShaderResourceView(texture, &srvDesc, &srv);
  if (FAILED(hr)) goto CleanupTex;

  // 5) Generar mips si aplica
  if (generateMips)
  {
    context->GenerateMips(srv);
  }

  // OK: devolver resultados
  *outSRV = srv;
  if (outWidth)  *outWidth = w;
  if (outHeight) *outHeight = h;

  SAFE_RELEASE(texture);
  SAFE_RELEASE(converter);
  SAFE_RELEASE(frame);
  SAFE_RELEASE(decoder);
  SAFE_RELEASE(factory);
  return S_OK;

CleanupTex:
  SAFE_RELEASE(srv);
  SAFE_RELEASE(texture);
Cleanup:
  SAFE_RELEASE(converter);
  SAFE_RELEASE(frame);
  SAFE_RELEASE(decoder);
  SAFE_RELEASE(factory);
  return hr;
}
