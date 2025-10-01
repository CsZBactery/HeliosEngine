// HeliosEngine.cpp — DX11 minimal + Device + DeviceContext (sin D3DX, sin DirectXTK)

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <windows.h>
#include <wrl/client.h>
#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <cstdint>
#include <string>
#include <cstring>   // std::strlen
#include <cmath>     // sinf, cosf

// Wrappers propios
#include "../include/Device.h"
#include "../include/DeviceContext.h"
#include "../include/Window.h"   // <- usa tu clase Window real

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

// Win32 / COM (resolver unresolved externals de User32 y COM)
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "ole32.lib")

using Microsoft::WRL::ComPtr;

// =============================
// Shaders embebidos (HLSL)
// =============================
static const char* g_HLSL = R"(
cbuffer CBNeverChanges     : register(b0) { float4x4 gView;       };
cbuffer CBChangeOnResize   : register(b1) { float4x4 gProjection; };
cbuffer CBChangesEveryFrame: register(b2) { float4x4 gWorld; float4 gMeshColor; };

struct VS_INPUT  { float3 Pos : POSITION; float2 Tex : TEXCOORD0; };
struct VS_OUTPUT { float4 Pos : SV_POSITION; float2 Tex : TEXCOORD0; };

VS_OUTPUT VS(VS_INPUT i) {
    VS_OUTPUT o;
    float4 p = float4(i.Pos, 1.0f);
    o.Pos = mul(p, gWorld);
    o.Pos = mul(o.Pos, gView);
    o.Pos = mul(o.Pos, gProjection);
    o.Tex = i.Tex;
    return o;
}

Texture2D txDiffuse : register(t0);
SamplerState samLinear : register(s0);

float4 PS(VS_OUTPUT i) : SV_Target {
    float4 texc = txDiffuse.Sample(samLinear, i.Tex);
    return texc * gMeshColor;
}
)";

// Compilación desde memoria
static HRESULT CompileFromSource(const char* src, const char* entry, const char* target, ID3DBlob** blobOut) {
  UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(_DEBUG)
  flags |= D3DCOMPILE_DEBUG;
#endif
  ComPtr<ID3DBlob> err, blob;
  HRESULT hr = D3DCompile(src, (UINT)std::strlen(src),
    nullptr, nullptr, nullptr,
    entry, target,
    flags, 0,
    &blob, &err);
  if (FAILED(hr)) {
    if (err) OutputDebugStringA((const char*)err->GetBufferPointer());
    return hr;
  }
  *blobOut = blob.Detach();
  return S_OK;
}

// =============================
// Estructuras
// =============================
struct SimpleVertex { DirectX::XMFLOAT3 Pos; DirectX::XMFLOAT2 Tex; };
struct CBNeverChanges { DirectX::XMMATRIX mView; };
struct CBChangeOnResize { DirectX::XMMATRIX mProjection; };
struct CBChangesEveryFrame { DirectX::XMMATRIX mWorld; DirectX::XMFLOAT4 vMeshColor; };

// =============================
// Globals DX
// =============================
static Window                           g_window;
static Device                           g_device;     // contiene: ID3D11Device* m_device
static DeviceContext                    g_devctx;     // wrapper del contexto inmediato
static ComPtr<ID3D11DeviceContext>      g_ctx;        // dueño real del contexto
static ComPtr<IDXGISwapChain>           g_swap;
static ComPtr<ID3D11RenderTargetView>   g_rtv;
static ComPtr<ID3D11Texture2D>          g_depth;
static ComPtr<ID3D11DepthStencilView>   g_dsv;
static ComPtr<ID3D11VertexShader>       g_vs;
static ComPtr<ID3D11PixelShader>        g_ps;
static ComPtr<ID3D11InputLayout>        g_layout;
static ComPtr<ID3D11Buffer>             g_vb, g_ib;
static ComPtr<ID3D11Buffer>             g_cbView, g_cbProj, g_cbFrame;
static ComPtr<ID3D11ShaderResourceView> g_srv; // textura 1x1 blanca
static ComPtr<ID3D11SamplerState>       g_samp;
static ComPtr<ID3D11RasterizerState>    g_rsSolid, g_rsWire;
static bool                             g_wire = false;

