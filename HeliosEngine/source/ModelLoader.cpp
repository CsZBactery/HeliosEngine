#include "../include/ModelLoader.h"
#include "../include/OBJ_Loader.h"   

// Carga un .obj con OBJ_Loader y vuelca datos a MeshComponent
bool ModelLoader::LoadOBJ(const std::string& objPath, MeshComponent& outMesh, bool flipV)
{
    objl::Loader loader;

    // Intenta cargar el archivo .obj
    if (!loader.LoadFile(objPath)) {
        ERROR(L"ModelLoader", L"LoadOBJ", L"Failed to load OBJ file");
        return false;
    }

    // Limpiamos cualquier contenido previo
    outMesh.m_vertex.clear();
    outMesh.m_index.clear();
    outMesh.m_numVertex = 0;
    outMesh.m_numIndex = 0;

    // Acumuladores (por si el .obj trae varios submeshes/grupos)
    unsigned baseVertex = 0;

    // Recorremos cada Mesh de OBJ_Loader
    for (const auto& m : loader.LoadedMeshes)
    {
        // Vértices
        for (const auto& v : m.Vertices)
        {
            SimpleVertex sv{};
            sv.Pos = XMFLOAT3(v.Position.X, v.Position.Y, v.Position.Z);

            float u = v.TextureCoordinate.X;
            float vv = v.TextureCoordinate.Y;
            if (flipV) vv = 1.0f - vv;

            sv.Tex = XMFLOAT2(u, vv);
            outMesh.m_vertex.push_back(sv);
        }

        // Índices (aplicando offset)
        for (unsigned idx : m.Indices)
        {
            outMesh.m_index.push_back(baseVertex + idx);
        }

        baseVertex += static_cast<unsigned>(m.Vertices.size());
    }

    outMesh.m_numVertex = static_cast<int>(outMesh.m_vertex.size());
    outMesh.m_numIndex = static_cast<int>(outMesh.m_index.size());

    if (outMesh.m_numVertex == 0 || outMesh.m_numIndex == 0) {
        ERROR(L"ModelLoader", L"LoadOBJ", L"OBJ produced empty mesh");
        return false;
    }

    MESSAGE(L"ModelLoader", L"LoadOBJ", L"OBJ loaded successfully");
    return true;
}
