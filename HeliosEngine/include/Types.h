#pragma once
// Types.h — tipos compartidos del engine (vértices y constant-buffers)

#include <DirectXMath.h>
using namespace DirectX;

// ---------------------------------------
// Vértices
// ---------------------------------------
struct SimpleVertex
{
    XMFLOAT3 Pos;
    XMFLOAT2 Tex;
};

// ---------------------------------------
// Constant buffers (alineados a 16 bytes)
// ---------------------------------------
struct alignas(16) CBNeverChanges
{
    XMMATRIX mView;
};

struct alignas(16) CBChangeOnResize
{
    XMMATRIX mProjection;
};

struct alignas(16) CBChangesEveryFrame
{
    XMMATRIX mWorld;
    XMFLOAT4 vMeshColor; // 16B
};

// Validaciones de alineación (deja estos aquí, NO los repitas en otros archivos)
static_assert(alignof(CBNeverChanges) == 16, "CBNeverChanges must be 16-byte aligned.");
static_assert(alignof(CBChangeOnResize) == 16, "CBChangeOnResize must be 16-byte aligned.");
static_assert(alignof(CBChangesEveryFrame) == 16, "CBChangesEveryFrame must be 16-byte aligned.");
