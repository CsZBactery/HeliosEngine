/**
 * @file Skybox.h
 * @brief Gestión del entorno de fondo (Skybox) mediante mapas de cubos en HeliosEngine.
 */

#pragma once
#include "Prerequisites.h"
#include "ShaderProgram.h"
#include "Texture.h"
#include "Buffer.h"
#include "SamplerState.h"
#include "Model3D.h"
#include "RasterizerState.h"
#include "DepthStencilState.h"
#include "EngineUtilities/Utilities/Camera.h"
#include "ECS/Actor.h"

class Device;
class DeviceContext;

/**
 * @class Skybox
 * @brief Gestiona la geometría, texturas y estados necesarios para dibujar un cielo infinito.
 * @details El Skybox utiliza una textura tipo Cubemap aplicada al interior de un cubo.
 * Modifica temporalmente los estados de Rasterización (para ver el interior) y
 * de Profundidad (para dibujarse detrás de todos los objetos) creando la ilusión
 * de un horizonte inalcanzable.
 */
class
	Skybox {
public:
	/** @brief Constructor por defecto. */
	Skybox() = default;
	/** @brief Destructor. */
	~Skybox() = default;

	/**
	 * @brief Inicializa los recursos (Geometría, Shaders, Estados y Textura).
	 * @param device Dispositivo de hardware de DirectX.
	 * @param deviceContext Contexto del dispositivo.
	 * @param cubemap Textura tipo Cubemap previamente cargada.
	 * @return S_OK si la inicialización fue exitosa.
	 */
	HRESULT
		init(Device& device, DeviceContext* deviceContext, Texture& cubemap);

	/**
	 * @brief Actualiza la posición del Skybox para que siempre rodee a la cámara.
	 * @param deviceContext Contexto del dispositivo.
	 * @param camera Cámara activa de la cual se extrae la rotación.
	 */
	void
		update(DeviceContext& deviceContext, Camera& camera);

	/**
	 * @brief Ejecuta las órdenes de dibujo del cielo en la GPU.
	 * @param deviceContext Contexto del dispositivo para emitir comandos de dibujo.
	 */
	void
		render(DeviceContext& deviceContext);

	/**
	 * @brief Libera la memoria de la GPU y recursos asociados.
	 */
	void
		destroy(); // <--- ¡CORREGIDO! Solo punto y coma.

private:
	ShaderProgram     m_shaderProgram;    /**< Shader que proyecta coordenadas 3D sobre el cubemap. */
	Buffer            m_constantBuffer;   /**< Buffer para la matriz de transformación del cielo. */
	SamplerState      m_samplerState;     /**< Reglas de filtrado para evitar costuras en el cielo. */
	RasterizerState   m_rasterizerState;  /**< Estado configurado con Front-Face Culling para ver el interior del cubo. */
	DepthStencilState m_depthStencilState;/**< Estado que permite al cielo pasar la prueba de profundidad estando al fondo. */

	Texture           m_skyboxTexture;    /**< Recurso que contiene las 6 caras del entorno. */
	Model3D* m_cubeModel = nullptr; /**< Geometría de cubo simplificada para el entorno. */
	EU::TSharedPointer<Actor> m_skybox;   /**< Entidad ECS que representa el objeto Skybox en la escena. */
};