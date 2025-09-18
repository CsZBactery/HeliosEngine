#include "../include/Prerequisites.h"
#include "../include/SwapChain.h"
#include "../include/Device.h"
#include "../include/DeviceContext.h"
#include "../include/Window.h"

// --- Fallbacks de utilería (si ya los tienes, quítalos) ---
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p) do { if (p) { (p)->Release(); (p) = nullptr; } } while(0)
#endif

#ifndef ERROR
#define ERROR(MOD, FUNC, WMSG) OutputDebugStringW(L"[ERROR][" L#MOD L"][" L#FUNC L"] " WMSG L"\n")
#endif

#ifndef MESSAGE
#define MESSAGE(MOD, FUNC, WMSG) OutputDebugStringW(L"[INFO ][" L#MOD L"][" L#FUNC L"] " WMSG L"\n")
#endif
// -----------------------------------------------------------

void SwapChain::destroyRTV_() {
  SAFE_RELEASE(m_rtv);
}

void SwapChain::destroy() {
  destroyRTV_();
  SAFE_RELEASE(m_swap);
  m_width = 0;
  m_height = 0;
  m_format = DXGI_FORMAT_R8G8B8A8_UNORM;
  MESSAGE(SwapChain, destroy, L"Released");
}

// ---------- LOW-LEVEL API ----------
HRESULT SwapChain::create(ID3D11Device* device,
  HWND hwnd,
  UINT width,
  UINT height,
  DXGI_FORMAT format,
  UINT bufferCount,
  BOOL windowed,
  UINT /*sampleCount*/)
{
  if (!device) { ERROR(SwapChain, create, L"device is nullptr"); return E_POINTER; }
  if (!hwnd) { ERROR(SwapChain, create, L"hwnd is nullptr");   return E_POINTER; }
  if (width == 0 || height == 0) {
    ERROR(SwapChain, create, L"invalid size (width/height == 0)");
    return E_INVALIDARG;
  }

  destroy();

  m_width = width;
  m_height = height;
  m_format = format;

  IDXGIDevice* dxgiDev = nullptr;
  IDXGIAdapter* adapter = nullptr;
  IDXGIFactory* factory = nullptr;

  HRESULT hr = device->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDev);
  if (FAILED(hr)) { ERROR(SwapChain, create, L"QueryInterface IDXGIDevice failed"); goto Cleanup; }

  hr = dxgiDev->GetAdapter(&adapter);
  if (FAILED(hr)) { ERROR(SwapChain, create, L"GetAdapter failed"); goto Cleanup; }

  hr = adapter->GetParent(__uuidof(IDXGIFactory), (void**)&factory);
  if (FAILED(hr)) { ERROR(SwapChain, create, L"GetParent IDXGIFactory failed"); goto Cleanup; }

  DXGI_SWAP_CHAIN_DESC sd{};
  sd.BufferDesc.Width = width;
  sd.BufferDesc.Height = height;
  sd.BufferDesc.Format = format;
  sd.BufferDesc.RefreshRate.Numerator = 0; // DXGI elige
  sd.BufferDesc.RefreshRate.Denominator = 1;
  sd.SampleDesc.Count = 1;   // ← backbuffer sin MSAA
  sd.SampleDesc.Quality = 0;
  sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  sd.BufferCount = bufferCount;       // 1 backbuffer (o 2 si quieres doble buffer)
  sd.OutputWindow = hwnd;
  sd.Windowed = windowed;
  sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD; // DXGI 1.0 clásico
  sd.Flags = 0;

  hr = factory->CreateSwapChain(device, &sd, &m_swap);
  if (FAILED(hr)) { ERROR(SwapChain, create, L"CreateSwapChain failed"); goto Cleanup; }

  hr = recreateRTV(device);
  if (FAILED(hr)) goto Cleanup;

  MESSAGE(SwapChain, create, L"Created");

