/**
 * @file Camera.h
 * @brief Gestión de la vista y proyección 3D para HeliosEngine.
 */

#pragma once
#include "Prerequisites.h"
#include "EngineUtilities/Vectors/Vector3.h"

 /**
  * @class Camera
  * @brief Clase encargada de gestionar la lógica de una cámara en un entorno 3D.
  * * Administra las matrices de vista (View) y proyección (Projection), así como el movimiento
  * y orientación del espectador en el espacio de mundo. Utiliza un sistema de mano izquierda (LH).
  */
class
	Camera {
public:
	/** @brief Constructor. Inicializa la cámara en el origen con orientación por defecto. */
	Camera();

	/** @brief Destructor por defecto. */
	~Camera() = default;

	/**
	 * @brief Configura la proyección en perspectiva (LH).
	 * * @param fovYRadians Campo de visión vertical en radianes (ej: XM_PIDIV4).
	 * @param aspectRatio Relación de aspecto (ancho / alto).
	 * @param nearPlane Distancia al plano de recorte cercano.
	 * @param farPlane Distancia al plano de recorte lejano (horizonte).
	 */
	void
		setLens(float fovYRadians, float aspectRatio, float nearPlane, float farPlane);

	/** @brief Define posición en mundo mediante componentes individuales. */
	void setPosition(float x, float y, float z);

	/** @brief Define posición en mundo mediante un vector. */
	void setPosition(const EU::Vector3& pos);

	/** @brief Obtiene una copia de la posición actual. */
	EU::Vector3 getPosition() const { return m_position; }

	/** @brief Obtiene una referencia a la posición (permite modificación directa). */
	EU::Vector3& getPosition() { return m_position; }

	/**
	 * @brief Fuerza la cámara a mirar a un objetivo específico.
	 * * Reconstruye la base ortonormal a partir del vector dirección (target - pos).
	 * @param pos Posición de la cámara.
	 * @param target Punto al que mirar.
	 * @param up Vector de referencia "arriba" (normalmente 0,1,0).
	 */
	void lookAt(const EU::Vector3& pos,
		const EU::Vector3& target,
		const EU::Vector3& up = EU::Vector3(0, 1, 0));

	/** @brief Movimiento relativo: adelante (distancia positiva) o atrás (negativa). */
	void walk(float d);

	/** @brief Movimiento relativo: derecha (distancia positiva) o izquierda (negativa). */
	void strafe(float d);

	/** @brief Rotación sobre el eje Y global (Yaw/Giro). */
	void yaw(float radians);

	/** @brief Rotación sobre el eje Right local (Pitch/Inclinación). */
	void pitch(float radians);

	/**
	 * @brief Recalcula la matriz View si hubo cambios en posición o rotación.
	 * * Debe llamarse antes de cada frame de renderizado.
	 */
	void updateViewMatrix();

	/** @brief Obtiene la matriz de Vista (Transformación Mundo -> Vista). */
	XMMATRIX getView() const { return XMLoadFloat4x4(&m_view); }

	/** @brief Obtiene la matriz de Proyección (Transformación Vista -> Clip Space). */
	XMMATRIX getProj() const { return XMLoadFloat4x4(&m_proj); }

	/**
	 * @brief Obtiene la matriz View sin traslación.
	 * @details Útil para el Skybox, para que el fondo rote con nosotros pero nunca nos alejemos de él.
	 */
	XMMATRIX GetViewNoTranslation() const {
		XMMATRIX v = getView();
		v.r[3] = XMVectorSet(0, 0, 0, 1); // Anula la traslación en la 4ta fila
		return v;
	}

	// Getters de parámetros de lente
	float getFovY()   const { return m_fovY; }
	float getAspect() const { return m_aspectRatio; }
	float getNearZ()  const { return m_nearPlane; }
	float getFarZ()   const { return m_farPlane; }

	// Getters de base ortonormal
	EU::Vector3 GetRight()   const { return m_right; }
	EU::Vector3 GetUp()      const { return m_up; }
	EU::Vector3 GetForward() const { return m_forward; }

	/** @brief Helper para convertir tipos de DirectXMath a vectores del motor. */
	inline EU::Vector3 FromXM(FXMVECTOR v) {
		XMFLOAT3 t;
		XMStoreFloat3(&t, v);
		return EU::Vector3(t.x, t.y, t.z);
	}

private:
	// Estado espacial
	EU::Vector3 m_position;

	// Basis Ortonormal (Ejes locales de la cámara en el mundo)
	EU::Vector3 m_right{ 1.0f, 0.0f, 0.0f };
	EU::Vector3 m_up{ 0.0f, 1.0f, 0.0f };
	EU::Vector3 m_forward{ 0.0f, 0.0f, 1.0f };

	// Matrices para la GPU
	XMFLOAT4X4 m_view{};
	XMFLOAT4X4 m_proj{};

	// Parámetros de la lente
	float m_fovY{ XM_PIDIV4 };
	float m_aspectRatio = 1.0f;
	float m_nearPlane = 0.01f;
	float m_farPlane = 1000.0f;

	/** @brief Indica si la matriz View debe recalcularse por un cambio de movimiento. */
	bool m_viewDirty = true;
};