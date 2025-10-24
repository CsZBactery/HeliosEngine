#pragma once
/**
 * @file RenderTargetView.h
 * @brief Wrapper mínimo para crear y gestionar un Render Target View (RTV) en D3D11.
 *
 * Funciona con el estilo de los wrappers del profe:
 *  - init(Device&, Texture&, DXGI_FORMAT)
 *  - render(DeviceContext&, DepthStencilView&, NumViews, ClearColor)
 *  - destroy()
 */

#include "../include/Prerequisites.h"

 // Adelantos para evitar ciclos de include
class Device;
class DeviceContext;
class Texture;
class DepthStencilView;

/**
 * @class RenderTargetView
 * @brief Encapsula un @c ID3D11RenderTargetView y su ciclo de vida.
 */
class RenderTargetView {
public:
    RenderTargetView() = default;
    ~RenderTargetView() { destroy(); }

    RenderTargetView(const RenderTargetView&) = delete;
    RenderTargetView& operator=(const RenderTargetView&) = delete;

    /**
     * @brief Libera el @c ID3D11RenderTargetView si existe.
     */
    void destroy();

    /**
     * @brief Crea el RTV desde el backbuffer (índice 0) de una swap chain.
     * @param swap  Swap chain válida (no nula).
     * @param dev   Dispositivo D3D11 usado para crear la vista.
     */
    HRESULT createFromBackbuffer(IDXGISwapChain* swap, ID3D11Device* dev);

    /**
     * @brief Crea el RTV a partir de una textura 2D existente.
     * @param dev   Dispositivo D3D11.
     * @param tex   Textura 2D origen (no nula).
     * @param desc  Descriptor opcional del RTV; si es nullptr se infiere.
     */
    HRESULT createFromTexture(ID3D11Device* dev, ID3D11Texture2D* tex,
        const D3D11_RENDER_TARGET_VIEW_DESC* desc = nullptr);

    /**
     * @brief Inicializa el RTV desde el backbuffer (estilo profe).
     * @param device      Wrapper del dispositivo.
     * @param backBuffer  Wrapper de la textura del backbuffer.
     * @param format      Formato del RTV (ej. DXGI_FORMAT_R8G8B8A8_UNORM).
     */
    HRESULT init(Device& device, Texture& backBuffer, DXGI_FORMAT format);

    /**
     * @brief Hace bind del RTV, del DSV y limpia el color (estilo profe).
     * @param deviceContext Contexto inmediato.
     * @param dsv           Wrapper de DepthStencilView a enlazar.
     * @param NumViews      Número de RTVs (normalmente 1).
     * @param ClearColor    RGBA para ClearRenderTargetView.
     */
    void render(DeviceContext& deviceContext,
        DepthStencilView& dsv,
        unsigned int NumViews,
        const float ClearColor[4]);

    /// Acceso crudo
    ID3D11RenderTargetView* get() const { return m_rtv; }

private:
    ID3D11RenderTargetView* m_rtv = nullptr;
};
