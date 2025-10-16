// HeliosEngine.cpp — DX11 con Buffer/MeshComponent wrappers
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
#include <cstring>
#include <memory>
#include <wincodec.h>

// Wrappers/headers propios
#include "../include/Window.h"
#include "../include/Device.h"
#include "../include/DeviceContext.h"
#include "../include/Buffer.h"          
#include "../include/MeshComponent.h"   

// Libs
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "windowscodecs.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "ole32.lib")

using Microsoft::WRL::ComPtr;
using namespace DirectX;

// =============================
// HLSL embebido
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

static HRESULT CompileFromSource(const char* src, const char* entry, const char* target, ID3DBlob** blobOut) {
    UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(_DEBUG)
    flags |= D3DCOMPILE_DEBUG;
#endif
    ComPtr<ID3DBlob> err, blob;
    HRESULT hr = D3DCompile(src, (UINT)std::strlen(src),
        nullptr, nullptr, nullptr, entry, target,
        flags, 0, &blob, &err);
    if (FAILED(hr)) {
        if (err) OutputDebugStringA((const char*)err->GetBufferPointer());
        return hr;
    }
    *blobOut = blob.Detach();
    return S_OK;
}

// =============================
// Carga de textura con WIC (helper global)
// =============================
static HRESULT LoadTextureWIC(
    ID3D11Device* device,
    ID3D11DeviceContext* ctx,
    const wchar_t* filename,
    ID3D11ShaderResourceView** outSRV,
    bool generateMips = true)
{
    if (!device || !outSRV || !filename) return E_INVALIDARG;
    *outSRV = nullptr;

    struct CoInitGuard {
        bool callUninit = false;
        CoInitGuard() {
            HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
            if (SUCCEEDED(hr)) callUninit = true;
        }
        ~CoInitGuard() { if (callUninit) CoUninitialize(); }
    } co;

    ComPtr<IWICImagingFactory>     factory;
    ComPtr<IWICBitmapDecoder>      decoder;
    ComPtr<IWICBitmapFrameDecode>  frame;
    ComPtr<IWICFormatConverter>    conv;

    HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(factory.GetAddressOf()));
    if (FAILED(hr)) return hr;

    hr = factory->CreateDecoderFromFilename(filename, nullptr, GENERIC_READ,
        WICDecodeMetadataCacheOnDemand, decoder.GetAddressOf());
    if (FAILED(hr)) return hr;

    hr = decoder->GetFrame(0, frame.GetAddressOf());
    if (FAILED(hr)) return hr;

    hr = factory->CreateFormatConverter(conv.GetAddressOf());
    if (FAILED(hr)) return hr;

    hr = conv->Initialize(frame.Get(), GUID_WICPixelFormat32bppRGBA,
        WICBitmapDitherTypeNone, nullptr, 0.0, WICBitmapPaletteTypeCustom);
    if (FAILED(hr)) return hr;

    UINT w = 0, h = 0;
    hr = conv->GetSize(&w, &h);
    if (FAILED(hr) || w == 0 || h == 0) return E_FAIL;

    const UINT stride = w * 4;
    const UINT size = stride * h;

    std::unique_ptr<BYTE[]> pixels(new (std::nothrow) BYTE[size]);
    if (!pixels) return E_OUTOFMEMORY;

    hr = conv->CopyPixels(nullptr, stride, size, pixels.get());
    if (FAILED(hr)) return hr;

    D3D11_TEXTURE2D_DESC td{};
    td.Width = w; td.Height = h; td.ArraySize = 1;
    td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    td.SampleDesc.Count = 1;
    td.MipLevels = generateMips ? 0u : 1u;
    td.Usage = generateMips ? D3D11_USAGE_DEFAULT : D3D11_USAGE_IMMUTABLE;
    td.BindFlags = D3D11_BIND_SHADER_RESOURCE | (generateMips ? D3D11_BIND_RENDER_TARGET : 0);
    td.MiscFlags = generateMips ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0;

    ComPtr<ID3D11Texture2D> tex;

    if (generateMips) {
        hr = device->CreateTexture2D(&td, nullptr, tex.GetAddressOf());
        if (FAILED(hr)) return hr;
        if (!ctx) return E_POINTER;
        ctx->UpdateSubresource(tex.Get(), 0, nullptr, pixels.get(), stride, 0);
    }
    else {
        D3D11_SUBRESOURCE_DATA srd{};
        srd.pSysMem = pixels.get();
        srd.SysMemPitch = stride;
        hr = device->CreateTexture2D(&td, &srd, tex.GetAddressOf());
        if (FAILED(hr)) return hr;
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC sd{};
    sd.Format = td.Format;
    sd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    sd.Texture2D.MostDetailedMip = 0;
    sd.Texture2D.MipLevels = generateMips ? -1 : 1;

    ComPtr<ID3D11ShaderResourceView> srv;
    hr = device->CreateShaderResourceView(tex.Get(), &sd, srv.GetAddressOf());
    if (FAILED(hr)) return hr;

    if (generateMips && ctx) ctx->GenerateMips(srv.Get());

    *outSRV = srv.Detach();
    return S_OK;
}

