#include <d3d11.h>
#include <d3dx11.h>   
#include <wrl/client.h>
#include <string>
#include "../include/DDSTextureLoader.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dx11.lib")

using Microsoft::WRL::ComPtr;

namespace DirectX
{
    static inline bool HasExtensionDDS(const wchar_t* path)
    {
        if (!path) return false;
        size_t len = wcslen(path);
        if (len < 4) return false;
        const wchar_t* ext = path + (len - 4);
        return (_wcsicmp(ext, L".dds") == 0);
    }

    HRESULT CreateDDSTextureFromFile(
        ID3D11Device* d3dDevice,
        const wchar_t* szFileName,
        ID3D11Resource** texture,
        ID3D11ShaderResourceView** textureView,
        size_t /*maxsize*/,
        DDS_ALPHA_MODE* alphaMode) noexcept
    {
        if (!d3dDevice || !szFileName || (!texture && !textureView))
            return E_INVALIDARG;

        if (!HasExtensionDDS(szFileName))
            return E_INVALIDARG; // esta versión mínima solo maneja .dds

        ComPtr<ID3D11ShaderResourceView> srv;
        HRESULT hr = D3DX11CreateShaderResourceViewFromFileW(
            d3dDevice,
            szFileName,
            nullptr,        // D3DX11_IMAGE_LOAD_INFO*
            nullptr,        // ID3DX11ThreadPump*
            srv.GetAddressOf(),
            nullptr         // HRESULT* pHResult
        );
        if (FAILED(hr))
            return hr;

        // devolver opcionalmente la SRV
        if (textureView) {
            *textureView = srv.Detach();
        }

        // si pidieron el resource base, lo resolvemos desde la SRV
        if (texture) {
            if (!*textureView) {
                // si no se pidió SRV, necesitamos pedirla localmente
                srv.Attach(*textureView);
            }
            ComPtr<ID3D11Resource> res;
            (*textureView)->GetResource(res.GetAddressOf());
            *texture = res.Detach();
        }

        if (alphaMode) {
            *alphaMode = DDS_ALPHA_MODE_UNKNOWN; // no analizamos el contenedor aquí
        }

        return S_OK;
    }

    HRESULT CreateDDSTextureFromFile(
        ID3D11Device* d3dDevice,
        ID3D11DeviceContext* /*d3dContext*/,
        const wchar_t* szFileName,
        ID3D11Resource** texture,
        ID3D11ShaderResourceView** textureView,
        size_t maxsize,
        DDS_ALPHA_MODE* alphaMode) noexcept
    {
        // En esta versión mínima, ignoramos d3dContext y llamamos al overload simple.
        return CreateDDSTextureFromFile(
            d3dDevice, szFileName, texture, textureView, maxsize, alphaMode);
    }

    HRESULT CreateDDSTextureFromMemory(
        ID3D11Device* /*d3dDevice*/,
        const uint8_t* /*ddsData*/,
        size_t /*ddsDataSize*/,
        ID3D11Resource** /*texture*/,
        ID3D11ShaderResourceView** /*textureView*/,
        size_t /*maxsize*/,
        DDS_ALPHA_MODE* /*alphaMode*/) noexcept
    {
        // Stub mínimo para que enlace. Implementa si más adelante lo necesitas.
        return E_NOTIMPL;
    }

    HRESULT CreateDDSTextureFromMemory(
        ID3D11Device* /*d3dDevice*/,
        ID3D11DeviceContext* /*d3dContext*/,
        const uint8_t* /*ddsData*/,
        size_t /*ddsDataSize*/,
        ID3D11Resource** /*texture*/,
        ID3D11ShaderResourceView** /*textureView*/,
        size_t /*maxsize*/,
        DDS_ALPHA_MODE* /*alphaMode*/) noexcept
    {
        // Stub mínimo para que enlace. Implementa si más adelante lo necesitas.
        return E_NOTIMPL;
    }
}
