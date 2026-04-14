#include "SceneGraph/SceneGraph.h"
#include "SceneGraph/HierarchyComponent.h"
#include "ECS/Entity.h"
#include "ECS/Transform.h"
#include "DeviceContext.h"
#include <algorithm>

// NUEVOS INCLUDES PARA EL RENDERER
#include "ECS/LightComponent.h"
#include "ECS/MeshRendererComponent.h"
#include "EngineUtilities/Utilities/Camera.h"
#include "Rendering/Material.h"
#include "Rendering/MaterialInstance.h"
#include "Rendering/RenderScene.h"

// Prepara el grafo vaciando la lista de entidades registradas
void SceneGraph::init() {
    m_entities.clear();
}

// Limpia el grafo y rompe los vínculos de jerarquía para evitar referencias colgadas
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

// Registra una entidad en el motor y se asegura de que tenga los componentes básicos
void SceneGraph::addEntity(Entity* e) {
    if (!e) return;
    if (isRegistered(e)) return;

    // Un objeto en el mundo siempre necesita una posición (Transform)
    if (!e->getComponent<Transform>()) {
        e->addComponent(EU::MakeShared<Transform>());
        e->getComponent<Transform>()->init();
    }

    // Para que el objeto pueda ser padre o hijo, necesita este componente
    if (!e->getComponent<HierarchyComponent>()) {
        e->addComponent(EU::MakeShared<HierarchyComponent>());
        e->getComponent<HierarchyComponent>()->init();
    }

    m_entities.push_back(e);
}

// Elimina una entidad del grafo y reorganiza a sus hijos
void SceneGraph::removeEntity(Entity* e) {
    if (!e) return;
    if (!isRegistered(e)) return;

    // 1) Lo desconectamos de su padre si tenía uno
    detach(e);

    // 2) Sus hijos no se borran, simplemente se quedan sin padre (se vuelven raíces)
    auto h = e->getComponent<HierarchyComponent>();
    if (h) {
        auto childrenCopy = h->m_children;
        for (Entity* c : childrenCopy) {
            if (!c) continue;

            auto hc = c->getComponent<HierarchyComponent>();
            if (hc && hc->m_parent == e)
                hc->m_parent = nullptr;

            h->removeChild(c);
        }
        h->m_children.clear();
    }

    // 3) Lo quitamos de la lista global de entidades del grafo
    m_entities.erase(std::remove(m_entities.begin(), m_entities.end(), e), m_entities.end());
}

// Verifica si un nodo es ancestro de otro (evita que un padre sea hijo de su propio hijo)
bool SceneGraph::isAncestor(Entity* possibleAncestor, Entity* node) const {
    if (!possibleAncestor || !node) return false;

    auto h = node->getComponent<HierarchyComponent>();
    Entity* currentNode = node;

    // Escalamos por la jerarquía hacia arriba buscando al ancestro
    while (h && h->m_parent) {
        if (h->m_parent == possibleAncestor) return true;

        currentNode = h->m_parent;

        if (currentNode) {
            h = currentNode->getComponent<HierarchyComponent>();
        }
        else {
            // Si el nodo actual no es válido, detenemos la búsqueda con un puntero vacío
            h = EU::TSharedPointer<HierarchyComponent>();
        }
    }
    return false;
}

// Determina si una entidad es una raíz (no tiene padre)
bool SceneGraph::isRoot(Entity* e) const {
    if (!e) return false;
    auto h = e->getComponent<HierarchyComponent>();
    return (!h || h->m_parent == nullptr);
}

// Comprueba si la entidad ya existe en la lista del grafo
bool SceneGraph::isRegistered(Entity* e) const {
    return std::find(m_entities.begin(), m_entities.end(), e) != m_entities.end();
}

// Crea un vínculo donde un objeto se vuelve hijo de otro
bool SceneGraph::attach(Entity* child, Entity* parent) {
    if (!child || !parent) return false;
    if (child == parent) return false;

    // Ambos deben estar registrados en el grafo
    addEntity(child);
    addEntity(parent);

    // Seguridad: Evitamos ciclos infinitos en la jerarquía
    if (isAncestor(child, parent)) return false;

    // Si el hijo ya tenía otro padre, lo soltamos primero
    detach(child);

    auto hc = child->getComponent<HierarchyComponent>();
    auto hp = parent->getComponent<HierarchyComponent>();

    if (!hc || !hp) return false;

    // Establecemos la conexión bidireccional
    hc->m_parent = parent;
    hp->addChild(child);

    return true;
}