// =============================
// Datos / globals
// =============================
struct SimpleVertex { XMFLOAT3 Pos; XMFLOAT2 Tex; };
struct CBNeverChanges { XMMATRIX mView; };
struct CBChangeOnResize { XMMATRIX mProjection; };
struct CBChangesEveryFrame { XMMATRIX mWorld; XMFLOAT4 vMeshColor; };

static Window                           g_window;
static Device                           g_device;
static DeviceContext                    g_devctx;
static ComPtr<ID3D11DeviceContext>      g_ctx;
static ComPtr<IDXGISwapChain>           g_swap;
static ComPtr<ID3D11RenderTargetView>   g_rtv;
static ComPtr<ID3D11Texture2D>          g_depth;
static ComPtr<ID3D11DepthStencilView>   g_dsv;
static ComPtr<ID3D11DepthStencilState>  g_dss;
static ComPtr<ID3D11VertexShader>       g_vs;
static ComPtr<ID3D11PixelShader>        g_ps;
static ComPtr<ID3D11InputLayout>        g_layout;
// --- Buffers ahora son wrappers:
static Buffer                           g_vb;
static Buffer                           g_ib;
static Buffer                           g_cbView, g_cbProj, g_cbFrame;
static UINT                             g_indexCount = 0;

static ComPtr<ID3D11ShaderResourceView> g_srv;
static ComPtr<ID3D11SamplerState>       g_samp;
static ComPtr<ID3D11RasterizerState>    g_rsSolid, g_rsWire;
static bool                             g_wire = false;

static XMMATRIX g_world, g_view, g_proj;
static XMFLOAT4 g_meshColor(0.7f, 0.7f, 0.7f, 1.0f);

// =============================
// RTV/DSV/Viewport helpers
// =============================
static HRESULT CreateBackbufferTargets(UINT w, UINT h)
{
    // RTV desde backbuffer
    g_rtv.Reset();
    ComPtr<ID3D11Texture2D> back;
    HRESULT hr = g_swap->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)back.GetAddressOf());
    if (FAILED(hr)) return hr;

    hr = g_device.m_device->CreateRenderTargetView(back.Get(), nullptr, g_rtv.ReleaseAndGetAddressOf());
    if (FAILED(hr)) return hr;

    // Depth + DSV
    g_dsv.Reset(); g_depth.Reset();
    D3D11_TEXTURE2D_DESC d{};
    d.Width = w; d.Height = h;
    d.MipLevels = 1; d.ArraySize = 1;
    d.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    d.SampleDesc.Count = 1;
    d.Usage = D3D11_USAGE_DEFAULT;
    d.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    hr = g_device.m_device->CreateTexture2D(&d, nullptr, &g_depth);
    if (FAILED(hr)) return hr;
    hr = g_device.m_device->CreateDepthStencilView(g_depth.Get(), nullptr, &g_dsv);
    if (FAILED(hr)) return hr;

    // Depth-stencil state (una vez)
    if (!g_dss) {
        D3D11_DEPTH_STENCIL_DESC ds{};
        ds.DepthEnable = TRUE;
        ds.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        ds.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
        hr = g_device.m_device->CreateDepthStencilState(&ds, &g_dss);
        if (FAILED(hr)) return hr;
    }
    g_ctx->OMSetDepthStencilState(g_dss.Get(), 0);

    // Bind a OM
    ID3D11RenderTargetView* rtv = g_rtv.Get();
    g_ctx->OMSetRenderTargets(1, &rtv, g_dsv.Get());

    // Viewport
    D3D11_VIEWPORT vp{};
    vp.Width = (FLOAT)w; vp.Height = (FLOAT)h;
    vp.MinDepth = 0.0f;  vp.MaxDepth = 1.0f;
    g_ctx->RSSetViewports(1, &vp);

    return S_OK;
}