static DirectX::XMMATRIX gWorld, gView, gProj;
static DirectX::XMFLOAT4 gMeshColor(0.7f, 0.7f, 0.7f, 1.0f);

// =============================
// Prototipos
// =============================
static HRESULT InitDevice();
static void    CleanupDevice();
static void    Render();
static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// =============================
// wWinMain
// =============================
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int nCmdShow) {
  if (FAILED(g_window.init(hInstance, nCmdShow, WndProc))) return 0;
  if (FAILED(InitDevice())) { CleanupDevice(); return 0; }

  MSG msg{};
  while (msg.message != WM_QUIT) {
    if (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
      TranslateMessage(&msg);
      DispatchMessageW(&msg);
    }
    else {
      Render();
    }
  }
  CleanupDevice();
  return (int)msg.wParam;
}

// =============================
// InitDevice
// =============================
static HRESULT InitDevice() {
  // SwapChain
  DXGI_SWAP_CHAIN_DESC sd{};
  sd.BufferCount = 1;
  sd.BufferDesc.Width = g_window.width();
  sd.BufferDesc.Height = g_window.height();
  sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  sd.BufferDesc.RefreshRate.Numerator = 60;
  sd.BufferDesc.RefreshRate.Denominator = 1;
  sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  sd.OutputWindow = g_window.handle();
  sd.SampleDesc.Count = 1;
  sd.Windowed = TRUE;

  UINT flags = 0;
#if defined(_DEBUG)
  flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
  D3D_FEATURE_LEVEL req[] = {
    D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0
  };
  D3D_FEATURE_LEVEL out{};

  // Crear device + swapchain
  HRESULT hr = D3D11CreateDeviceAndSwapChain(
    nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags,
    req, (UINT)(sizeof(req) / sizeof(req[0])),
    D3D11_SDK_VERSION, &sd, &g_swap, &g_device.m_device, &out, &g_ctx);
  if (FAILED(hr)) return hr;

  // DeviceContext (adjunta al inmediato)
  g_devctx.attach(g_ctx.Get());

  // RTV
  ComPtr<ID3D11Texture2D> back;
  if (FAILED(g_swap->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)back.GetAddressOf()))) return E_FAIL;
  if (FAILED(g_device.CreateRenderTargetView(back.Get(), nullptr, g_rtv.ReleaseAndGetAddressOf()))) return E_FAIL;

  // Depth + DSV
  D3D11_TEXTURE2D_DESC d{};
  d.Width = g_window.width(); d.Height = g_window.height();
  d.MipLevels = 1; d.ArraySize = 1;
  d.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
  d.SampleDesc.Count = 1;
  d.Usage = D3D11_USAGE_DEFAULT;
  d.BindFlags = D3D11_BIND_DEPTH_STENCIL;

  if (FAILED(g_device.m_device->CreateTexture2D(&d, nullptr, &g_depth))) return E_FAIL;
  if (FAILED(g_device.m_device->CreateDepthStencilView(g_depth.Get(), nullptr, &g_dsv))) return E_FAIL;

  // OMSetRenderTargets
  {
    ID3D11RenderTargetView* rtv = g_rtv.Get();
    g_devctx.OMSetRenderTargets(1, &rtv, g_dsv.Get());
  }

  // Viewport
  D3D11_VIEWPORT vp{};
  vp.Width = (FLOAT)g_window.width();
  vp.Height = (FLOAT)g_window.height();
  vp.MinDepth = 0.f; vp.MaxDepth = 1.f;
  g_ctx->RSSetViewports(1, &vp);

  // Shaders
  ComPtr<ID3DBlob> vsBlob, psBlob;
  if (FAILED(CompileFromSource(g_HLSL, "VS", "vs_4_0", &vsBlob))) return E_FAIL;
  if (FAILED(CompileFromSource(g_HLSL, "PS", "ps_4_0", &psBlob))) return E_FAIL;

  if (FAILED(g_device.m_device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &g_vs))) return E_FAIL;
  if (FAILED(g_device.m_device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &g_ps))) return E_FAIL;

  // Input Layout
  D3D11_INPUT_ELEMENT_DESC il[] = {
      { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
      { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
  };
  if (FAILED(g_device.m_device->CreateInputLayout(il, 2, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &g_layout))) return E_FAIL;
  g_ctx->IASetInputLayout(g_layout.Get());

  // Geometría: cubo
  SimpleVertex vertices[] = {
      { DirectX::XMFLOAT3(-1,  1, -1), DirectX::XMFLOAT2(0,0) }, { DirectX::XMFLOAT3(1,  1, -1), DirectX::XMFLOAT2(1,0) },
      { DirectX::XMFLOAT3(1,  1,  1), DirectX::XMFLOAT2(1,1) },  { DirectX::XMFLOAT3(-1,  1,  1), DirectX::XMFLOAT2(0,1) },
      { DirectX::XMFLOAT3(-1, -1, -1), DirectX::XMFLOAT2(0,0) }, { DirectX::XMFLOAT3(1, -1, -1), DirectX::XMFLOAT2(1,0) },
      { DirectX::XMFLOAT3(1, -1,  1),  DirectX::XMFLOAT2(1,1) }, { DirectX::XMFLOAT3(-1, -1,  1), DirectX::XMFLOAT2(0,1) },
      { DirectX::XMFLOAT3(-1, -1,  1), DirectX::XMFLOAT2(0,0) }, { DirectX::XMFLOAT3(-1, -1, -1), DirectX::XMFLOAT2(1,0) },
      { DirectX::XMFLOAT3(-1,  1, -1), DirectX::XMFLOAT2(1,1) }, { DirectX::XMFLOAT3(-1,  1,  1), DirectX::XMFLOAT2(0,1) },
      { DirectX::XMFLOAT3(1, -1,  1),  DirectX::XMFLOAT2(0,0) }, { DirectX::XMFLOAT3(1, -1, -1), DirectX::XMFLOAT2(1,0) },
      { DirectX::XMFLOAT3(1,  1, -1),  DirectX::XMFLOAT2(1,1) }, { DirectX::XMFLOAT3(1,  1,  1), DirectX::XMFLOAT2(0,1) },
      { DirectX::XMFLOAT3(-1, -1, -1), DirectX::XMFLOAT2(0,0) }, { DirectX::XMFLOAT3(1, -1, -1), DirectX::XMFLOAT2(1,0) },
      { DirectX::XMFLOAT3(1,  1, -1),  DirectX::XMFLOAT2(1,1) }, { DirectX::XMFLOAT3(-1,  1, -1), DirectX::XMFLOAT2(0,1) },
      { DirectX::XMFLOAT3(-1, -1,  1), DirectX::XMFLOAT2(0,0) }, { DirectX::XMFLOAT3(1, -1,  1), DirectX::XMFLOAT2(1,0) },
      { DirectX::XMFLOAT3(1,  1,  1),  DirectX::XMFLOAT2(1,1) }, { DirectX::XMFLOAT3(-1,  1,  1), DirectX::XMFLOAT2(0,1) },
  };
  WORD indices[] = {
      3,1,0,  2,1,3,  6,4,5,  7,4,6,  11,9,8, 10,9,11,
      14,12,13, 15,12,14, 19,17,16, 18,17,19, 22,20,21, 23,20,22
  };

  D3D11_BUFFER_DESC bd{}; D3D11_SUBRESOURCE_DATA srd{};
  bd.Usage = D3D11_USAGE_DEFAULT;
  bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  bd.ByteWidth = (UINT)sizeof(vertices);
  srd.pSysMem = vertices;
  if (FAILED(g_device.m_device->CreateBuffer(&bd, &srd, &g_vb))) return E_FAIL;

  bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
  bd.ByteWidth = (UINT)sizeof(indices);
  srd.pSysMem = indices;
  if (FAILED(g_device.m_device->CreateBuffer(&bd, &srd, &g_ib))) return E_FAIL;

  // Constant buffers
  bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  bd.ByteWidth = sizeof(CBNeverChanges);
  if (FAILED(g_device.m_device->CreateBuffer(&bd, nullptr, &g_cbView))) return E_FAIL;

  bd.ByteWidth = sizeof(CBChangeOnResize);
  if (FAILED(g_device.m_device->CreateBuffer(&bd, nullptr, &g_cbProj))) return E_FAIL;

  bd.ByteWidth = sizeof(CBChangesEveryFrame);
  if (FAILED(g_device.m_device->CreateBuffer(&bd, nullptr, &g_cbFrame))) return E_FAIL;

  // Textura 1x1 blanca
  {
    UINT32 white = 0xFFFFFFFF;
    D3D11_TEXTURE2D_DESC td{};
    td.Width = 1; td.Height = 1; td.MipLevels = 1; td.ArraySize = 1;
    td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    td.SampleDesc.Count = 1;
    td.Usage = D3D11_USAGE_IMMUTABLE;
    td.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA texSRD{}; texSRD.pSysMem = &white; texSRD.SysMemPitch = sizeof(white);
    ComPtr<ID3D11Texture2D> tex;
    if (FAILED(g_device.m_device->CreateTexture2D(&td, &texSRD, &tex))) return E_FAIL;
    if (FAILED(g_device.m_device->CreateShaderResourceView(tex.Get(), nullptr, &g_srv))) return E_FAIL;
  }

  // Sampler
  D3D11_SAMPLER_DESC sdsc{};
  sdsc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
  sdsc.AddressU = sdsc.AddressV = sdsc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
  sdsc.ComparisonFunc = D3D11_COMPARISON_NEVER;
  sdsc.MinLOD = 0; sdsc.MaxLOD = D3D11_FLOAT32_MAX;
  if (FAILED(g_device.m_device->CreateSamplerState(&sdsc, &g_samp))) return E_FAIL;

  // Rasterizer states
  D3D11_RASTERIZER_DESC rs{};
  rs.FillMode = D3D11_FILL_SOLID;
  rs.CullMode = D3D11_CULL_BACK;
  g_device.m_device->CreateRasterizerState(&rs, &g_rsSolid);
  rs.FillMode = D3D11_FILL_WIREFRAME;
  g_device.m_device->CreateRasterizerState(&rs, &g_rsWire);
  g_ctx->RSSetState(g_rsSolid.Get());

  // Matrices iniciales
  gWorld = DirectX::XMMatrixIdentity();

  using namespace DirectX;
  XMVECTOR Eye = XMVectorSet(0.0f, 3.0f, -6.0f, 0.0f);
  XMVECTOR At = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
  XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
  gView = XMMatrixLookAtLH(Eye, At, Up);

  CBNeverChanges cbV{ XMMatrixTranspose(gView) };
  g_ctx->UpdateSubresource(g_cbView.Get(), 0, nullptr, &cbV, 0, 0);

  gProj = XMMatrixPerspectiveFovLH(
    XM_PIDIV4,
    (float)g_window.width() / (float)g_window.height(),
    0.01f, 100.0f);
  CBChangeOnResize cbP{ XMMatrixTranspose(gProj) };
  g_ctx->UpdateSubresource(g_cbProj.Get(), 0, nullptr, &cbP, 0, 0);

  return S_OK;
}

// =============================
// Cleanup
// =============================
static void CleanupDevice() {
  if (g_ctx) g_ctx->ClearState();
  g_rsWire.Reset(); g_rsSolid.Reset();
  g_samp.Reset(); g_srv.Reset();
  g_cbFrame.Reset(); g_cbProj.Reset(); g_cbView.Reset();
  g_ib.Reset(); g_vb.Reset(); g_layout.Reset();
  g_ps.Reset(); g_vs.Reset();
  g_dsv.Reset(); g_depth.Reset(); g_rtv.Reset();
  g_swap.Reset();

  g_devctx.destroy();
  g_ctx.Reset();

  g_device.destroy(); // libera m_device
}

// =============================
// Render
// =============================
static void Render() {
  // Tiempo
  static DWORD t0 = GetTickCount();
  DWORD t = GetTickCount();
  float secs = (t - t0) / 1000.0f;

  // Animación
  gWorld = DirectX::XMMatrixRotationY(secs);
  gMeshColor = DirectX::XMFLOAT4(
    (sinf(secs * 1.0f) + 1.f) * 0.5f,
    (cosf(secs * 3.0f) + 1.f) * 0.5f,
    (sinf(secs * 5.0f) + 1.f) * 0.5f,
    1.0f
  );

  // Clear
  float clear[4] = { 0.0f, 0.125f, 0.3f, 1.0f };
  g_ctx->ClearRenderTargetView(g_rtv.Get(), clear);
  g_ctx->ClearDepthStencilView(g_dsv.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

  // Actualiza CBuffer frame
  CBChangesEveryFrame cbF{};
  cbF.mWorld = DirectX::XMMatrixTranspose(gWorld);
  cbF.vMeshColor = gMeshColor;
  g_ctx->UpdateSubresource(g_cbFrame.Get(), 0, nullptr, &cbF, 0, 0);

  // Pipeline
  UINT stride = sizeof(SimpleVertex), offset = 0;
  ID3D11Buffer* vb = g_vb.Get();
  g_ctx->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
  g_ctx->IASetIndexBuffer(g_ib.Get(), DXGI_FORMAT_R16_UINT, 0);
  g_ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

  g_ctx->VSSetShader(g_vs.Get(), nullptr, 0);
  {
    ID3D11Buffer* cbs[] = { g_cbView.Get(), g_cbProj.Get(), g_cbFrame.Get() };
    g_ctx->VSSetConstantBuffers(0, 3, cbs);
  }

  g_ctx->PSSetShader(g_ps.Get(), nullptr, 0);
  { ID3D11Buffer* cb = g_cbFrame.Get(); g_ctx->PSSetConstantBuffers(2, 1, &cb); }
  { ID3D11ShaderResourceView* s = g_srv.Get(); g_ctx->PSSetShaderResources(0, 1, &s); }
  { ID3D11SamplerState* s = g_samp.Get(); g_ctx->PSSetSamplers(0, 1, &s); }

  g_ctx->DrawIndexed(36, 0, 0);
  g_swap->Present(1, 0); // VSync ON
}

// =============================
// WndProc
// =============================
static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  switch (msg) {
  case WM_SIZE:
    // TODO: recrear backbuffer/DSV y reproyección aquí si manejas resize.
    break;
  case WM_KEYDOWN:
    if (wParam == VK_ESCAPE) PostQuitMessage(0);
    if (wParam == VK_F1) {
      g_wire = !g_wire;
      g_ctx->RSSetState(g_wire ? g_rsWire.Get() : g_rsSolid.Get());
    }
    break;
  case WM_DESTROY:
    PostQuitMessage(0);
    break;
  case WM_PAINT: {
    PAINTSTRUCT ps; BeginPaint(hWnd, &ps); EndPaint(hWnd, &ps);
    break;
  }
  default:
    return DefWindowProcW(hWnd, msg, wParam, lParam);
  }
  return 0;
}
