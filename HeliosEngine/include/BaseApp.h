/**
 * @file BaseApp.h
 * @brief Clase núcleo que gestiona el ciclo de vida de la aplicación HeliosEngine.
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

/**
 * @brief Declaración externa para el manejador de eventos de ImGui.
 * @details Permite que la interfaz de usuario procese mensajes de Windows (mouse, teclado).
 */
extern IMGUI_IMPL_API
LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

/**
 * @class BaseApp
 * @brief Clase principal que orquesta el sistema de renderizado, la lógica de escena y la interfaz de usuario.
 * @details Esta clase controla la inicialización profunda de DirectX 11, el bucle de mensajes de Windows
 * y la actualización constante de todos los componentes del motor.
 */
class
    BaseApp {
public:
    /**
     * @brief Constructor por defecto.
     */
    BaseApp() = default;

    /**
     * @brief Destructor. Llama automáticamente a destroy() para liberar memoria.
     */
    ~BaseApp() { destroy(); }

    /**
     * @brief Fase previa a la inicialización de recursos de hardware.
     * @details Se utiliza para inicializar sistemas lógicos como el Grafo de Escena.
     * @return Código HRESULT de la operación.
     */
    HRESULT
        awake();

    /**
     * @brief Inicia el bucle principal de la aplicación.
     * @param hInst Instancia del ejecutable proporcionada por Windows.
     * @param nCmdShow Estado de visualización inicial de la ventana.
     * @return Código de salida del programa para el sistema operativo.
     */
    int
        run(HINSTANCE hInst, int nCmdShow);

    /**
     * @brief Inicialización de recursos de DirectX 11 y carga de Assets iniciales.
     * @details Crea el dispositivo, los buffers constantes, carga el modelo 3D y configura la cámara.
     * @return Código HRESULT de la operación.
     */
    HRESULT
        init();

    /**
     * @brief Actualización lógica por frame.
     * @param deltaTime Tiempo transcurrido entre el frame anterior y el actual.
     */
    void
        update(float deltaTime);

    /**
     * @brief Renderizado visual de la escena.
     * @details Ejecuta los comandos de dibujo en la GPU, limpia el backbuffer y presenta la imagen final.
     */
    void
        render();

    /**
     * @brief Liberación de recursos y limpieza de memoria.
     * @details Destruye todos los objetos COM de DirectX y libera punteros de modelos y actores.
     */
    void
        destroy();

private:
    /**
     * @brief Procedimiento de ventana (Callback de Windows).
     * @details Gestiona eventos del sistema operativo (redimensionado, cierre, input).
     */
    static LRESULT CALLBACK
        WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
    Window                              m_window;           /**< Gestor de la ventana de sistema operativo. */
    Device                              m_device;           /**< Representa el dispositivo de hardware (GPU). */
    DeviceContext                       m_deviceContext;    /**< Contexto para emitir comandos de renderizado. */
    SwapChain                           m_swapChain;        /**< Cadena de intercambio de buffers para visualización. */
    Texture                             m_backBuffer;       /**< Textura que actúa como buffer de color principal. */
    RenderTargetView                    m_renderTargetView; /**< Vista del recurso para escribir píxeles en el backbuffer. */
    Texture                             m_depthStencil;     /**< Textura de profundidad para el Z-Buffer. */
    DepthStencilView                    m_depthStencilView; /**< Vista del recurso para pruebas de profundidad. */
    Viewport                            m_viewport;         /**< Configuración del área de dibujado en pantalla. */
    ShaderProgram                       m_shaderProgram;    /**< Encapsula los Shaders (Vertex y Pixel). */

    Buffer                              m_cbNeverChanges;   /**< Buffer constante para datos estáticos (Luz, Vista). */
    Buffer                              m_cbChangeOnResize; /**< Buffer constante para datos de proyección. */

    // Assets
    Texture                             m_repsolTexture;    /**< Textura cargada para el modelo de la Moto. */
    Texture                             m_skyboxTex;        /**< Textura del tipo Cubemap para el fondo del cielo. */

    // Cámara y Matrices
    Camera                              m_camera;           /**< Objeto cámara que gestiona la orientación y perspectiva. */

    // Escena y Actores
    SceneGraph                          m_sceneGraph;       /**< Gestor de la jerarquía de objetos en el mundo. */
    std::vector<EU::TSharedPointer<Actor>> m_actors;        /**< Contenedor de actores presentes en la escena. */
    EU::TSharedPointer<Actor>              m_repsolActor;   /**< Actor específico que representa la Moto. */

    Model3D* m_model;                                       /**< Puntero al recurso de malla 3D cargada. */

    // Constant Buffers CPU
    CBChangeOnResize                    cbChangesOnResize;  /**< Estructura CPU para datos de redimensión. */
    CBNeverChanges                      cbNeverChanges;     /**< Estructura CPU para datos de iluminación y vista. */

    // GUI
    GUI                                 m_gui;              /**< Gestor de la interfaz de usuario (ImGui). */
};