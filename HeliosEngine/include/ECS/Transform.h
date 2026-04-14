#pragma once
#include "Prerequisites.h"
#include "EngineUtilities/Vectors/Vector3.h"
#include "Component.h"

/**
 * @class Transform
 * @brief Componente que define la posición, rotación y escala de una entidad.
 *
 * El Transform es el componente más importante de la escena.
 * Calcula la **Matriz de Mundo (World Matrix)**, que es la responsable de llevar
 * los vértices del modelo (espacio local) a su posición final en el mundo 3D.
 */
class Transform : public Component {
public:
    /**
     * @brief Constructor por defecto.
     * Inicializa la identidad (Pos: 0,0,0 | Rot: 0,0,0 | Escala: 0,0,0 -> Ojo, se corrige en init).
     */
    Transform() :
        position(),
        rotation(),
        scale(),
        matrix(),
        worldMatrix(), // Añadido por el profe para soportar jerarquías
        Component(ComponentType::TRANSFORM) {
    }

    /**
     * @brief Inicializa los valores por defecto.
     *
     * Establece la escala en (1, 1, 1) y resetea las matrices a Identidad.
     */
    void init() override {
        scale.one(); // Asume que tu clase Vector3 tiene este método
        matrix = XMMatrixIdentity();
        worldMatrix = XMMatrixIdentity(); // Añadido por el profe
    }

    /**
     * @brief Calcula la matriz de transformación final.
     *
     * Aplica las transformaciones en el orden estándar SRT (Scale -> Rotate -> Translate).
     *
     * @param deltaTime Tiempo transcurrido (no se usa para el cálculo directo, pero requerido por herencia).
     */
    void update(float deltaTime) override {
        // 1. Matriz de Escala
        XMMATRIX scaleMatrix = XMMatrixScaling(scale.x, scale.y, scale.z);
        // 2. Matriz de Rotación (Euler: Pitch, Yaw, Roll)
        XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);
        // 3. Matriz de Traslación
        XMMATRIX translationMatrix = XMMatrixTranslation(position.x, position.y, position.z);

        // COMPOSICIÓN DE LA MATRIZ:
        // El orden de multiplicación es CRÍTICO. En DirectX (Row-Major logic) es:
        // Final = Escala * Rotación * Traslación.
        matrix = scaleMatrix * rotationMatrix * translationMatrix;

        // El profe iguala la matriz global a la local por ahora. 
        // El SceneGraph se encargará de multiplicar esto por la matriz del "padre" más adelante.
        worldMatrix = matrix;
    }

    /**
     * @brief Método de renderizado.
     *
     * @note El Transform no dibuja píxeles por sí mismo, solo calcula matemáticas.
     * Por lo tanto, este método está vacío.
     *
     * @param deviceContext Contexto gráfico.
     */
    void render(DeviceContext& deviceContext) override {}

    /**
     * @brief Libera recursos.
     */
    void destroy() override {}

    // ------------------------------------------------------------------------
    // GETTERS & SETTERS (POSICIÓN)
    // ------------------------------------------------------------------------

    /** @brief Obtiene la posición global actual. */
    const EU::Vector3& getPosition() const { return position; }

    /** @brief Asigna una nueva posición absoluta. */
    void setPosition(const EU::Vector3& newPos) { position = newPos; }

    // ------------------------------------------------------------------------
    // GETTERS & SETTERS (ROTACIÓN)
    // ------------------------------------------------------------------------

    /** @brief Obtiene la rotación actual (en radianes). */
    const EU::Vector3& getRotation() const { return rotation; }

    /** @brief Asigna una nueva rotación absoluta (en radianes). */
    void setRotation(const EU::Vector3& newRot) { rotation = newRot; }

    // ------------------------------------------------------------------------
    // GETTERS & SETTERS (ESCALA)
    // ------------------------------------------------------------------------

    /** @brief Obtiene la escala actual. */
    const EU::Vector3& getScale() const { return scale; }

    /** @brief Asigna una nueva escala absoluta. */
    void setScale(const EU::Vector3& newScale) { scale = newScale; }

    /**
     * @brief Establece los tres valores de transformación de una vez.
     *
     * @param newPos Nueva posición.
     * @param newRot Nueva rotación (radianes).
     * @param newSca Nueva escala.
     */
    void setTransform(const EU::Vector3& newPos,
        const EU::Vector3& newRot,
        const EU::Vector3& newSca) {
        position = newPos;
        rotation = newRot;
        scale = newSca;
    }

    /**
     * @brief Mueve el objeto relativo a su posición actual.
     *
     * @param translation Vector delta a sumar a la posición actual.
     */
    void translate(const EU::Vector3& translation);

private:
    EU::Vector3 position;  ///< Coordenadas X, Y, Z en el espacio.
    EU::Vector3 rotation;  ///< Ángulos de Euler (Pitch, Yaw, Roll).
    EU::Vector3 scale;     ///< Factor de escala local.

public:
    /**
     * @brief Matriz de transformación local (relativa al padre).
     */
    XMMATRIX matrix;

    /**
     * @brief Matriz de transformación global (relativa al mundo).
     * Esta es la que se envía al Vertex Shader para dibujar el objeto.
     */
    XMMATRIX worldMatrix;
};