/**
 * @file BaseApp.h
 * @brief Clase núcleo que gestiona el ciclo de vida y el bucle principal de HeliosEngine.
 */

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
#include "EngineUtilities/GUI/GUI.h"
#include "SceneGraph/SceneGraph.h"
#include "EngineUtilities/Utilities/Camera.h"
#include "EngineUtilities/Utilities/Skybox.h"
#include "EngineUtilities/Utilities/LayoutBuilder.h"
#include "EngineUtilities/Utilities/EditorViewportPass.h"

 // ==========================================
 // NUEVOS SISTEMAS DEL PROFESOR (ECS y Render)
 // ==========================================
#include "ECS/LightComponent.h"
#include "ECS/MeshRendererComponent.h"
#include "Rendering/Material.h"
#include "Rendering/MaterialInstance.h"
#include "Rendering/Mesh.h"
#include "Rendering/ForwardRenderer.h"
#include "Rendering/RenderScene.h"
#include <string>

#include <DirectXMath.h> // Necesario para XMFLOAT4X4

// ======================================================================================
// Declaración externa para el manejador de eventos de ImGui.
// ======================================================================================
extern IMGUI_IMPL_API
LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

/**
 * @class BaseApp
 * @brief Orquestador principal del motor.
 * @details Controla la inicialización de DirectX 11, el bucle de mensajes de Windows,
 * la actualización de la lógica (ECS/SceneGraph) y el renderizado de cada frame.
 */
class BaseApp {
public:
    BaseApp() = default;
    ~BaseApp() { destroy(); }

    /** @brief Primera fase de arranque: configuración de parámetros iniciales (Lógica). */
    HRESULT awake();

    /** @brief Inicia el bucle infinito de mensajes de Windows y el Game Loop. */
    int run(HINSTANCE hInst, int nCmdShow);

    /** @brief Inicialización de recursos de hardware (GPU), shaders y objetos 3D. */
    HRESULT init();

    /** @brief Actualiza la lógica de los actores, input y cálculo de matrices. */
    void update(float deltaTime);

    /** @brief Ejecuta las órdenes de dibujo y envía los píxeles al monitor. */
    void render();

    /** @brief Libera todos los recursos COM de DirectX y apaga el motor de forma segura. */
    void destroy();

    /** @brief Evento disparado cuando el usuario cambia el tamaño de la ventana de Windows. */
    void onResize(unsigned int newW, unsigned int newH);

    /** @brief Lógica interna para redimensionar la textura del Editor sin afectar la ventana. */
    void handleEditorViewportResize();

    // ==========================================
    // SISTEMA DE GUARDADO DE ESCENAS (NUEVO)
    // ==========================================
    bool saveScene(const std::string& path);
    bool loadScene(const std::string& path);
    std::string getDefaultScenePath() const;

private:
    /** @brief Callback estático del sistema operativo para procesar clics y teclas. */
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
    // ==========================================
    // INFRAESTRUCTURA DIRECTX 11
    // ==========================================
    Window            m_window;            /**< Ventana nativa de Windows (Lienzo). */
    Device            m_device;            /**< Fábrica de recursos en la tarjeta de video. */
    DeviceContext     m_deviceContext;     /**< Director de orquesta (ejecuta dibujos). */
    SwapChain         m_swapChain;         /**< Manejador de doble buffer (Front/Back buffer). */
    Texture           m_backBuffer;        /**< Lienzo de color final de la pantalla. */
    RenderTargetView  m_renderTargetView;  /**< Vista de salida para pintar color. */
    Texture           m_depthStencil;      /**< Buffer de memoria para cálculo de oclusión. */
    DepthStencilView  m_depthStencilView;  /**< Vista de pruebas de profundidad (Z-Buffer). */
    Viewport          m_viewport;          /**< Área de la pantalla donde se permite dibujar. */
    bool              m_d3dReady = false;  /**< Bandera para saber si la GPU ya está inicializada. */

