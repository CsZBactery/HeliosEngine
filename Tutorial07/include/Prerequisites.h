#pragma once
// STD
#include <string>
#include <sstream>
#include <vector>
#include <thread>

// Win32 + Direct3D 11
#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>

// DirectXMath (moderna)
#include <DirectXMath.h>

// Si usas los macros de logging que ya tenías:
#define SAFE_RELEASE(x) if((x) != nullptr) { (x)->Release(); (x) = nullptr; }

#define MESSAGE(classObj, method, state) \
{ std::wostringstream os_; os_ << L#classObj L"::" L#method L" : " << L"[ " << L#state << L" ]\n"; \
  OutputDebugStringW(os_.str().c_str()); }

#define ERROR(classObj, method, errorMSG) \
{ try { std::wostringstream os_; os_ << L"ERROR : " << L#classObj << L"::" L#method \
  << L" : " << errorMSG << L"\n"; OutputDebugStringW(os_.str().c_str()); } \
  catch (...) { OutputDebugStringW(L"Failed to log error message.\n"); } }
