#pragma once
#include "Prerequisites.h"
#include "IResource.h"
#include "MeshComponent.h"

#include <string>
#include <vector>
#include <map>
#include <tuple>

#ifdef USE_FBX_SDK
#include <fbxsdk.h>
#endif

//--------------------------------------------------------------------------------------
// Tipo de modelo soportado
//--------------------------------------------------------------------------------------
enum class ModelType {
    OBJ,
    FBX
};

//--------------------------------------------------------------------------------------
// Helper: parser OBJ manual (tu versión, integrada como utilitario)
//--------------------------------------------------------------------------------------
class OBJParser
{
public:
    // flipV=true para coord. V en estilo D3D
    bool LoadOBJ(const std::string& objPath, MeshComponent& outMesh, bool flipV = true);

private:
    struct VertexIndices {
        int v = 0;
        int vt = 0;
        int vn = 0;

        bool operator<(const VertexIndices& o) const {
            if (v != o.v)  return v < o.v;
            if (vt != o.vt) return vt < o.vt;
            return vn < o.vn;
        }
    };
};

//--------------------------------------------------------------------------------------
// Recurso de modelo 3D (OBJ/FBX) integrado al sistema de recursos
//--------------------------------------------------------------------------------------
class Model3D final : public IResource
{
public:
    explicit Model3D(const std::string& name, ModelType modelType)
        : IResource(name)
        , m_modelType(modelType)
    {
        m_type = ResourceType::Model3D;
        m_state = ResourceState::Unloaded;
    }

    ~Model3D() override = default;

    // Carga desde disco (no crea GPU). Guarda path y parsea a m_meshes.
    bool load(const std::string& path) override;

    // Crea recursos GPU si lo deseas (opcional, según tu pipeline)
    bool init() override;

    // Libera CPU/GPU según tu flujo
    void unload() override;

    // Para profiler
    size_t getSizeInBytes() const override;

    // Accesores
    const std::vector<MeshComponent>& GetMeshes() const { return m_meshes; }
    ModelType                        GetModelType() const { return m_modelType; }
    const std::vector<std::string>& GetTextureFiles() const { return m_textureFileNames; }

private:
    // ---------- Implementaciones por formato ----------
    bool loadOBJ_Internal(const std::string& path);

#ifdef USE_FBX_SDK
    bool loadFBX_Internal(const std::string& path);

    // Utilidades FBX (solo si se compila con SDK)
    static bool TriangulateScene(FbxScene* scene);
    void        ExtractMeshesFromFbx(FbxScene* scene);
    void        ExtractSingleMesh(FbxMesh* fbxMesh, const FbxAMatrix& xform);

    // Conversión básica FBX -> XMFLOAT
    static XMFLOAT3 ToXM(const FbxVector4& v) {
        return XMFLOAT3((float)v[0], (float)v[1], (float)v[2]);
    }

    static XMFLOAT2 ToXM(const FbxVector2& v, bool flipV) {
        return XMFLOAT2(
            (float)v[0],
            flipV ? (1.0f - (float)v[1]) : (float)v[1]
        );
    }

    // Handles del SDK
    FbxManager* m_fbxManager = nullptr;
    FbxScene* m_fbxScene = nullptr;
#endif

private:
    ModelType m_modelType = ModelType::OBJ;

    // Salida CPU (lo que luego subes a GPU con tus Buffers)
    std::vector<MeshComponent> m_meshes;

    // Texturas detectadas (mtl / material FBX) – opcional
    std::vector<std::string> m_textureFileNames;
};
