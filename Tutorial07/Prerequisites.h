#pragma once

// Recorta Windows.h y evita choque con std::min/max
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif

// ==== Windows + COM ====
#include <windows.h>
#include <wrl/client.h>

// ==== DirectX 11 + DXGI + Shader Compiler (moderno, sin D3DX) ====
#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

// ==== C++ STL ====
#include <string>
#include <sstream>
#include <vector>
#include <thread>

// ==== Enlaces automáticos a libs ====
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

using Microsoft::WRL::ComPtr;
using namespace DirectX;

#ifndef ARRAYSIZE
#define ARRAYSIZE(a) (sizeof(a)/sizeof((a)[0]))
#endif

// ====== UTILIDADES ======
#define SAFE_RELEASE(x) do { if (x) { (x)->Release(); (x) = nullptr; } } while(0)

// Conversión helper (UTF-8 -> wide) para logs cuando te pasen char*
inline std::wstring _ToWide(const char* s) {
  if (!s) return L"";
  int len = MultiByteToWideChar(CP_UTF8, 0, s, -1, nullptr, 0);
  std::wstring w(len ? len - 1 : 0, L'\0');
  if (len) MultiByteToWideChar(CP_UTF8, 0, s, -1, w.data(), len);
  return w;
}

// Overloads de logging (aceptan char* o wchar_t*)
inline void _LogMessage(const wchar_t* cls, const wchar_t* method, const wchar_t* state) {
  std::wostringstream os; os << cls << L"::" << method << L" : [CREATION OF RESOURCE: " << state << L"]\n";
  OutputDebugStringW(os.str().c_str());
}
inline void _LogMessage(const char* cls, const char* method, const char* state) {
  _LogMessage(_ToWide(cls).c_str(), _ToWide(method).c_str(), _ToWide(state).c_str());
}
inline void _LogError(const wchar_t* cls, const wchar_t* method, const wchar_t* msg) {
  std::wostringstream os; os << L"ERROR : " << cls << L"::" << method << L" : " << msg << L"\n";
  OutputDebugStringW(os.str().c_str());
}
inline void _LogError(const char* cls, const char* method, const char* msg) {
  _LogError(_ToWide(cls).c_str(), _ToWide(method).c_str(), _ToWide(msg).c_str());
}

#define MESSAGE(classObj, method, state) _LogMessage(classObj, method, state)
#define ERROR(classObj, method, errorMSG) _LogError(classObj, method, errorMSG)

// ====== SHIM para compilar shaders (sustituye a D3DX11CompileFromFile) ======
// Usa D3DCompileFromFile (moderno). Si tu código viejo llama a D3DX11CompileFromFileW,
// puedes llamar directamente a esta función en su lugar.
inline HRESULT CompileShaderFromFile(
  LPCWSTR file, LPCSTR entry, LPCSTR model, ID3DBlob** blobOut,
  UINT flags = D3DCOMPILE_ENABLE_STRICTNESS
#if defined(_DEBUG)
  | D3DCOMPILE_DEBUG
#endif
) {
  ID3DBlob* errorBlob = nullptr;
  HRESULT hr = D3DCompileFromFile(
    file, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entry, model,
    flags, 0, blobOut, &errorBlob);
  if (FAILED(hr) && errorBlob) {
    OutputDebugStringA((const char*)errorBlob->GetBufferPointer());
    errorBlob->Release();
  }
  return hr;
}

// ====== OPCIONAL: Shim NO-OP para D3DX11CreateShaderResourceViewFromFile ======
// Esto solo evita errores de compilación si aún lo llamas.
// Devuelve E_NOTIMPL hasta que migres a DirectXTK (CreateWICTextureFromFile).
inline HRESULT D3DX11CreateShaderResourceViewFromFileW(
  ID3D11Device* device, LPCWSTR file,
  const D3D11_SHADER_RESOURCE_VIEW_DESC* desc,
  void* /*pPump*/, ID3D11ShaderResourceView** outSRV, HRESULT* pHResult)
{
  (void)device; (void)file; (void)desc; (void)outSRV;
  if (pHResult) *pHResult = E_NOTIMPL;
  return E_NOTIMPL;
}
#ifdef UNICODE
#define D3DX11CreateShaderResourceViewFromFile D3DX11CreateShaderResourceViewFromFileW
#endif

// ==== NO INCLUIR ====
// #include "Resource.h"
// #include "resource.h"
// (usa icono por defecto con IDI_APPLICATION; si quieres ícono propio, luego metemos .rc)