    // ==========================================
    // SHADERS Y CONSTANT BUFFERS
    // ==========================================
    ShaderProgram     m_shaderProgram;        /**< Pipeline completo de Shaders (Vertex + Pixel). */
    Buffer            m_constantBuffer;       /**< Buffer unificado para reemplazar a los dos anteriores. */
    CBMain            m_constantBufferStruct; /**< Estructura de datos que se enviará al Shader. */

    // ==========================================
    // MATERIALES PBR (Texturas de alta fidelidad)
    // ==========================================
    Texture m_AlbedoSRV;    /**< Color base de TU MODELO (Xbox/Repsol). */
    Texture m_MetallicSRV;
    Texture m_RoughnessSRV;
    Texture m_AOSRV;
    Texture m_NormalSRV;
    Texture m_EmissiveSRV;  /**< Mapa de emisión de luz (NUEVO). */

    // Texturas extra del profe (Por si las llega a llamar en el .cpp para que no crashee)
    Texture m_drakefireAlbedoSRV;
    Texture m_drakefireNormalSRV;
    Texture m_drakefireMetallicSRV;
    Texture m_drakefireRoughnessSRV;
    Texture m_drakefireAOSRV;

    // ==========================================
    // ESCENA 3D Y ACTORES
    // ==========================================
    Camera                                 m_camera;                 /**< Lente a través del cual se ve el mundo. */
    EU::Vector3                            m_cameraPos;              /**< Posición actual de la cámara. */
    SceneGraph                             m_sceneGraph;             /**< Árbol jerárquico de la escena. */
    std::vector<EU::TSharedPointer<Actor>> m_actors;                 /**< Lista maestra de actores. */

    EU::TSharedPointer<Actor>              m_cyberGun;               /**< AQUÍ VA TU XBOX/REPSOL (Usamos este nombre por compatibilidad). */
    EU::TSharedPointer<Actor>              m_drakefirePistol;        /**< Arma secundaria del profe. */
    EU::TSharedPointer<Actor>              m_directionalLightActor;  /**< Actor que contiene la luz principal (NUEVO). */

    Model3D* m_model = nullptr;           /**< Puntero a la geometría de tu Xbox. */
    Model3D* m_drakefireModel = nullptr;  /**< Geometría extra del profe. */

    // ==========================================
    // ENTORNO, GUI Y ESTADOS DE PIPELINE
    // ==========================================
    GUI               m_gui;                 /**< Interfaz gráfica del editor (Dear ImGui). */
    bool              m_guiInitialized = false; /**< Bandera de estado para ImGui (NUEVA). */

    Skybox            m_skybox;              /**< Gestor del entorno del cielo. */
    Texture           m_skyboxTex;           /**< Recurso Cubemap. */

    RasterizerState   m_defaultRasterizer;   /**< Reglas de dibujo. */
    DepthStencilState m_defaultDepthStencil; /**< Reglas de profundidad. */
    SamplerState      m_defaultSampler;      /**< Muestreador de texturas (NUEVO). */

    // ==========================================
    // SISTEMA DE RENDERIZADO AVANZADO (NUEVOS)
    // ==========================================
    Mesh             m_cyberGunRenderMesh;   /**< Malla adaptada al nuevo renderer. */
    Mesh             m_drakefireRenderMesh;

    Material         m_pbrMaterial;            /**< Material base PBR Opaco. */
    Material         m_transparentPbrMaterial; /**< Material base PBR Transparente. */

    MaterialInstance m_cyberGunMaterial;     /**< Material instanciado para TU modelo. */
    MaterialInstance m_drakefireMaterial;

    ForwardRenderer  m_forwardRenderer;      /**< Motor de renderizado en cascada (Forward). */
    RenderScene      m_renderScene;          /**< Contenedor lógico de la escena a renderizar. */

    // ==========================================
    // SISTEMA DE RENDER-TO-TEXTURE (EDITOR)
    // ==========================================
    EditorViewportPass m_editorViewportPass;
    bool               m_editorViewportResizePending = false;
    unsigned int       m_pendingViewportWidth = 1;
    unsigned int       m_pendingViewportHeight = 1;
    unsigned int       m_lastRequestedViewportWidth = 1;
    unsigned int       m_lastRequestedViewportHeight = 1;
    int                m_viewportResizeStableFrames = 0;
};