Cleanup:
  SAFE_RELEASE(factory);
  SAFE_RELEASE(adapter);
  SAFE_RELEASE(dxgiDev);
  return hr;
}

HRESULT SwapChain::recreateRTV(ID3D11Device* device) {
  if (!m_swap) { ERROR(SwapChain, recreateRTV, L"swap chain is nullptr"); return E_FAIL; }
  if (!device) { ERROR(SwapChain, recreateRTV, L"device is nullptr");     return E_POINTER; }

  destroyRTV_();

  ID3D11Texture2D* backBuffer = nullptr;
  HRESULT hr = m_swap->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
  if (FAILED(hr)) { ERROR(SwapChain, recreateRTV, L"GetBuffer(0) failed"); return hr; }

  hr = device->CreateRenderTargetView(backBuffer, nullptr, &m_rtv);
  SAFE_RELEASE(backBuffer);

  if (FAILED(hr)) { ERROR(SwapChain, recreateRTV, L"CreateRenderTargetView failed"); return hr; }

  MESSAGE(SwapChain, recreateRTV, L"RTV_Recreated");
  return S_OK;
}

HRESULT SwapChain::resize(ID3D11Device* device, UINT width, UINT height) {
  if (!m_swap) { ERROR(SwapChain, resize, L"swap chain is nullptr"); return E_FAIL; }
  if (!device) { ERROR(SwapChain, resize, L"device is nullptr");     return E_POINTER; }
  if (width == 0 || height == 0) {
    ERROR(SwapChain, resize, L"invalid size (width/height == 0)");
    return E_INVALIDARG;
  }

  destroyRTV_(); // RTV debe liberarse antes del ResizeBuffers

  HRESULT hr = m_swap->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
  if (FAILED(hr)) { ERROR(SwapChain, resize, L"ResizeBuffers failed"); return hr; }

  m_width = width;
  m_height = height;

  hr = recreateRTV(device);
  if (FAILED(hr)) return hr;

  MESSAGE(SwapChain, resize, L"Resized");
  return S_OK;
}

HRESULT SwapChain::present(UINT syncInterval, UINT flags) {
  if (!m_swap) { ERROR(SwapChain, present, L"swap chain is nullptr"); return E_FAIL; }
  return m_swap->Present(syncInterval, flags);
}

void SwapChain::bindAsRenderTarget(ID3D11DeviceContext* ctx, ID3D11DepthStencilView* dsv) const {
  if (!ctx) { ERROR(SwapChain, bindAsRenderTarget, L"ctx is nullptr"); return; }
  if (!m_rtv) { ERROR(SwapChain, bindAsRenderTarget, L"RTV is nullptr"); return; }
  ID3D11RenderTargetView* rtv = m_rtv;
  ctx->OMSetRenderTargets(1, &rtv, dsv);
}

// ---------- WRAPPERS (tus clases) ----------
HRESULT SwapChain::create(Device& device,
  Window& window,
  UINT width,
  UINT height,
  DXGI_FORMAT format,
  UINT bufferCount,
  BOOL windowed,
  UINT sampleCount)
{
  // Si Device/Window exponen getters, usa: device.get(), window.hwnd()
  return create(device.m_device, window.m_hWnd, width, height,
    format, bufferCount, windowed, sampleCount);
}

HRESULT SwapChain::recreateRTV(Device& device) {
  return recreateRTV(device.m_device);
}

HRESULT SwapChain::resize(Device& device, UINT width, UINT height) {
  return resize(device.m_device, width, height);
}

void SwapChain::bindAsRenderTarget(DeviceContext& ctx, ID3D11DepthStencilView* dsv) const {
  if (!m_rtv) { ERROR(SwapChain, bindAsRenderTarget, L"RTV is nullptr"); return; }
  ID3D11RenderTargetView* rtv = m_rtv;
  ctx.OMSetRenderTargets(1, &rtv, dsv);
}
