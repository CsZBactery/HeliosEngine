#pragma once
#include "Prerequisites.h"

class DeviceContext;


class
    MeshComponent {
public:

    MeshComponent() : m_numVertex(0), m_numIndex(0) {}

    virtual
        ~MeshComponent() = default;

    void
        init();

    void
        update(float deltaTime);

    void
        render(DeviceContext& deviceContext);


    void
        destroy();

public:

    std::string m_name;

    std::vector<SimpleVertex> m_vertex;

    std::vector<unsigned int> m_index;

    int m_numVertex;

    int m_numIndex;
};
