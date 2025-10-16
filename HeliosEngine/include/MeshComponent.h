#pragma once
// ../include/
#include "../include/Prerequisites.h"
#include <vector>
#include <cstdint>
#include <type_traits>

/**
 * @class MeshComponent
 * @brief Contenedor agnóstico de vértices/índices con stride configurable.
 *
 * Permite cargar cualquier tipo de vértice (tu struct) sin acoplarse al engine:
 * usa plantillas para guardar datos crudos y recordar el stride.
 * Soporta índices de 16 o 32 bits (DXGI_FORMAT_R16_UINT / R32_UINT).
 */
class MeshComponent {
public:
    MeshComponent() = default;
    ~MeshComponent() = default;

    MeshComponent(const MeshComponent&) = default;
    MeshComponent& operator=(const MeshComponent&) = default;

    // ----------------------------
    // Carga de datos
    // ----------------------------

    /// Guarda un array de vértices de tipo arbitrario (p. ej. SimpleVertex)
    template<typename TVertex>
    void setVertices(const std::vector<TVertex>& verts)
    {
        static_assert(!std::is_pointer<TVertex>::value, "TVertex must be a POD struct, not a pointer.");
        m_vertexStride = (UINT)sizeof(TVertex);
        m_vertices.resize(m_vertexStride * verts.size());
        if (!verts.empty()) {
            std::memcpy(m_vertices.data(), verts.data(), m_vertices.size());
        }
    }

    /// Guarda índices de 16 o 32 bits; el formato DXGI se deduce automáticamente.
    template<typename TIndex>
    void setIndices(const std::vector<TIndex>& idx)
    {
        static_assert(std::is_same<TIndex, uint16_t>::value || std::is_same<TIndex, uint32_t>::value,
            "Index type must be uint16_t or uint32_t");
        const UINT elemSize = (UINT)sizeof(TIndex);
        m_indexFormat = (elemSize == 2) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
        m_indices.resize(elemSize * idx.size());
        if (!idx.empty()) {
            std::memcpy(m_indices.data(), idx.data(), m_indices.size());
        }
    }

    // ----------------------------
    // Accesores
    // ----------------------------
    const void* vertexData() const { return m_vertices.empty() ? nullptr : m_vertices.data(); }
    const void* indexData()  const { return m_indices.empty() ? nullptr : m_indices.data(); }

    UINT vertexStride() const { return m_vertexStride; }
    UINT indexStride()  const {
        return (m_indexFormat == DXGI_FORMAT_R16_UINT) ? 2u :
            (m_indexFormat == DXGI_FORMAT_R32_UINT) ? 4u : 0u;
    }

    UINT vertexCount() const { return (m_vertexStride ? (UINT)(m_vertices.size() / m_vertexStride) : 0u); }
    UINT indexCount()  const { UINT is = indexStride(); return (is ? (UINT)(m_indices.size() / is) : 0u); }

    DXGI_FORMAT indexFormat() const { return m_indexFormat; }

    bool hasVertices() const { return vertexCount() > 0; }
    bool hasIndices()  const { return indexCount() > 0; }

    void clear()
    {
        m_vertices.clear();
        m_indices.clear();
        m_vertexStride = 0;
        m_indexFormat = DXGI_FORMAT_UNKNOWN;
    }

private:
    std::vector<uint8_t> m_vertices;
    std::vector<uint8_t> m_indices;
    UINT         m_vertexStride = 0;
    DXGI_FORMAT  m_indexFormat = DXGI_FORMAT_UNKNOWN;
};