// Rompe el vínculo entre un hijo y su padre
bool SceneGraph::detach(Entity* child) {
    if (!child) return false;

    auto hc = child->getComponent<HierarchyComponent>();
    if (!hc) return false;

    Entity* parent = hc->m_parent;
    if (!parent) return true; // Si no tenía padre, ya estaba suelto

    auto hp = parent->getComponent<HierarchyComponent>();
    if (hp) hp->removeChild(child);

    hc->m_parent = nullptr;
    return true;
}

// Actualiza todas las entidades y calcula sus posiciones reales en el mundo
void SceneGraph::update(float deltaTime, DeviceContext& deviceContext) {
    // 1) Primero actualizamos la lógica individual de cada objeto (scripts, física, etc.)
    for (Entity* e : m_entities) {
        if (!e) continue;
        e->update(deltaTime, deviceContext);
    }

    // 2) Propagación de Matrices: Calculamos la posición real sumando las de los padres
    for (Entity* e : m_entities) {
        if (!e) continue;
        // Solo empezamos desde los objetos raíz; ellos propagarán el movimiento a sus hijos
        if (isRoot(e)) {
            updateWorldRecursive(e, XMMatrixIdentity());
        }
    }
}

// Función recursiva que hereda la transformación del padre a los hijos
void SceneGraph::updateWorldRecursive(Entity* node, const XMMATRIX& parentWorld) {
    if (!node) return;

    auto t = node->getComponent<Transform>();
    auto h = node->getComponent<HierarchyComponent>();

    if (!t || !h) return;

    // CÁLCULO MATEMÁTICO:
    // La matriz World final es la matriz local (t->matrix) multiplicada por la del padre.
    // Esto hace que si mueves al padre, el hijo se mueva con él.
    XMMATRIX worldMatrix = t->matrix * parentWorld;

    // Guardamos el resultado en la variable worldMatrix (NUEVO DEL PROFE)
    t->worldMatrix = worldMatrix;

    // Si tiene hijos, repetimos el proceso para cada uno de ellos usando nuestra nueva matriz global
    for (Entity* c : h->m_children) {
        updateWorldRecursive(c, worldMatrix);
    }
}

// Dibuja todas las entidades (MÉTODO LEGACY, ahora se usa gatherRenderScene)
void SceneGraph::render(DeviceContext& deviceContext) {
    for (auto& e : m_entities) {
        if (e) {
            e->render(deviceContext);
        }
    }
}

// ======================================================================================
// NUEVO MÉTODO DEL PROFE: Recolectar datos para el Forward Renderer
// ======================================================================================
void SceneGraph::gatherRenderScene(RenderScene& outScene, const Camera& camera) {
    for (Entity* entity : m_entities) {
        if (!entity) continue;

        // 1. Buscar Luces
        auto lightComponent = entity->getComponent<LightComponent>();
        if (lightComponent) {
            outScene.directionalLights.push_back(lightComponent->getLightData());
        }

        // 2. Buscar Mallas Renderizables
        auto meshRenderer = entity->getComponent<MeshRendererComponent>();
        auto transform = entity->getComponent<Transform>();

        // Si no tiene malla, no tiene transform, o está invisible, lo ignoramos
        if (!meshRenderer || !transform || !meshRenderer->isVisible()) {
            continue;
        }

        // 3. Empaquetar el Objeto
        RenderObject renderObject{};
        renderObject.mesh = meshRenderer->getMesh();
        renderObject.materialInstance = meshRenderer->getMaterialInstance();
        renderObject.materialInstances = meshRenderer->getMaterialInstances();
        renderObject.world = transform->worldMatrix; // Usamos la matriz global calculada
        renderObject.castShadow = meshRenderer->canCastShadow();

        // 4. Calcular distancia a la cámara (útil para ordenar transparencias)
        EU::Vector3 cameraPos = camera.getPosition();
        XMFLOAT4X4 worldMatrix{};
        XMStoreFloat4x4(&worldMatrix, transform->worldMatrix);
        EU::Vector3 objectPos = EU::Vector3(worldMatrix._41, worldMatrix._42, worldMatrix._43);

        float dx = objectPos.x - cameraPos.x;
        float dy = objectPos.y - cameraPos.y;
        float dz = objectPos.z - cameraPos.z;
        renderObject.distanceToCamera = dx * dx + dy * dy + dz * dz;

        // 5. Clasificar si es un objeto Opaco o Transparente
        MaterialDomain domain = MaterialDomain::Opaque;
        if (renderObject.materialInstance && renderObject.materialInstance->getMaterial()) {
            domain = renderObject.materialInstance->getMaterial()->getDomain();
        }

        renderObject.transparent = (domain == MaterialDomain::Transparent);
        if (renderObject.transparent) {
            outScene.transparentObjects.push_back(renderObject);
        }
        else {
            outScene.opaqueObjects.push_back(renderObject);
        }
    }
}