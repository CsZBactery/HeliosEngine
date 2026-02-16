/**
 * @file HierarchyComponent.h
 * @brief Componente encargado de gestionar las relaciones jerárquicas entre entidades.
 */

#pragma once
#include "Prerequisites.h"
#include "ECS/Component.h"

class DeviceContext;
class Entity;

/**
 * @class HierarchyComponent
 * @brief Permite que una entidad forme parte de un Grafo de Escena (Scene Graph).
 * @details Este componente almacena referencias al padre y a los hijos de una entidad,
 * facilitando la propagación de transformaciones espaciales (como el movimiento heredado).
 */
class
	HierarchyComponent : public Component {
public:
	/**
	 * @brief Constructor por defecto. Inicializa el componente con el tipo HIERARCHY.
	 */
	HierarchyComponent() : Component(ComponentType::HIERARCHY) {}

	/**
	 * @brief Destructor por defecto.
	 */
	~HierarchyComponent() = default;

	/**
	 * @brief Inicializa el componente. Actualmente no realiza operaciones.
	 */
	void
		init() override {}

	/**
	 * @brief Actualización lógica por frame. Actualmente no realiza operaciones.
	 * @param deltaTime Tiempo transcurrido.
	 */
	void
		update(float) override {}

	/**
	 * @brief Fase de renderizado. Actualmente no realiza operaciones.
	 * @param deviceContext Contexto del dispositivo para comandos de render.
	 */
	void
		render(DeviceContext& deviceContext) override {}

	/**
	 * @brief Limpia las relaciones jerárquicas.
	 * @details Desvincula al padre y vacía la lista de hijos para evitar referencias inválidas.
	 */
	void
		destroy() override {
		m_children.clear();
		m_parent = nullptr;
	}

	// API SceneGraph

	/**
	 * @brief Establece una entidad como padre de esta instancia.
	 * @param parent Puntero a la entidad padre.
	 */
	void
		setParent(Entity* parent) {
		m_parent = parent;
	}

	/**
	 * @brief Comprueba si la entidad es una raíz (no tiene padre).
	 * @return true si m_parent es nullptr, false en caso contrario.
	 */
	bool
		isRoot() const {
		return m_parent == nullptr;
	}

	/**
	 * @brief Comprueba si la entidad tiene hijos vinculados.
	 * @return true si la lista de hijos no está vacía.
	 */
	bool
		hasChildren() const {
		return !m_children.empty();
	}

	/**
	 * @brief Vincula una entidad como hijo de esta instancia.
	 * @details Realiza validaciones para evitar punteros nulos y duplicados en la lista.
	 * @param child Puntero a la entidad que será el nuevo hijo.
	 */
	void
		addChild(Entity* child) {
		if (!child) {
			return;
		}

		// Evitamos agregar al mismo hijo dos veces
		if (std::find(m_children.begin(), m_children.end(), child) != m_children.end()) {
			return;
		}
		m_children.push_back(child);
	}

	/**
	 * @brief Desvincula una entidad de la lista de hijos.
	 * @param child Puntero al hijo que se desea remover.
	 */
	void
		removeChild(Entity* child) {
		if (!child) return;

		m_children.erase(
			std::remove(m_children.begin(), m_children.end(), child),
			m_children.end()
		);
	}

public:
	Entity* m_parent = nullptr;        /**< Puntero a la entidad padre en la jerarquía. */
	std::vector<Entity*> m_children;   /**< Contenedor de punteros a todas las entidades hijas. */
};