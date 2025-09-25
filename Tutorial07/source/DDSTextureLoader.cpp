// #include "pch.h"   // (solo si usas PCH)

#include "WICTextureLoader.h"    // o "DDSTextureLoader.h"
#include "DirectXHelpers.h"
#include "PlatformHelpers.h"
#include "LoaderHelpers.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#define NOMINMAX

#include <Windows.h>
#include <d3d11.h>
#include <dxgiformat.h>
#include <wrl/client.h>
#include <wincodec.h>       // (solo WICTextureLoader.cpp)
#include <propvarutil.h>    // (solo WICTextureLoader.cpp)
#include <iterator>
#include <tuple>
#include <cstring>
#include <memory>
#include <algorithm>
#include <vector>

using namespace DirectX;
// Evita: using namespace DirectX::LoaderHelpers;   (mejor califica)
