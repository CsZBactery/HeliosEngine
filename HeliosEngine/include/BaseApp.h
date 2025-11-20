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
#include "ModelLoader.h"
#include "Buffer.h"
#include "SamplerState.h"
#include "ModelLoader.h"

// Si usas funciones antiguas de D3DX para cargar texturas (opcional)
#include <d3d11.h>
#ifdef _HAS_D3DX11    // define esto en tu proyecto si de verdad usas D3DX
#include <d3dx11.h>
#endif

/**
 * @file BaseApp.h
 * @brief Orquesta la ventana, inicializa D3D11 y ejecuta el game loop.
 */
class BaseApp {
public:
    BaseApp(HINSTANCE hInst, int nCmdShow);
    ~BaseApp() { destroy(); }

    int     run(HINSTANCE hInst, int nCmdShow);
    HRESULT init();
    void    update(float deltaTime);
    void    render();
    void    destroy();

private:
    // WndProc estático (guardamos this en GWLP_USERDATA)
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    // --- Plataforma / Dispositivo ---
    Window        m_window;
    Device        m_device;
    DeviceContext m_deviceContext;
    SwapChain     m_swapChain;

    // --- Render targets / profundidad ---
    Texture          m_backBuffer;
    RenderTargetView m_renderTargetView;
    Texture          m_depthStencil;
    DepthStencilView m_depthStencilView;
    Viewport         m_viewport;

    // --- Pipeline programable ---
    ShaderProgram m_shaderProgram;

    // --- Geometría y buffers ---
    MeshComponent m_mesh;
    Buffer        m_vertexBuffer;
    Buffer        m_indexBuffer;
    Buffer        m_cbNeverChanges;       // b0 (view)
    Buffer        m_cbChangeOnResize;     // b1 (projection)
    Buffer        m_cbChangesEveryFrame;  // b2 (world/color)
    Texture       m_textureCube;          // Wrapper de textura (opcional)
    SamplerState  m_samplerState;

    // --- Transformaciones / cámara ---
    XMMATRIX m_World;
    XMMATRIX m_View;
    XMMATRIX m_Projection;
    XMFLOAT4 m_vMeshColor{ 1, 1, 1, 1 };

    // Textura directa (cuando cargas con D3DX/stb)
    ID3D11ShaderResourceView* m_pModelTextureSRV = nullptr;

    // --- Cámara y animación ---
    float m_cameraDistance = 6.0f;   // zoom base (rueda del mouse)
    float m_spinAngle = 0.0f;   // rotación del modelo (radianes)
    float m_orbitAngle = 0.0f;   // órbita de cámara (radianes)
    float m_spinSpeedDeg = 20.0f;  // vel. giro del modelo (grados/seg)
    float m_orbitSpeedDeg = 10.0f;  // vel. órbita de cámara (grados/seg)

    // Entrada
    void onMouseWheel(int zDelta);

    // --- Payloads CPU para Constant Buffers ---
    CBChangeOnResize    cbChangesOnResize;
    CBNeverChanges      cbNeverChanges;
    CBChangesEveryFrame cb;
};
