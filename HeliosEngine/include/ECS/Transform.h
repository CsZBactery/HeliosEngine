#pragma once
#include "../include/Prerequisites.h"
#include "EngineUtilities/Vectors/Vector3.h"
#include "Component.h"

class
    Transform : public Component {
public:
    // Constructor que inicializa posici�n, rotaci�n y escala por defecto
    Transform() : position(),
        rotation(),
        scale(),
        matrix(),
        Component(ComponentType::TRANSFORM) {
    }

    // M�todos para inicializaci�n, actualizaci�n, renderizado y destrucci�n
    // Inicializa el objeto Transform
    void
        init() {
        scale.one();
        matrix = XMMatrixIdentity();
    }

    // Actualiza el estado del objeto Transform basado en el tiempo transcurrido
    // @param deltaTime: Tiempo transcurrido desde la �ltima actualizaci�n
    void
        update(float deltaTime) override {
        // Aplicar escala
        XMMATRIX scaleMatrix = XMMatrixScaling(scale.x, scale.y, scale.z);
        // Aplicar rotacion
        XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);
        // Aplicar traslacion
        XMMATRIX translationMatrix = XMMatrixTranslation(position.x, position.y, position.z);

        // Componer la matriz final en el orden: scale -> rotation -> translation
        matrix = scaleMatrix * rotationMatrix * translationMatrix;
    }

    // Renderiza el objeto Transform
    // @param deviceContext: Contexto del dispositivo de renderizado
    void
        render(DeviceContext& deviceContext) override {}

    // Destruye el objeto Transform y libera recursos
    void
        destroy() {}

    // M�todos de acceso a los datos de posici�n
    // Retorna la posici�n actual
    const EU::Vector3&
        getPosition() const { return position; }

    // Establece una nueva posici�n
    void
        setPosition(const EU::Vector3& newPos) { position = newPos; }

    // M�todos de acceso a los datos de rotaci�n
    // Retorna la rotaci�n actual
    const EU::Vector3&
        getRotation() const { return rotation; }

    // Establece una nueva rotaci�n
    void
        setRotation(const EU::Vector3& newRot) { rotation = newRot; }

    // M�todos de acceso a los datos de escala
    // Retorna la escala actual
    const EU::Vector3&
        getScale() const { return scale; }

    // Establece una nueva escala
    void
        setScale(const EU::Vector3& newScale) { scale = newScale; }

    void
        setTransform(const EU::Vector3& newPos,
            const EU::Vector3& newRot,
            const EU::Vector3& newSca) {
        position = newPos;
        rotation = newRot;
        scale = newSca;
    }

    // M�todo para trasladar la posici�n del objeto
    // @param translation: Vector que representa la cantidad de traslado en cada eje
    void
        translate(const EU::Vector3& translation);

private:
    EU::Vector3 position;  // Posici�n del objeto
    EU::Vector3 rotation;  // Rotaci�n del objeto
    EU::Vector3 scale;     // Escala del objeto

public:
    XMMATRIX matrix;    // Matriz de transformaci�n
};