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
    virtual ~Entity() = default;

    /**
     * @brief Inicialización temprana de la entidad.
     *
     * Método virtual puro. Se ejecuta antes de init() para configurar referencias iniciales.
     */
    virtual void awake() = 0;

    /**
     * @brief Inicializa la entidad.
     *
     * Método virtual puro. Las clases hijas deben implementar cómo inicializan sus recursos.
     */
    virtual void init() = 0;

    /**
     * @brief Método virtual puro para actualizar la entidad.
     *
     * @param deltaTime Tiempo transcurrido desde la última actualización.
     * @param deviceContext Contexto del dispositivo (necesario si algún componente actualiza buffers).
     */
    virtual void update(float deltaTime, DeviceContext& deviceContext) = 0;

    /**
     * @brief Método virtual puro para renderizar la entidad.
     *
     * @param deviceContext Contexto del dispositivo para operaciones gráficas.
     */
    virtual void render(DeviceContext& deviceContext) = 0;

    /**
     * @brief Método virtual puro para destruir la entidad.
     * Libera los recursos asociados.
     */
    virtual void destroy() = 0;

    /**
     * @brief Agrega un nuevo componente a la lista de la entidad.
     *
     * Utiliza plantillas (templates) para asegurar que solo se agreguen clases derivadas de Component.
     *
     * @tparam T Tipo del componente (debe heredar de Component).
     * @param component Puntero inteligente (SharedPointer) al componente a agregar.
     */
    template <typename T>
    void addComponent(EU::TSharedPointer<T> component) {
        // Verifica en tiempo de compilación que T hereda de Component.
        static_assert(std::is_base_of<Component, T>::value, "T must be derived from Component");
        m_components.push_back(component.template dynamic_pointer_cast<Component>());
    }

    /**
     * @brief Busca y recupera un componente específico de la entidad.
     *
     * @tparam T Tipo de componente que buscamos.
     * @return Puntero al componente si existe, o nullptr si no lo tiene.
     */
    template<typename T>
    EU::TSharedPointer<T> getComponent() {
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
     * @brief Indica si la entidad está activa en la escena.
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