static HRESULT ResizeBackbuffer(UINT w, UINT h)
{
    if (!g_swap) return E_FAIL;

    g_ctx->OMSetRenderTargets(0, nullptr, nullptr);
    g_rtv.Reset(); g_dsv.Reset(); g_depth.Reset();

    HRESULT hr = g_swap->ResizeBuffers(0, w, h, DXGI_FORMAT_UNKNOWN, 0);
    if (FAILED(hr)) return hr;

    hr = CreateBackbufferTargets(w, h);
    if (FAILED(hr)) return hr;

    // reproyección
    g_proj = XMMatrixPerspectiveFovLH(XM_PIDIV4, (float)w / (float)h, 0.01f, 100.0f);
    CBChangeOnResize cbP{ XMMatrixTranspose(g_proj) };
    g_cbProj.update(g_devctx, nullptr, 0, nullptr, &cbP, 0, 0);

    return S_OK;
}

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

    wchar_t wd[MAX_PATH]{}; GetCurrentDirectoryW(MAX_PATH, wd);
    OutputDebugStringW((std::wstring(L"[DBG] WD = ") + wd + L"\n").c_str());

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
    D3D_FEATURE_LEVEL req[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0 };
    D3D_FEATURE_LEVEL out{};

    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags,
        req, (UINT)_countof(req),
        D3D11_SDK_VERSION, &sd, &g_swap, &g_device.m_device, &out, &g_ctx);
    if (FAILED(hr)) return hr;

    g_devctx.attach(g_ctx.Get());

    // RTV / DSV / Viewport
    hr = CreateBackbufferTargets(g_window.width(), g_window.height());
    if (FAILED(hr)) return hr;

    // Shaders
    ComPtr<ID3DBlob> vsBlob, psBlob;
    if (FAILED(CompileFromSource(g_HLSL, "VS", "vs_4_0", &vsBlob))) return E_FAIL;
    if (FAILED(CompileFromSource(g_HLSL, "PS", "ps_4_0", &psBlob))) return E_FAIL;
    if (FAILED(g_device.m_device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &g_vs))) return E_FAIL;
    if (FAILED(g_device.m_device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &g_ps))) return E_FAIL;

    // Input layout
    D3D11_INPUT_ELEMENT_DESC il[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    if (FAILED(g_device.m_device->CreateInputLayout(il, 2, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &g_layout))) return E_FAIL;
    g_ctx->IASetInputLayout(g_layout.Get());

    // ===== Geometría (cubo) vía MeshComponent + Buffer =====
    SimpleVertex vertices[] = {
        { {-1,  1, -1}, {0,0} }, { { 1,  1, -1}, {1,0} }, { { 1,  1,  1}, {1,1} }, { {-1,  1,  1}, {0,1} },
        { {-1, -1, -1}, {0,0} }, { { 1, -1, -1}, {1,0} }, { { 1, -1,  1}, {1,1} }, { {-1, -1,  1}, {0,1} },
        { {-1, -1,  1}, {0,0} }, { {-1, -1, -1}, {1,0} }, { {-1,  1, -1}, {1,1} }, { {-1,  1,  1}, {0,1} },
        { { 1, -1,  1}, {0,0} }, { { 1, -1, -1}, {1,0} }, { { 1,  1, -1}, {1,1} }, { { 1,  1,  1}, {0,1} },
        { {-1, -1, -1}, {0,0} }, { { 1, -1, -1}, {1,0} }, { { 1,  1, -1}, {1,1} }, { {-1,  1, -1}, {0,1} },
        { {-1, -1,  1}, {0,0} }, { { 1, -1,  1}, {1,0} }, { { 1,  1,  1}, {1,1} }, { {-1,  1,  1}, {0,1} },
    };
    uint16_t indices[] = {
        3,1,0,  2,1,3,  6,4,5,  7,4,6,  11,9,8, 10,9,11,
        14,12,13, 15,12,14, 19,17,16, 18,17,19, 22,20,21, 23,20,22
    };

    MeshComponent mesh;
    {
        std::vector<SimpleVertex> v(vertices, vertices + _countof(vertices));
        std::vector<uint16_t>     i(indices, indices + _countof(indices));
        mesh.setVertices(v);
        mesh.setIndices(i);
    }
    g_indexCount = mesh.indexCount();

    if (FAILED(g_vb.init(g_device, mesh, D3D11_BIND_VERTEX_BUFFER))) return E_FAIL;
    if (FAILED(g_ib.init(g_device, mesh, D3D11_BIND_INDEX_BUFFER)))  return E_FAIL;

    // ===== Constant buffers vía Buffer =====
    if (FAILED(g_cbView.init(g_device, sizeof(CBNeverChanges))))   return E_FAIL;
    if (FAILED(g_cbProj.init(g_device, sizeof(CBChangeOnResize)))) return E_FAIL;
    if (FAILED(g_cbFrame.init(g_device, sizeof(CBChangesEveryFrame)))) return E_FAIL;

    // ===== Textura (WIC) =====
    g_srv.Reset();
    const wchar_t* kTexturePath = L"Assets\\Textures\\checker_256.png";
    HRESULT thr = LoadTextureWIC(g_device.m_device, g_ctx.Get(), kTexturePath,
        g_srv.ReleaseAndGetAddressOf(), true);
    if (FAILED(thr) || !g_srv) {
        OutputDebugStringW(L"[ERR] No pude cargar checker_256.png. Uso fallback 1x1 blanco.\n");
        UINT32 white = 0xFFFFFFFF;
        D3D11_TEXTURE2D_DESC td{}; td.Width = 1; td.Height = 1; td.MipLevels = 1; td.ArraySize = 1;
        td.Format = DXGI_FORMAT_R8G8B8A8_UNORM; td.SampleDesc.Count = 1; td.Usage = D3D11_USAGE_IMMUTABLE; td.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        D3D11_SUBRESOURCE_DATA texSRD{}; texSRD.pSysMem = &white; texSRD.SysMemPitch = sizeof(white);
        ComPtr<ID3D11Texture2D> tex;
        g_device.m_device->CreateTexture2D(&td, &texSRD, &tex);
        g_device.m_device->CreateShaderResourceView(tex.Get(), nullptr, &g_srv);
    }
    else {
        ComPtr<ID3D11Resource> res; g_srv->GetResource(&res);
        if (res) {
            ComPtr<ID3D11Texture2D> t2d;
            if (SUCCEEDED(res.As(&t2d)) && t2d) {
                D3D11_TEXTURE2D_DESC td{}; t2d->GetDesc(&td);
                char buf[256];
                sprintf_s(buf, "[DBG] Texture loaded: %ux%u, mips=%u, fmt=%d\n",
                    td.Width, td.Height, td.MipLevels, (int)td.Format);
                OutputDebugStringA(buf);
            }
        }
    }

    // Sampler
    D3D11_SAMPLER_DESC sdsc{};
    sdsc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sdsc.AddressU = sdsc.AddressV = sdsc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sdsc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sdsc.MinLOD = 0; sdsc.MaxLOD = D3D11_FLOAT32_MAX;
    if (FAILED(g_device.m_device->CreateSamplerState(&sdsc, &g_samp))) return E_FAIL;

    // Rasterizer
    D3D11_RASTERIZER_DESC rs{}; rs.FillMode = D3D11_FILL_SOLID; rs.CullMode = D3D11_CULL_BACK;
    g_device.m_device->CreateRasterizerState(&rs, &g_rsSolid);
    rs.FillMode = D3D11_FILL_WIREFRAME;
    g_device.m_device->CreateRasterizerState(&rs, &g_rsWire);
    g_ctx->RSSetState(g_rsSolid.Get());

    // Cámaras
    g_world = XMMatrixIdentity();
    auto Eye = XMVectorSet(0.0f, 3.0f, -6.0f, 0.0f);
    auto At = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    auto Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    g_view = XMMatrixLookAtLH(Eye, At, Up);
    CBNeverChanges cbV{ XMMatrixTranspose(g_view) };
    g_cbView.update(g_devctx, nullptr, 0, nullptr, &cbV, 0, 0);

    g_proj = XMMatrixPerspectiveFovLH(XM_PIDIV4,
        (float)g_window.width() / (float)g_window.height(), 0.01f, 100.0f);
    CBChangeOnResize cbP{ XMMatrixTranspose(g_proj) };
    g_cbProj.update(g_devctx, nullptr, 0, nullptr, &cbP, 0, 0);

    return S_OK;
}

