#pragma once
#include "Prerequisites.h"
#include "Window.h"
#include "Device.h"
#include "DeviceContext.h"
#include "SwapChain.h"
#include "Texture.h"
#include "RenderTargetView.h"
#include "DepthStencilView.h"
#include "Viewport.h"
#include "ShaderProgram.h"
#include "MeshComponent.h"
#include "Buffer.h"
#include "SamplerState.h"
#include "Model3D.h"
#include "ECS/Actor.h"

class BaseApp {
public:
    /**
     * @brief Constructor por defecto (Sin argumentos, estilo referencia).
     */
    BaseApp() = default;

    /**
     * @brief Destructor.
     */
    ~BaseApp() { destroy(); }

    HRESULT awake();

    int run(HINSTANCE hInst, int nCmdShow);

    HRESULT init();

    void update(float deltaTime);

    void render();

    void destroy();

private:
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
    // ------------------------------------------------------------------------
    // CORE DX11
    // ------------------------------------------------------------------------
    Window              m_window;
    Device              m_device;
    DeviceContext       m_deviceContext;
    SwapChain           m_swapChain;
    Texture             m_backBuffer;
    RenderTargetView    m_renderTargetView;
    Texture             m_depthStencil;
    DepthStencilView    m_depthStencilView;
    Viewport            m_viewport;
    ShaderProgram       m_shaderProgram;

    // ------------------------------------------------------------------------
    // BUFFERS
    // ------------------------------------------------------------------------
    Buffer              m_cbNeverChanges;
    Buffer              m_cbChangeOnResize;

    // ------------------------------------------------------------------------
    // ASSETS (Aquí corregimos los nombres para que coincidan con tu .cpp)
    // ------------------------------------------------------------------------
    Texture             m_repsolTexture; // Antes m_cyberGunAlbedo

    // ------------------------------------------------------------------------
    // MATRICES
    // ------------------------------------------------------------------------
    XMMATRIX            m_View;
    XMMATRIX            m_Projection;

    // ------------------------------------------------------------------------
    // ACTORES
    // ------------------------------------------------------------------------
    std::vector<EU::TSharedPointer<Actor>> m_actors;
    EU::TSharedPointer<Actor>              m_repsolActor; // Antes m_cyberGun

    Model3D* m_model;

    // ------------------------------------------------------------------------
    // STRUCTS CPU
    // ------------------------------------------------------------------------
    CBChangeOnResize    cbChangesOnResize;
    CBNeverChanges      cbNeverChanges;
};