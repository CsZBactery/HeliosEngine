/**
 * @file EditorViewportPass.h
 * @brief Gestiona el renderizado de la escena hacia una textura para su visualización en el editor.
 */

#pragma once
#include "Prerequisites.h"
#include "Texture.h"
#include "RenderTargetView.h"
#include "DepthStencilView.h"

class Device;
class DeviceContext;

/**
 * @class EditorViewportPass
 * @brief Implementa la técnica de "Render to Texture" para el Viewport del Editor.
 * * Esta clase crea un Framebuffer privado (Color + Profundidad). Al renderizar la escena
 * aquí, obtenemos un Shader Resource View (SRV) que podemos pasarle a ImGui para
 * mostrar el juego dentro de una ventana flotante.
 */
class
	EditorViewportPass {
public:
	/** @brief Constructor por defecto. */
	EditorViewportPass() = default;
	/** @brief Destructor. */
	~EditorViewportPass() = default;

	/**
	 * @brief Inicializa los buffers de color y profundidad con el tamaño especificado.
	 * @param device Dispositivo DirectX para crear los recursos.
	 * @param width Ancho inicial de la textura.
	 * @param height Alto inicial de la textura.
	 * @return S_OK si la creación fue exitosa.
	 */
	HRESULT init(Device& device, unsigned int width, unsigned int height);

	/**
	 * @brief Libera los buffers actuales y los recrea con nuevas dimensiones.
	 * @details Se llama cuando el usuario escala la ventana del Viewport en la UI.
	 */
	HRESULT resize(Device& device, unsigned int width, unsigned int height);

	/**
	 * @brief Prepara el pipeline para dibujar en esta textura en lugar de la pantalla.
	 * @param deviceContext Contexto del dispositivo.
	 * @param clearColor Color de fondo (RGBA).
	 */
	void begin(DeviceContext& deviceContext, const float clearColor[4]);

	/**
	 * @brief Intercambia los recursos con otro pase (útil para efectos de post-procesado).
	 */
	void swap(EditorViewportPass& other);

	/** @brief Limpia el buffer de profundidad para un nuevo frame. */
	void clearDepth(DeviceContext& deviceContext);

	/** @brief Ajusta el Viewport de DirectX para que coincida con el tamaño de esta textura. */
	void setViewport(DeviceContext& deviceContext);

	/** @brief Libera todos los recursos COM de la GPU. */
	void destroy();

	/**
	 * @brief Retorna el recurso de textura para ser usado en la UI (ImGui).
	 * @return Puntero al ID3D11ShaderResourceView de la textura de color.
	 */
	ID3D11ShaderResourceView* getSRV() const { return m_colorSRV.m_textureFromImg; }

	/** @brief Retorna el ancho actual. */
	unsigned int getWidth() const { return m_width; }
	/** @brief Retorna el alto actual. */
	unsigned int getHeight() const { return m_height; }

	/** @brief Verifica si todos los recursos están creados correctamente. */
	bool isValid() const
	{
		return m_colorTexture.m_texture != nullptr &&
			m_colorSRV.m_textureFromImg != nullptr &&
			m_depthTexture.m_texture != nullptr;
	}

private:
	/** @brief Lógica interna compartida para crear las texturas y vistas. */
	HRESULT createResources(Device& device, unsigned int width, unsigned int height);

private:
	Texture           m_colorTexture; /**< Recurso de textura donde se guarda el color. */
	Texture           m_colorSRV;     /**< Vista de recurso para leer la textura desde la UI. */
	RenderTargetView  m_rtv;          /**< Vista de salida para que la GPU escriba en la textura. */

	Texture           m_depthTexture; /**< Recurso de textura para la profundidad. */
	DepthStencilView  m_dsv;          /**< Vista para pruebas de profundidad privadas del viewport. */

	unsigned int      m_width = 1;
	unsigned int      m_height = 1;
};