#include "SceneGraph/SceneGraph.h"
#include "SceneGraph/HierarchyComponent.h" // Asegúrate de tener este archivo creado
#include "ECS/Entity.h"
#include "ECS/Transform.h"
#include "DeviceContext.h"
#include <algorithm>

void SceneGraph::init() {
    m_entities.clear();
}

void SceneGraph::destroy() {
    for (Entity* e : m_entities) {
        if (!e) continue;
        auto h = e->getComponent<HierarchyComponent>();
        if (h) {
            h->m_parent = nullptr;
            h->m_children.clear();
        }
    }
    m_entities.clear();
}

void SceneGraph::addEntity(Entity* e) {
    if (!e) return;
    if (isRegistered(e)) return;

    // Validar que existen los componentes mínimos
    // Si no tienen Transform, se lo agregamos
    if (!e->getComponent<Transform>()) {
        e->addComponent(EU::MakeShared<Transform>());
        e->getComponent<Transform>()->init();
    }

    // Si no tienen HierarchyComponent (necesario para el grafo), se lo agregamos
    if (!e->getComponent<HierarchyComponent>()) {
        e->addComponent(EU::MakeShared<HierarchyComponent>());
        e->getComponent<HierarchyComponent>()->init();
    }

    m_entities.push_back(e);
}

void SceneGraph::removeEntity(Entity* e) {
    if (!e) return;
    if (!isRegistered(e)) return;

    // 1) Detach de su padre (si tiene)
    detach(e);

    // 2) Reparent de hijos a null (se vuelven roots)
    auto h = e->getComponent<HierarchyComponent>();
    if (h) {
        // Copia local para no invalidar iteradores mientras modificamos
        auto childrenCopy = h->m_children;
        for (Entity* c : childrenCopy) {
            if (!c) continue;

            // Detach del padre (que es e)
            auto hc = c->getComponent<HierarchyComponent>();
            if (hc && hc->m_parent == e)
                hc->m_parent = nullptr;

            // Quitar referencia en e
            h->removeChild(c);
        }
        h->m_children.clear();
    }

    // 3) Eliminar del registro del grafo
    m_entities.erase(std::remove(m_entities.begin(), m_entities.end(), e), m_entities.end());
}

bool SceneGraph::isAncestor(Entity* possibleAncestor, Entity* node) const {
    // Recorre hacia arriba desde node: si encuentra possibleAncestor, hay ciclo
    if (!possibleAncestor || !node) return false;

    auto h = node->getComponent<HierarchyComponent>();
    // Usamos un puntero temporal para recorrer
    Entity* currentNode = node;

    // Mientras tengamos componente de jerarquía y un padre válido
    while (h && h->m_parent) {
        if (h->m_parent == possibleAncestor) return true;

        currentNode = h->m_parent;

        if (currentNode) {
            h = currentNode->getComponent<HierarchyComponent>();
        }
        else {
            // CORRECCIÓN AQUÍ:
            // "h = nullptr;" causaba el error si TSharedPointer no soporta asignación con nullptr.
            // Asignamos un puntero vacío usando el constructor por defecto.
            h = EU::TSharedPointer<HierarchyComponent>();
        }
    }
    return false;
}

bool SceneGraph::isRoot(Entity* e) const {
    if (!e) return false;
    auto h = e->getComponent<HierarchyComponent>();
    // Es root si no tiene componente de jerarquía o si su padre es null
    return (!h || h->m_parent == nullptr);
}

bool SceneGraph::isRegistered(Entity* e) const {
    return std::find(m_entities.begin(), m_entities.end(), e) != m_entities.end();
}

bool SceneGraph::attach(Entity* child, Entity* parent) {
    if (!child || !parent) return false;
    if (child == parent) return false;

    // Registro automático si no estaban en el grafo
    addEntity(child);
    addEntity(parent);

    // Evita ciclos: parent no puede estar debajo de child
    if (isAncestor(child, parent)) return false;

    // Si child ya tiene padre, detach primero
    detach(child);

    auto hc = child->getComponent<HierarchyComponent>();
    auto hp = parent->getComponent<HierarchyComponent>();

    if (!hc || !hp) return false;

    hc->m_parent = parent;
    hp->addChild(child);

    return true;
}

bool SceneGraph::detach(Entity* child) {
    if (!child) return false;

    auto hc = child->getComponent<HierarchyComponent>();
    if (!hc) return false;

    Entity* parent = hc->m_parent;
    if (!parent) return true; // ya era root

    auto hp = parent->getComponent<HierarchyComponent>();
    if (hp) hp->removeChild(child);

    hc->m_parent = nullptr;
    return true;
}

void SceneGraph::update(float deltaTime, DeviceContext& deviceContext) {
    // 1) Actualiza la lógica local de todas las entidades
    for (Entity* e : m_entities) {
        if (!e) continue;
        e->update(deltaTime, deviceContext);
    }

    // 2) Propagación World: procesa desde los roots hacia abajo
    for (Entity* e : m_entities) {
        if (!e) continue;
        if (isRoot(e)) {
            // Un root tiene WorldMatrix = LocalMatrix (multiplicado por Identidad)
            updateWorldRecursive(e, XMMatrixIdentity());
        }
    }
}

void SceneGraph::updateWorldRecursive(Entity* node, const XMMATRIX& parentWorld) {
    if (!node) return;

    auto t = node->getComponent<Transform>();
    auto h = node->getComponent<HierarchyComponent>();

    if (!t) return;

    // CALCULO DE MATRIZ GLOBAL
    // World = Local * ParentWorld
    XMMATRIX worldMatrix = t->matrix * parentWorld;

    // ACTUALIZAMOS LA MATRIZ PARA EL RENDER
    t->matrix = worldMatrix;

    if (h) {
        for (Entity* c : h->m_children) {
            updateWorldRecursive(c, worldMatrix);
        }
    }
}

void SceneGraph::render(DeviceContext& deviceContext) {
    // Renderiza todas las entidades
    for (auto& e : m_entities) {
        if (e) {
            e->render(deviceContext);
        }
    }
}