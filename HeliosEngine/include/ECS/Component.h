#pragma once
#include "Prerequisites.h"

class DeviceContext;

/**
 * @class Component
 * @brief Clase base abstracta para todos los componentes del juego.
 *
 * En un sistema de Entidad-Componente (o similar), el Componente representa
 * una pieza modular de lógica o datos (ej: una Malla, una Transformación, un Script).
 *
 * Esta clase define la "interfaz común" (Polimorfismo): todos los componentes
 * deben saber inicializarse, actualizarse, dibujarse y destruirse, aunque cada
 * uno lo haga de forma diferente.
 */
class Component {
public:
    /**
     * @brief Constructor por defecto.
     */
    Component() = default;

    /**
     * @brief Constructor que asigna el tipo de componente.
     * @param type Identificador del tipo (ej: TRANSFORM, MESH, CAMERA).
     */
    Component(const ComponentType type) : m_type(type) {}

    /**
     * @brief Destructor virtual.
     * Es crucial que sea virtual para que se llame al destructor correcto de la clase hija.
     */
    virtual
        ~Component() = default;

    /**
     * @brief Inicialización lógica del componente.
     * Se llama una sola vez cuando el componente se agrega o inicia.
     */
    virtual void
        init() = 0;

    /**
     * @brief Método virtual puro para actualizar la lógica (Física, IA, Movimiento).
     *
     * @param deltaTime Tiempo en segundos transcurrido desde el último frame.
     */
    virtual void
        update(float deltaTime) = 0;

    /**
     * @brief Método virtual puro para operaciones de dibujo.
     *
     * @note Algunos componentes lógicos (como Scripts de IA) pueden dejar este método vacío,
     * mientras que componentes visuales (Mesh) lo usarán para llamar a DrawIndexed.
     *
     * @param deviceContext Contexto del dispositivo para enviar comandos a la GPU.
     */
    virtual void
        render(DeviceContext& deviceContext) = 0;

    /**
     * @brief Método virtual puro para liberar memoria y recursos.
     */
    virtual void
        destroy() = 0;

    /**
     * @brief Obtiene el identificador del tipo de componente.
     * @return Enum ComponentType con el tipo.
     */
    ComponentType
        getType() const { return m_type; }

protected:
    /**
     * @brief Almacena el tipo de componente para identificaciones rápidas (RTTI simple).
     */
    ComponentType m_type;
};