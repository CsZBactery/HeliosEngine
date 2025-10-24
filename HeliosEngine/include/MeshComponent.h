#pragma once
#include "../include/Prerequisites.h"
#include "../include/Types.h"
#include <vector>
#include <string>
#include <cstdint>
#include <type_traits>
#include <cstring>

/**
 * MeshComponent compatible con el código del profe (m_vertex/m_index públicos)
 * + una capa de compatibilidad con setVertices/setIndices y accesores.
 */
class MeshComponent {
public:
    MeshComponent() : m_numVertex(0), m_numIndex(0) {}
    ~MeshComponent() = default;

    // ---- Compat layer (opcional) ------------------------------------------
    template<typename TVertex>
    void setVertices(const std::vector<TVertex>& verts) {
        m_vertex.resize(verts.size());
        if (!verts.empty()) {
            // copia uno a uno para evitar aliasing en PODs
            std::memcpy(m_vertex.data(), verts.data(), sizeof(TVertex) * verts.size());
        }
        m_numVertex = static_cast<int>(m_vertex.size());
    }

   // template<typename TIndex>
   // void setIndices(const std::vector<TIndex>& idx) {
        //static_assert(std::is_same<TIndex, uint16_t>::value ||
            //std::is_same<TIndex, uint32_t>::value,
            //"Index type must be uint16_t or uint32_t");
       // m_index.resize(idx.size());
       //if (!idx.empty()) {
            //for (size_t i = 0; i < idx.size(); ++i) m_index[i] = static_cast<unsigned int>(idx[i]);
       // }
       // m_numIndex = static_cast<int>(m_index.size());
   // }

    // Accesores compatibles con el código que espera API “genérica”
    UINT vertexStride() const { return sizeof(SimpleVertex); }
    UINT indexStride()  const { return sizeof(unsigned int); }
    UINT vertexCount()  const { return static_cast<UINT>(m_vertex.size()); }
    UINT indexCount()   const { return static_cast<UINT>(m_index.size()); }
    const void* vertexData() const { return m_vertex.empty() ? nullptr : m_vertex.data(); }
    const void* indexData()  const { return m_index.empty() ? nullptr : m_index.data(); }
    DXGI_FORMAT indexFormat() const { return DXGI_FORMAT_R32_UINT; }

    void clear() {
        m_vertex.clear(); m_index.clear();
        m_numVertex = 0;   m_numIndex = 0;
    }
    // -----------------------------------------------------------------------

public:
    std::string                 m_name;
    std::vector<SimpleVertex>   m_vertex;
    std::vector<unsigned int>   m_index;
    int                         m_numVertex;
    int                         m_numIndex;
};
