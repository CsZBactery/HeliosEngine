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

/**
 * @class BaseApp
 * @brief Clase principal de la aplicación HeliosEngine.
 *
 * Esta clase administra el ciclo de vida de la aplicación, incluyendo la inicialización
 * de los recursos de DirectX 11, el bucle principal de mensajes (Game Loop),
 * la lógica de actualización (Update) y el renderizado (Render).
 */
class BaseApp {
public:
    /**
     * @brief Constructor de la aplicación.
     * @param hInst Manejador (Handle) de la instancia actual de la aplicación.
     * @param nCmdShow Indica cómo se debe mostrar la ventana (minimizado, maximizado, etc.).
     */
    BaseApp(HINSTANCE hInst, int nCmdShow);

    /**
     * @brief Destructor. Llama internamente a destroy() para liberar recursos.
     */
    ~BaseApp() { destroy(); }

    /**
     * @brief Ejecuta el bucle principal de la aplicación.
     *
     * Inicializa la ventana y los recursos, y comienza el bucle de mensajes de Windows.
     * Calcula el DeltaTime para las actualizaciones.
     *
     * @param hInst Manejador de la instancia.
     * @param nCmdShow Comandos de visualización de la ventana.
     * @return int Código de salida del programa (wParam del mensaje WM_QUIT).
     */
    int run(HINSTANCE hInst, int nCmdShow);

    /**
     * @brief Inicializa los recursos de DirectX y de la escena.
     *
     * Crea el Device, SwapChain, Vistas (RTV, DSV), Shaders, Buffers,
     * y carga los modelos y texturas iniciales (como la moto Repsol).
     *
     * @return HRESULT S_OK si todo se inicializó correctamente, o un código de error si falló.
     */
    HRESULT init();

    /**
     * @brief Actualiza la lógica de la escena.
     *
     * Se llama una vez por frame antes de renderizar. Aquí se actualizan las físicas,
     * la UI, las transformaciones de los actores y las cámaras.
     *
     * @param deltaTime Tiempo transcurrido en segundos desde el último frame.
     */
    void update(float deltaTime);

    /**
     * @brief Renderiza la escena actual.
     *
     * Limpia los buffers, configura el pipeline de dibujo, dibuja los actores
     * y presenta el back buffer en pantalla.
     */
    void render();

    /**
     * @brief Libera todos los recursos y memoria asignada.
     *
     * Se debe llamar al cerrar la aplicación para evitar fugas de memoria (memory leaks).
     */
    void destroy();

private:
    /**
     * @brief Procedimiento de ventana (Callback estático).
     *
     * Maneja los eventos de Windows (teclado, ratón, cierre, redimensionado) y los pasa a ImGui.
     */
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
    // ------------------------------------------------------------------------
    // RECURSOS PRINCIPALES DEL MOTOR (CORE DX11)
    // ------------------------------------------------------------------------
    Window              m_window;           ///< Encapsula la ventana de Win32.
    Device              m_device;           ///< Interfaz para crear recursos en la GPU.
    DeviceContext       m_deviceContext;    ///< Interfaz para emitir comandos de renderizado.
    SwapChain           m_swapChain;        ///< Cadena de intercambio (Front/Back buffers).
    Texture             m_backBuffer;       ///< Textura que representa el buffer trasero.
    RenderTargetView    m_renderTargetView; ///< Vista para dibujar en el back buffer.
    Texture             m_depthStencil;     ///< Textura para el buffer de profundidad.
    DepthStencilView    m_depthStencilView; ///< Vista para el test de profundidad/stencil.
    Viewport            m_viewport;         ///< Define el área de dibujo en la ventana.
    ShaderProgram       m_shaderProgram;    ///< Gestiona los Vertex y Pixel Shaders activos.

    // ------------------------------------------------------------------------
    // BUFFERS CONSTANTES (Shader Communication)
    // ------------------------------------------------------------------------
    Buffer              m_cbNeverChanges;      ///< Datos que nunca cambian durante la ejecución.
    Buffer              m_cbChangeOnResize;    ///< Datos que cambian al redimensionar (Proyección).
    Buffer              m_cbChangesEveryFrame; ///< Datos que cambian cada frame (si fuera necesario).

    // ------------------------------------------------------------------------
    // RECURSOS ESPECÍFICOS DE LA ESCENA (ASSETS)
    // ------------------------------------------------------------------------

    /// @brief Textura principal para el modelo de la moto.
    /// @note Anteriormente llamada m_cyberGunAlbedo.
    Texture             m_repsolTexture;

    SamplerState        m_samplerState;     ///< Estado de muestreo para las texturas (filtros, addressing).

    // ------------------------------------------------------------------------
    // MATRICES Y DATOS GLOBALES
    // ------------------------------------------------------------------------
    XMMATRIX            m_World;        ///< Matriz de Mundo (Transformación local -> global).
    XMMATRIX            m_View;         ///< Matriz de Vista (Cámara).
    XMMATRIX            m_Projection;   ///< Matriz de Proyección (Perspectiva).
    XMFLOAT4            m_vMeshColor;   ///< Color base global (si no se usa textura).

    // ------------------------------------------------------------------------
    // SISTEMA DE ENTIDADES Y ACTORES
    // ------------------------------------------------------------------------

    /// Lista de todos los actores en la escena para actualización/renderizado masivo.
    std::vector<EU::TSharedPointer<Actor>> m_actors;

    /// @brief Puntero inteligente al actor principal (la moto Repsol).
    /// @note Anteriormente m_cyberGun.
    EU::TSharedPointer<Actor>              m_repsolActor;

    Model3D* m_model;   ///< Puntero al recurso del modelo 3D cargado (OBJ/FBX).

    // ------------------------------------------------------------------------
    // ESTRUCTURAS DE DATOS PARA CONSTANT BUFFERS
    // ------------------------------------------------------------------------
    CBChangeOnResize    cbChangesOnResize;  ///< Estructura de datos CPU para el buffer de redimensionado.
    CBNeverChanges      cbNeverChanges;     ///< Estructura de datos CPU para datos estáticos.
    CBChangesEveryFrame cb;                 ///< Estructura de datos CPU para datos por frame.

    // ------------------------------------------------------------------------
    // INTERFAZ DE USUARIO (GUI)
    // ------------------------------------------------------------------------
    UserInterface       UI;                 ///< Sistema de gestión de ImGui.
};