#pragma once
#include "Prerequisites.h"
#include "EngineUtilities\Vectors\Vector3.h"

/**
 * @class Camera
 * @brief Clase encargada de gestionar la lógica de una cámara en un entorno 3D.
 * * Administra las matrices de vista (View) y proyección (Projection), así como el movimiento
 * y orientación del espectador en el espacio de mundo.
 */
class
	Camera {
public:
	/**
	 * @brief Constructor por defecto de la clase Camera.
	 */
	Camera();

	/**
	 * @brief Destructor por defecto.
	 */
	~Camera() = default;

	/**
	 * @brief Configura la proyeccin en perspectiva (LH).
	 *
	 * **Pasos**
	 * - Calcula la matriz de proyeccin con XMMatrixPerspectiveFovLH.
	 * - Guarda FOV, aspect, near y far para debug/inspeccin.
	 *
	 * **Aplicacin prctica**
	 * - Llamar al inicializar ventana y al cambiar resolucin.
	 * * @param fovYRadians Campo de visin vertical en radianes.
	 * @param aspectRatio Relacin de aspecto (ancho/alto).
	 * @param nearPlane Distancia al plano cercano de recorte.
	 * @param farPlane Distancia al plano lejano de recorte.
	 */
	void
		setLens(float fovYRadians, float aspectRatio, float nearPlane, float farPlane);

	/**
	 * @brief Define posicin en mundo mediante componentes individuales.
	 * @param x Coordenada en el eje X.
	 * @param y Coordenada en el eje Y.
	 * @param z Coordenada en el eje Z.
	 */
	void
		setPosition(float x, float y, float z);

	/**
	 * @brief Define posicin en mundo mediante un vector.
	 * @param pos Vector de posicin EU::Vector3.
	 */
	void
		setPosition(const EU::Vector3& pos);

	/**
	 * @brief Obtiene la posicin actual de la cmara en el mundo.
	 * @return EU::Vector3 con las coordenadas de la cmara.
	 */
	EU::Vector3
		getPosition() const { return m_position; }

	/**
	 * @brief Fuerza la cmara a mirar a un objetivo (LH).
	 *
	 * **Pasos**
	 * - Calcula basis a partir de (target - pos).
	 * - Normaliza Forward, Right y Up.
	 * - Marca dirty para recalcular View.
	 *
	 * **Aplicacin prctica**
	 * - Cinemticas simples o cmaras orbit.
	 * * @param pos Posicin actual de la cmara.
	 * @param target Punto al que la cmara debe apuntar.
	 * @param up Vector que define la direccin "arriba" (por defecto 0,1,0).
	 */
	void
		lookAt(const EU::Vector3& pos,
			const EU::Vector3& target,
			const EU::Vector3& up = EU::Vector3(0, 1, 0));

	/**
	 * @brief Movimiento relativo a la cmara (adelante/atrs).
	 * @param d Distancia a desplazar (positivo adelante, negativo atrs).
	 */
	void
		walk(float d);

	/**
	 * @brief Movimiento relativo a la cmara (izquierda/derecha).
	 * @param d Distancia a desplazar (positivo derecha, negativo izquierda).
	 */
	void
		strafe(float d);

	/**
	 * @brief Rotacin sobre el eje Y global (yaw).
	 *
	 * **Aplicacin prctica**
	 * - Mouse X para FPS.
	 * * @param radians Cantidad de rotacin en radianes.
	 */
	void
		yaw(float radians);

	/**
	 * @brief Rotacin sobre el eje Right local (pitch).
	 *
	 * **Aplicacin prctica**
	 * - Mouse Y para FPS.
	 * * @param radians Cantidad de rotacin en radianes.
	 */
	void
		pitch(float radians);

	/**
	 * @brief Recalcula la matriz View si es necesario.
	 *
	 * **Pasos**
	 * - Reconstruye basis ortonormal (Right/Up/Forward).
	 * - Calcula View con XMMatrixLookToLH.
	 *
	 * **Aplicacin prctica**
	 * - Llamar una vez por frame antes de render.
	 */
	void
		updateViewMatrix();

	/**
	 * @brief Obtiene la matriz de Vista (mundo -> vista).
	 * @return XMMATRIX con la transformacin de vista.
	 */
	XMMATRIX
		getView() const { return XMLoadFloat4x4(&m_view); }

	/**
	 * @brief Obtiene la matriz de Proyeccin (vista -> clip).
	 * @return XMMATRIX con la transformacin de proyeccin.
	 */
	XMMATRIX
		getProj() const { return XMLoadFloat4x4(&m_proj); }

	/**
	 * @brief View sin traslacin (solo rotacin). Ideal para Skybox.
	 *
	 * **Aplicacin prctica**
	 * - Skybox: ViewNoTranslation * Proj
	 * * @return XMMATRIX de vista con la traslacin puesta en cero.
	 */
	XMMATRIX
		GetViewNoTranslation() const {
		XMMATRIX v = getView();
		// Quitar traslacin (fila 4)
		v.r[3] = XMVectorSet(0, 0, 0, 1);
		return v;
	}

	/**
	 * @brief Devuelve el valor del FOV vertical.
	 * @return float en radianes.
	 */
	float getFovY()   const { return m_fovY; }

	/**
	 * @brief Devuelve la relacin de aspecto actual.
	 * @return float (ancho / alto).
	 */
	float getAspect() const { return m_aspectRatio; }

	/**
	 * @brief Devuelve la distancia al plano de recorte cercano.
	 * @return float distancia Near.
	 */
	float getNearZ()  const { return m_nearPlane; }

	/**
	 * @brief Devuelve la distancia al plano de recorte lejano.
	 * @return float distancia Far.
	 */
	float getFarZ()   const { return m_farPlane; }

	/**
	 * @brief Obtiene el vector base Right (derecha) en espacio de mundo.
	 * @return Vector3 unitario apuntando a la derecha de la cmara.
	 */
	EU::Vector3 GetRight()   const { return m_right; }

	/**
	 * @brief Obtiene el vector base Up (arriba) en espacio de mundo.
	 * @return Vector3 unitario apuntando arriba de la cmara.
	 */
	EU::Vector3 GetUp()      const { return m_up; }

	/**
	 * @brief Obtiene el vector base Forward (adelante) en espacio de mundo.
	 * @return Vector3 unitario apuntando hacia donde mira la cmara.
	 */
	EU::Vector3 GetForward() const { return m_forward; }

	/**
	 * @brief Helper para convertir un FXMVECTOR (DirectXMath) a EU::Vector3.
	 * @param v Vector de DirectXMath a convertir.
	 * @return EU::Vector3 equivalente.
	 */
	inline EU::Vector3
		FromXM(FXMVECTOR v) {
		XMFLOAT3 t;
		XMStoreFloat3(&t, v);
		return EU::Vector3(t.x, t.y, t.z);
	}


private:
	/// Posicin de la cmara en el espacio de mundo.
	EU::Vector3 m_position;

	/// Vector Right de la base ortonormal de la cmara.
	EU::Vector3 m_right{ 1.0f, 0.0f, 0.0f };

	/// Vector Up de la base ortonormal de la cmara.
	EU::Vector3 m_up{ 0.0f, 1.0f, 0.0f };

	/// Vector Forward de la base ortonormal de la cmara.
	EU::Vector3 m_forward{ 0.0f, 0.0f, 1.0f };

	/// Matriz de vista almacenada en formato 4x4.
	XMFLOAT4X4 m_view{};

	/// Matriz de proyeccin almacenada en formato 4x4.
	XMFLOAT4X4 m_proj{};

	/// ngulo del campo de visin (FOV) en radianes.
	float m_fovY{ XM_PIDIV4 };

	/// Relacin de aspecto de la vista de la cmara.
	float m_aspectRatio = 1.0f;

	/// Distancia al plano de recorte cercano.
	float m_nearPlane = 0.01f;

	/// Distancia al plano de recorte lejano.
	float m_farPlane = 1000.0f;

	/// Bandera que indica si la matriz de vista necesita ser recalculada.
	bool m_viewDirty = true;
};