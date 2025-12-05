#pragma once
#include "Prerequisites.h"
#include "Component.h"

class DeviceContext;

/**
 * @class Entity
 * @brief Clase base para cualquier objeto de la escena (Patrón Entidad-Componente).
 *
 * Una Entidad actúa como un contenedor genérico. Por sí sola no hace nada.
 * Su comportamiento se define mediante los componentes que se le agregan.
 *
 * Ejemplo:
 * - Entity "Jugador" = Componente Transform + Componente Malla + Componente Cámara.
 */
class Entity {
public:
    /**
     * @brief Constructor por defecto.
     */
    Entity() = default;

    /**
     * @brief Destructor virtual.
     */
    virtual
        ~Entity() = default;

    /**
     * @brief Inicializa la entidad.
     *
     * Método virtual puro. Las clases hijas (como Actor) deben implementar cómo
     * inicializan sus recursos internos.
     */
    virtual void
        init() = 0;

    /**
     * @brief Actualiza la lógica de la entidad y sus componentes.
     *
     * @param deltaTime Tiempo transcurrido en segundos desde el último frame.
     * @param deviceContext Contexto del dispositivo (necesario si algún componente actualiza buffers).
     */
    virtual void
        update(float deltaTime, DeviceContext& deviceContext) = 0;

    /**
     * @brief Ejecuta el renderizado de la entidad.
     *
     * Normalmente itera sobre sus componentes visuales y llama a sus métodos render.
     *
     * @param deviceContext Contexto del dispositivo para enviar comandos de dibujo.
     */
    virtual void
        render(DeviceContext& deviceContext) = 0;

    /**
     * @brief Libera los recursos de la entidad.
     */
    virtual void
        destroy() = 0;

    /**
     * @brief Agrega un nuevo componente a la lista de la entidad.
     *
     * Utiliza plantillas (templates) para asegurar que solo se agreguen clases derivadas de Component.
     *
     * @tparam T Tipo del componente (debe heredar de Component).
     * @param component Puntero inteligente (SharedPointer) al componente a agregar.
     */
    template <typename T> void
        addComponent(EU::TSharedPointer<T> component) {
        // Verifica en tiempo de compilación que T hereda de Component para evitar errores.
        static_assert(std::is_base_of<Component, T>::value, "T must be derived from Component");
        m_components.push_back(component.template dynamic_pointer_cast<Component>());
    }

    /**
     * @brief Busca y recupera un componente específico de la entidad.
     *
     * Realiza una búsqueda lineal y un cast dinámico para encontrar el componente solicitado.
     *
     * @tparam T Tipo de componente que buscamos (ej: Transform, MeshComponent).
     * @return Puntero al componente si existe, o un puntero nulo (nullptr) si no lo tiene.
     */
    template<typename T>
    EU::TSharedPointer<T>
        getComponent() {
        for (auto& component : m_components) {
            EU::TSharedPointer<T> specificComponent = component.template dynamic_pointer_cast<T>();
            if (specificComponent) {
                return specificComponent;
            }
        }
        return EU::TSharedPointer<T>();
    }

protected:
    /**
     * @brief Indica si la entidad está activa en la escena (se actualiza/renderiza).
     */
    bool m_isActive;

    /**
     * @brief Identificador único de la entidad.
     */
    int m_id;

    /**
     * @brief Lista de punteros compartidos a los componentes que posee esta entidad.
     */
    std::vector<EU::TSharedPointer<Component>> m_components;
};