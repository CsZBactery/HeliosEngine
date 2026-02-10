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
     * @brief Inicialización temprana.
     * Se ejecuta antes de init() para configurar referencias o estados iniciales.
     */
    virtual void awake() = 0;

    /**
     * @brief Inicializa la entidad.
     * Método virtual puro. Las clases hijas deben implementar cómo inicializan sus recursos.
     */
    virtual void init() = 0;

    /**
     * @brief Actualiza la lógica de la entidad.
     * @param deltaTime Tiempo transcurrido desde la última actualización.
     * @param deviceContext Contexto del dispositivo (necesario si algún componente actualiza buffers).
     */
    virtual void update(float deltaTime, DeviceContext& deviceContext) = 0;

    /**
     * @brief Renderiza la entidad.
     * @param deviceContext Contexto del dispositivo para operaciones gráficas.
     */
    virtual void render(DeviceContext& deviceContext) = 0;

    /**
     * @brief Libera los recursos de la entidad.
     */
    virtual void destroy() = 0;

    /**
     * @brief Agrega un componente a la entidad.
     * @tparam T Tipo del componente (debe heredar de Component).
     * @param component Puntero compartido al componente a agregar.
     */
    template <typename T>
    void addComponent(EU::TSharedPointer<T> component) {
        static_assert(std::is_base_of<Component, T>::value, "T must be derived from Component");
        m_components.push_back(component.template dynamic_pointer_cast<Component>());
    }

    /**
     * @brief Obtiene un componente específico de la entidad.
     * @tparam T Tipo de componente a obtener.
     * @return Puntero compartido al componente si existe, nullptr si no.
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
    bool m_isActive;
    int m_id;
    std::vector<EU::TSharedPointer<Component>> m_components;
};