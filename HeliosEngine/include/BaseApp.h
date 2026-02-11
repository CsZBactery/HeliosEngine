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

// Includes del Profe
#include "EngineUtilities/GUI/GUI.h"
#include "SceneGraph/SceneGraph.h"
#include "EngineUtilities/Utilities/Camera.h"

// Declaración externa para que Windows pueda manejar los inputs de ImGui
extern IMGUI_IMPL_API
LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

class
    BaseApp {
public:
    BaseApp() = default;
    ~BaseApp() { destroy(); }

    HRESULT
        awake();

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
    Window                              m_window;
    Device                              m_device;
    DeviceContext                       m_deviceContext;
    SwapChain                           m_swapChain;
    Texture                             m_backBuffer;
    RenderTargetView                    m_renderTargetView;
    Texture                             m_depthStencil;
    DepthStencilView                    m_depthStencilView;
    Viewport                            m_viewport;
    ShaderProgram                       m_shaderProgram;

    Buffer                              m_cbNeverChanges;
    Buffer                              m_cbChangeOnResize;

    // Assets
    Texture                             m_repsolTexture; // Tu textura (Moto)
    Texture                             m_skyboxTex;     // Nueva textura (Skybox)

    // Cámara y Matrices
    Camera                              m_camera;        // Nueva clase Cámara
    //XMMATRIX                          m_View;          // (Comentado: gestionado por Camera)
    //XMMATRIX                          m_Projection;    // (Comentado: gestionado por Camera)

    // Escena y Actores
    SceneGraph                          m_sceneGraph;
    std::vector<EU::TSharedPointer<Actor>> m_actors;
    EU::TSharedPointer<Actor>              m_repsolActor; // Tu actor (Moto)


    Model3D* m_model;

    // Constant Buffers CPU
    CBChangeOnResize                    cbChangesOnResize;
    CBNeverChanges                      cbNeverChanges;

    // GUI
    GUI                                 m_gui;
};