// =============================
// Cleanup
// =============================
static void CleanupDevice() {
    if (g_ctx) g_ctx->ClearState();
    g_rsWire.Reset(); g_rsSolid.Reset();
    g_samp.Reset(); g_srv.Reset();

    g_cbFrame.destroy(); g_cbProj.destroy(); g_cbView.destroy();
    g_ib.destroy(); g_vb.destroy();

    g_layout.Reset();
    g_ps.Reset(); g_vs.Reset();
    g_dsv.Reset(); g_depth.Reset(); g_rtv.Reset();
    g_swap.Reset();
    g_devctx.destroy();
    g_ctx.Reset();
    g_device.destroy();
}

// =============================
// Render
// =============================
static void Render() {
    static ULONGLONG t0 = GetTickCount64();
    ULONGLONG t = GetTickCount64();
    float secs = float(t - t0) / 1000.0f;

    g_world = XMMatrixRotationY(secs);
    g_meshColor = XMFLOAT4(
        (sinf(secs * 1.0f) + 1.f) * 0.5f,
        (cosf(secs * 3.0f) + 1.f) * 0.5f,
        (sinf(secs * 5.0f) + 1.f) * 0.5f,
        1.0f
    );

    float clear[4] = { 0.02f, 0.07f, 0.16f, 1.0f };
    g_ctx->ClearRenderTargetView(g_rtv.Get(), clear);
    g_ctx->ClearDepthStencilView(g_dsv.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    CBChangesEveryFrame cbF{};
    cbF.mWorld = XMMatrixTranspose(g_world);
    cbF.vMeshColor = g_meshColor;
    g_cbFrame.update(g_devctx, nullptr, 0, nullptr, &cbF, 0, 0);

    // IA
    g_vb.render(g_devctx, 0, 1);                 // IASetVertexBuffers
    g_ib.render(g_devctx, 0, 1, false);          // IASetIndexBuffer (usa formato del mesh)
    g_devctx.IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Shaders + CBs
    g_ctx->VSSetShader(g_vs.Get(), nullptr, 0);
    g_cbView.render(g_devctx, 0, 1);             // b0 VS
    g_cbProj.render(g_devctx, 1, 1);             // b1 VS
    g_cbFrame.render(g_devctx, 2, 1, true);      // b2 VS + PS

    g_ctx->PSSetShader(g_ps.Get(), nullptr, 0);
    { ID3D11ShaderResourceView* s = g_srv.Get(); g_ctx->PSSetShaderResources(0, 1, &s); }
    { ID3D11SamplerState* s = g_samp.Get(); g_ctx->PSSetSamplers(0, 1, &s); }

    // Draw
    g_devctx.DrawIndexed(g_indexCount, 0, 0);
    g_swap->Present(1, 0);
}

// =============================
// WndProc (con resize)
// =============================
static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_SIZE:
        if (g_ctx && g_swap) {
            UINT newW = LOWORD(lParam), newH = HIWORD(lParam);
            if (newW > 0 && newH > 0) ResizeBackbuffer(newW, newH);
        }
        return 0;
    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE) PostQuitMessage(0);
        if (wParam == VK_F1) { g_wire = !g_wire; g_ctx->RSSetState(g_wire ? g_rsWire.Get() : g_rsSolid.Get()); }
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0); return 0;
    case WM_PAINT: {
        PAINTSTRUCT ps; BeginPaint(hWnd, &ps); EndPaint(hWnd, &ps); return 0;
    }
    }
    return DefWindowProcW(hWnd, msg, wParam, lParam);
}
