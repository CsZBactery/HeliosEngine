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

/**
 * @file BaseApp.h
 * @brief Punto de orquestación principal de la app: crea ventana, inicializa D3D11 y ejecuta el bucle principal.
 *
 * Esta clase encapsula el ciclo de vida de la aplicación (init/update/render/destroy),
 * y mantiene los wrappers de recursos Direct3D 11 necesarios para dibujar.
 */

 /**
  * @class BaseApp
  * @brief Aplicación base: administra ventana, dispositivo, pipeline y ciclo principal.
  *
  * Responsabilidades:
  * - Crear la ventana y el swap chain.
  * - Crear los recursos de render (RTV/DSV/Viewport).
  * - Compilar/enlazar shaders, buffers y sampler.
  * - Ejecutar el bucle principal con @ref update y @ref render.
  */
class
	BaseApp {
public:

	/**
	 * @brief Construye la aplicación base y recibe parámetros de arranque de Win32.
	 * @param hInst Instancia de la aplicación.
	 * @param nCmdShow Modo de visualización de la ventana (ShowCmd).
	 */
	BaseApp(HINSTANCE hInst, int nCmdShow);

	/**
	 * @brief Destructor: libera recursos mediante @ref destroy().
	 */
	~BaseApp() { destroy(); }

	/**
	 * @brief Punto de entrada a nivel clase. Crea ventana, llama a @ref init() y ejecuta el loop.
	 * @param hInst Instancia de la aplicación.
	 * @param nCmdShow Modo de visualización de la ventana.
	 * @return Código de salida de la aplicación (wParam de WM_QUIT).
	 */
	int
		run(HINSTANCE hInst, int nCmdShow);

	/**
	 * @brief Inicializa todos los recursos necesarios de D3D11 y de la escena.
	 * @return S_OK en éxito, o HRESULT de error.
	 */
	HRESULT
		init();

	/**
	 * @brief Actualiza el estado de la escena y/o lógica de juego.
	 * @param deltaTime Tiempo transcurrido (en segundos) desde el frame anterior.
	 */
	void
		update(float deltaTime);

	/**
	 * @brief Emite las órdenes de dibujo para el frame actual.
	 */
	void
		render();

	/**
	 * @brief Libera todos los recursos creados por la aplicación.
	 */
	void
		destroy();

private:

	/**
	 * @brief Procedimiento de ventana Win32.
	 * @param hWnd Handle de la ventana.
	 * @param message Mensaje recibido.
	 * @param wParam Parámetro W.
	 * @param lParam Parámetro L.
	 * @return LRESULT esperado por Win32.
	 */
	static LRESULT CALLBACK
		WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	/** @name Wrappers de plataforma y dispositivo */
	///@{
	Window                              m_window;            /**< Ventana nativa de la aplicación. */
	Device                              m_device;            /**< ID3D11Device wrapper. */
	DeviceContext                       m_deviceContext;     /**< ID3D11DeviceContext wrapper. */
	SwapChain                           m_swapChain;         /**< IDXGISwapChain wrapper. */
	///@}

	/** @name Render targets y profundidad */
	///@{
	Texture                             m_backBuffer;        /**< Textura del back-buffer. */
	RenderTargetView                    m_renderTargetView;  /**< RTV del back-buffer. */
	Texture                             m_depthStencil;      /**< Textura de profundidad/stencil. */
	DepthStencilView                    m_depthStencilView;  /**< DSV para la textura de profundidad. */
	Viewport                            m_viewport;          /**< Viewport activo. */
	///@}

	/** @name Pipeline programable */
	///@{
	ShaderProgram                       m_shaderProgram;     /**< Programa de shaders (VS/PS + input layout). */
	///@}

	/** @name Geometría y buffers */
	///@{
	MeshComponent												m_mesh;               /**< Malla en CPU (vértices/índices). */
	Buffer															m_vertexBuffer;       /**< Vertex buffer (GPU). */
	Buffer															m_indexBuffer;        /**< Index buffer (GPU). */
	Buffer															m_cbNeverChanges;     /**< Constant buffer b0 (view). */
	Buffer															m_cbChangeOnResize;   /**< Constant buffer b1 (proj). */
	Buffer															m_cbChangesEveryFrame;/**< Constant buffer b2 (world/color). */
	Texture 														m_textureCube;        /**< Textura difusa del cubo (si aplica). */
	SamplerState                        m_samplerState;       /**< Sampler para PS. */
	///@}

	/** @name Estado de cámara/transformaciones */
	///@{
	XMMATRIX                            m_World;             /**< Matriz World actual. */
	XMMATRIX                            m_View;              /**< Matriz View actual. */
	XMMATRIX                            m_Projection;        /**< Matriz Projection actual. */
	XMFLOAT4                            m_vMeshColor;        /**< Color del mesh (RGBA). */
	///@}

	/** @name Datos subidos a constant buffers */
	///@{
	CBChangeOnResize cbChangesOnResize;  /**< Payload CPU para b1 (proyección). */
	CBNeverChanges   cbNeverChanges;     /**< Payload CPU para b0 (vista). */
	CBChangesEveryFrame cb;              /**< Payload CPU para b2 (world/color). */
	///@}
};
