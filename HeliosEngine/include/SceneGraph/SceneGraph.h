#pragma once
#include "Prerequisites.h"
#include <vector>

class Entity;
class DeviceContext;

/**
 * @class SceneGraph
 * @brief Gestiona la jerarquía de entidades y la propagación de transformaciones.
 */
class SceneGraph {
public:
    SceneGraph() = default;
    ~SceneGraph() = default;

    void init();

    /**
     * @brief Registra una entidad en el grafo.
     * @param e Puntero raw a la entidad.
     */
    void addEntity(Entity* e);

    void removeEntity(Entity* e);

    bool isAncestor(Entity* possibleAncestor, Entity* node) const;

    bool attach(Entity* child, Entity* parent);

    bool detach(Entity* child);

    void update(float deltaTime, DeviceContext& deviceContext);

    void render(DeviceContext& deviceContext);

    void destroy();

private:
    /**
     * @brief Recorre recursivamente multiplicando la matriz local por la del padre.
     */
    void updateWorldRecursive(Entity* node, const XMMATRIX& parentWorld);

    bool isRoot(Entity* e) const;

    bool isRegistered(Entity* e) const;

public:
    // Usamos punteros raw porque la propiedad (ownership) la tiene BaseApp::m_actors (SharedPtr)
    std::vector<Entity*> m_entities;
};