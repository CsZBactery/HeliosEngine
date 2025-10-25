#pragma once

#include <d3d11.h>
#include <cstddef>
#include <cstdint>

namespace DirectX
{
    // Modo de alpha leido del DDS (opcional)
    enum DDS_ALPHA_MODE : uint32_t
    {
        DDS_ALPHA_MODE_UNKNOWN = 0,
        DDS_ALPHA_MODE_STRAIGHT = 1,
        DDS_ALPHA_MODE_PREMULTIPLIED = 2,
        DDS_ALPHA_MODE_OPAQUE = 3,
        DDS_ALPHA_MODE_CUSTOM = 4,
    };

    // Cargar desde archivo (sin autogenerar mips)
    HRESULT CreateDDSTextureFromFile(
        ID3D11Device* d3dDevice,
        const wchar_t* szFileName,
        ID3D11Resource** texture = nullptr,   // opcional
        ID3D11ShaderResourceView** textureView = nullptr,   // opcional
        size_t                      maxsize = 0,         // 0 = sin límite
        DDS_ALPHA_MODE* alphaMode = nullptr    // opcional (out)
    ) noexcept;

    // Cargar desde archivo con contexto (permite autogenerar mips si procede)
    HRESULT CreateDDSTextureFromFile(
        ID3D11Device* d3dDevice,
        ID3D11DeviceContext* d3dContext,                   // puede ser nullptr si no quieres autogenerar mips
        const wchar_t* szFileName,
        ID3D11Resource** texture = nullptr,
        ID3D11ShaderResourceView** textureView = nullptr,
        size_t                      maxsize = 0,
        DDS_ALPHA_MODE* alphaMode = nullptr
    ) noexcept;

    // (Opcional) Cargar desde memoria — por si más adelante lo necesitas
    HRESULT CreateDDSTextureFromMemory(
        ID3D11Device* d3dDevice,
        const uint8_t* ddsData,
        size_t                      ddsDataSize,
        ID3D11Resource** texture = nullptr,
        ID3D11ShaderResourceView** textureView = nullptr,
        size_t                      maxsize = 0,
        DDS_ALPHA_MODE* alphaMode = nullptr
    ) noexcept;

    // (Opcional) Memoria + contexto (autogenerar mips)
    HRESULT CreateDDSTextureFromMemory(
        ID3D11Device* d3dDevice,
        ID3D11DeviceContext* d3dContext,
        const uint8_t* ddsData,
        size_t                      ddsDataSize,
        ID3D11Resource** texture = nullptr,
        ID3D11ShaderResourceView** textureView = nullptr,
        size_t                      maxsize = 0,
        DDS_ALPHA_MODE* alphaMode = nullptr
    ) noexcept;
} // namespace DirectX
