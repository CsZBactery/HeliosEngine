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

// Inclusiones adicionales necesarias para tu proyecto
#include "Model3D.h"
#include "UserInterface.h"
#include "ECS/Actor.h"

class BaseApp {
public:
    // Tu constructor personalizado (necesario para tu .cpp)
    BaseApp(HINSTANCE hInst, int nCmdShow);

    // Destructor
    ~BaseApp() { destroy(); }

    int
        run(HINSTANCE hInst, int nCmdShow);

    HRESULT
        init();

    void
        update(float deltaTime);

    void
        render();

    void
        destroy();

private:
    static LRESULT CALLBACK
        WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
    Window                          m_window;
    Device                          m_device;
    DeviceContext                   m_deviceContext;
    SwapChain                       m_swapChain;
    Texture                         m_backBuffer;
    RenderTargetView                m_renderTargetView;
    Texture                         m_depthStencil;
    DepthStencilView                m_depthStencilView;
    Viewport                        m_viewport;
    ShaderProgram                   m_shaderProgram;

    // MeshComponent                 m_mesh;
    // Buffer                        m_vertexBuffer;
    // Buffer                        m_indexBuffer;

    Buffer                          m_cbNeverChanges;
    Buffer                          m_cbChangeOnResize;
    Buffer                          m_cbChangesEveryFrame;

    Texture                         m_cyberGunAlbedo;       // Tu textura especifica
    SamplerState                    m_samplerState;

    // Matrices y Variables Globales
    XMMATRIX                        m_World;
    XMMATRIX                        m_View;
    XMMATRIX                        m_Projection;
    XMFLOAT4                        m_vMeshColor;

    // Actores
    std::vector<EU::TSharedPointer<Actor>> m_actors;
    EU::TSharedPointer<Actor>              m_cyberGun;

    // Recursos
    Model3D* m_model;

    // Estructuras de Constant Buffers
    CBChangeOnResize                    cbChangesOnResize;
    CBNeverChanges                      cbNeverChanges;
    CBChangesEveryFrame                 cb;

    // Interfaz de Usuario
    UserInterface                       UI;
};