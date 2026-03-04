/**
 * @file Skybox.h
 * @brief Define la clase encargada de renderizar el entorno de fondo (Skybox) usando un mapa de cubos.
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
 * * @details Utiliza una textura tipo Cubemap (6 imágenes) aplicada al interior de un cubo.
 * Modifica temporalmente los estados de Rasterización y Profundidad para asegurarse de que
 * siempre se dibuje al fondo de la escena sin ser recortado ni ocultar a los modelos 3D frontales.
 */
class
	Skybox {
public:
	/**
	 * @brief Constructor por defecto.
	 */
	Skybox() = default;

	/**
	 * @brief Destructor por defecto.
	 */
	~Skybox() = default;

	/**
	 * @brief Inicializa los recursos del Skybox (Geometría, Shaders, Estados y Textura).
	 * * @param device Referencia al dispositivo de hardware de DirectX.
	 * @param deviceContext Puntero al contexto del dispositivo.
	 * @param cubemap Textura tipo Cubemap previamente cargada en memoria.
	 * @return S_OK si la inicialización fue exitosa, o un código de error HRESULT si falló.
	 */
	HRESULT
		init(Device& device, DeviceContext* deviceContext, Texture& cubemap);

	/**
	 * @brief Actualiza lógica interna del Skybox si es necesario (animaciones de nubes, etc).
	 */
	void
		update() {}

	/**
	 * @brief Ejecuta las órdenes de dibujo del cielo en la GPU.
	 * * @details Aplica una matriz especial que sigue la rotación de la cámara pero ignora su
	 * traslación (posición), creando la ilusión de un entorno infinito al que nunca te puedes acercar.
	 * * @param deviceContext Contexto del dispositivo para emitir comandos de dibujo.
	 * @param camera Referencia a la cámara activa de la escena.
	 */
	void
		render(DeviceContext& deviceContext, Camera& camera);

	/**
	 * @brief Libera la memoria de la tarjeta gráfica y la RAM ocupada por el entorno.
	 * @note La implementación de esta función reside en el archivo .cpp correspondiente.
	 */
	void
		destroy();

private:
	ShaderProgram             m_shaderProgram;      /**< Shader especializado para leer coordenadas 3D de un cubemap. */
	Buffer                    m_constantBuffer;     /**< Buffer para enviar la matriz de vista-proyección a la GPU. */
	SamplerState              m_samplerState;       /**< Reglas de filtrado y muestreo para la textura del cielo. */

	RasterizerState           m_rasterizerState;    /**< Estado para renderizar el interior de los polígonos (Cull Front). */
	DepthStencilState         m_depthStencilState;  /**< Estado para forzar que el cielo se pinte detrás de todo (Z-Buffer). */

	Texture                   m_skyboxTexture;      /**< El recurso de textura que contiene las 6 caras del entorno. */
	Model3D* m_cubeModel = nullptr;/**< Puntero a la geometría de cubo generada paramétricamente. */
	EU::TSharedPointer<Actor> m_skybox;             /**< Entidad ECS que encapsula la malla del Skybox. */
};