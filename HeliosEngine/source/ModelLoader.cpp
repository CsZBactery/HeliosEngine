#include "../include/ModelLoader.h"
#include "../include/Prerequisites.h"

// OBJ_Loader es header-only
// Asegúrate de tenerlo incluido en tu proyecto
// y de que el path de include sea correcto.
#include "../include/OBJ_Loader.h"

bool ModelLoader::LoadOBJ(const std::string& objPath, MeshComponent& outMesh, bool flipV)
{
    try
    {
        objl::Loader loader;
        bool ok = loader.LoadFile(objPath);
        if (!ok || loader.LoadedMeshes.empty())
        {
            ERROR(L"ModelLoader", L"LoadOBJ", L"Failed to load OBJ file");
            return false;
        }

        // --- Seleccionar la mesh más grande (por número de índices) ---
        size_t bestIdx = 0;
        size_t bestIndexCount = 0;
        for (size_t i = 0; i < loader.LoadedMeshes.size(); ++i)
        {
            const objl::Mesh& m = loader.LoadedMeshes[i];
            if (m.Indices.size() > bestIndexCount)
            {
                bestIndexCount = m.Indices.size();
                bestIdx = i;
            }
        }

        const objl::Mesh& big = loader.LoadedMeshes[bestIdx];

        // Reservas
        outMesh.m_vertex.clear();
        outMesh.m_index.clear();
        outMesh.m_vertex.reserve(big.Vertices.size());
        outMesh.m_index.reserve(big.Indices.size());

        // Vértices
        for (size_t v = 0; v < big.Vertices.size(); ++v)
        {
            const objl::Vertex& ov = big.Vertices[v];

            SimpleVertex sv;
            sv.Pos = XMFLOAT3(ov.Position.X, ov.Position.Y, ov.Position.Z);

            float u = ov.TextureCoordinate.X;
            float vcoord = ov.TextureCoordinate.Y;
            if (flipV) vcoord = 1.0f - vcoord;

            // Si no hay UVs válidas, quedan en 0
            if (!std::isfinite(u)) u = 0.0f;
            if (!std::isfinite(vcoord)) vcoord = 0.0f;

            sv.Tex = XMFLOAT2(u, vcoord);

            outMesh.m_vertex.push_back(sv);
        }

        // Índices (tal cual vienen)
        for (size_t i = 0; i < big.Indices.size(); ++i)
        {
            outMesh.m_index.push_back(static_cast<unsigned int>(big.Indices[i]));
        }

        outMesh.m_numVertex = static_cast<int>(outMesh.m_vertex.size());
        outMesh.m_numIndex = static_cast<int>(outMesh.m_index.size());

        MESSAGE(L"ModelLoader", L"LoadOBJ", L"OBJ loaded successfully (largest mesh picked)");
        return true;
    }
    catch (...)
    {
        ERROR(L"ModelLoader", L"LoadOBJ", L"Exception while loading OBJ");
        return false;
    }
}
