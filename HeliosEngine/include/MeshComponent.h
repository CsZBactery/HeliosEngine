#pragma once
#include "Prerequisites.h"

/**
 * @file MeshComponent.h
 * @brief Declaración de @c MeshComponent: contenedor ligero de datos de malla (vértices e índices)
 *        con puntos de extensión para ciclo de vida y render.
 */

class DeviceContext;

/**
 * @class MeshComponent
 * @brief Representa una malla 3D básica: nombre, arreglo de vértices e índices, y contadores.
 *
 * Esta clase almacena los datos de geometría (vértices e índices) y ofrece métodos de ciclo de vida
 * (init/update/render/destroy) para integrarse con el motor. El dibujo real suele realizarse a través
 * de buffers (vertex/index buffers) y el @c DeviceContext.
 */
class
    MeshComponent {
public:
    /**
     * @brief Constructor por defecto. Inicializa los contadores a 0.
     */
    MeshComponent() : m_numVertex(0), m_numIndex(0) {}

    /**
     * @brief Destructor virtual por defecto.
     */
    virtual
        ~MeshComponent() = default;

    /**
     * @brief Punto de inicialización de la malla (reservas, generación de datos, etc.).
     *
     * No modifica la firma ni los datos; úsese para preparar el componente antes del render.
     */
    void
        init();

    /**
     * @brief Actualiza la lógica asociada a la malla (animaciones, morphing, etc.).
     * @param deltaTime Tiempo transcurrido desde el último frame (en segundos).
     */
    void
        update(float deltaTime);

    /**
     * @brief Solicita el render de la malla utilizando el contexto de dispositivo proporcionado.
     * @param deviceContext Contexto de dispositivo a través del cual se emiten los comandos de dibujo.
     *
     * Nota: típicamente esta función asume que los buffers y shaders ya están configurados externamente.
     */
    void
        render(DeviceContext& deviceContext);

    /**
     * @brief Libera o limpia recursos asociados a la malla si aplica.
     */
    void
        destroy();

public:
    /** @brief Nombre simbólico/descriptivo de la malla. */
    std::string m_name;

    /** @brief Arreglo de vértices (posición, UV, normal, etc.) de la malla. */
    std::vector<SimpleVertex> m_vertex;

    /** @brief Arreglo de índices (triángulos u otra topología) de la malla. */
    std::vector<unsigned int> m_index;

    /** @brief Cantidad de vértices válidos en @c m_vertex. */
    int m_numVertex;

    /** @brief Cantidad de índices válidos en @c m_index. */
    int m_numIndex;
};
