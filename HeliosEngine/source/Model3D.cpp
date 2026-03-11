// ======================================================================================
// Archivo: Model3D.cpp
// Implementación del gestor de recursos para modelos tridimensionales (OBJ y FBX).
// ======================================================================================

#include "Model3D.h"

// Inicia el proceso estándar de carga desde el sistema de archivos
bool Model3D::load(const std::string& path) {
    SetPath(path);
    SetState(ResourceState::Loading);

    // Intentamos procesar el archivo y extraer sus mallas
    init();

    // Si el vector de mallas no está vacío, la carga fue un éxito
    bool success = !m_meshes.empty();
    SetState(success ? ResourceState::Loaded : ResourceState::Failed);
    return success;
}

// Inicia la conversión del archivo 3D a datos que el motor puede usar
bool Model3D::init() {
    m_meshes.clear();
    LoadFBXModel(m_filePath);
    return !m_meshes.empty();
}

// Libera toda la memoria ocupada por las geometrías y cierra el SDK de Autodesk
void Model3D::unload() {
    m_meshes.clear();
    textureFileNames.clear();

    // Destruimos la Escena temporal que crea el SDK de FBX
    if (lScene) {
        lScene->Destroy();
        lScene = nullptr;
    }
    // Apagamos el administrador global de memoria del SDK
    if (lSdkManager) {
        lSdkManager->Destroy();
        lSdkManager = nullptr;
    }

    SetState(ResourceState::Unloaded);
}

// Calcula cuánta memoria RAM ocupan todos los vértices e índices de este modelo
size_t Model3D::getSizeInBytes() const {
    size_t size = 0;
    for (const auto& mesh : m_meshes) {
        size += mesh.m_vertex.size() * sizeof(SimpleVertex);
        size += mesh.m_index.size() * sizeof(unsigned int);
    }
    return size;
}

// Inicializa el motor interno del SDK de FBX (necesario antes de cargar cualquier archivo)
bool Model3D::InitializeFBXManager() {
    // Si ya existe, no hacemos nada
    if (lSdkManager) return true;

    lSdkManager = FbxManager::Create();
    if (!lSdkManager) {
        ERROR("Model3D", "InitializeFBXManager", "Unable to create FBX Manager!");
        return false;
    }

    FbxIOSettings* ios = FbxIOSettings::Create(lSdkManager, IOSROOT);
    lSdkManager->SetIOSettings(ios);

    lScene = FbxScene::Create(lSdkManager, "MyScene");
    if (!lScene) {
        ERROR("Model3D", "InitializeFBXManager", "Unable to create FBX Scene!");
        return false;
    }
    return true;
}

// Extrae toda la información de un archivo FBX u OBJ y la convierte en MeshComponents
std::vector<MeshComponent> Model3D::LoadFBXModel(const std::string& filePath) {
    if (!InitializeFBXManager()) return m_meshes;

    // El importador es el objeto encargado de leer los bytes del archivo en el disco
    FbxImporter* lImporter = FbxImporter::Create(lSdkManager, "");
    if (!lImporter) return m_meshes;

    // El parámetro "-1" permite que el SDK adivine el formato automáticamente
    if (!lImporter->Initialize(filePath.c_str(), -1, lSdkManager->GetIOSettings())) {
        ERROR("Model3D", "LoadFBXModel", ("Unable to init Importer: " + std::string(lImporter->GetStatus().GetErrorString())).c_str());
        lImporter->Destroy();
        return m_meshes;
    }

    if (!lImporter->Import(lScene)) {
        ERROR("Model3D", "LoadFBXModel", ("Unable to import Scene: " + std::string(lImporter->GetStatus().GetErrorString())).c_str());
        lImporter->Destroy();
        return m_meshes;
    }

    m_name = lImporter->GetFileName();
    lImporter->Destroy(); // Destruimos el importador para liberar memoria

    // MUY IMPORTANTE: Convertimos el modelo al sistema de DirectX (Left-Handed)
    FbxAxisSystem::DirectX.ConvertScene(lScene);
    FbxSystemUnit::m.ConvertScene(lScene); // Estandariza a Metros

    // CRUCIAL PARA DIRECTX: Convierte Cuadrados (Quads) a Triángulos.
    FbxGeometryConverter geometryConverter(lSdkManager);
    geometryConverter.Triangulate(lScene, /*replace*/true);

    // Empezamos a buscar mallas desde el nodo principal
    FbxNode* lRootNode = lScene->GetRootNode();
    if (lRootNode) {
        for (int i = 0; i < lRootNode->GetChildCount(); i++) {
            ProcessFBXNode(lRootNode->GetChild(i));
        }
    }

    return m_meshes;
}

// Recorre recursivamente el árbol de objetos dentro del archivo 3D
void Model3D::ProcessFBXNode(FbxNode* node) {
    if (!node) return;

    if (node->GetNodeAttribute() && node->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eMesh) {
        ProcessFBXMesh(node);
    }

    for (int i = 0; i < node->GetChildCount(); i++) {
        ProcessFBXNode(node->GetChild(i));
    }
}

// ======================================================================================
// EXTRAE LA GEOMETRÍA: Posición, UV, Normales, Tangentes y Bitangentes.
// (Esta es la versión PBR optimizada del profesor)
// ======================================================================================
void Model3D::ProcessFBXMesh(FbxNode* node) {
    FbxMesh* mesh = node->GetMesh();
    if (!mesh) return;

    // --- Asegura que normales y tangentes existan en el FBX ---
    if (mesh->GetElementNormalCount() == 0)
        mesh->GenerateNormals(true, true);

    const char* uvSetName = nullptr;
    {
        FbxStringList uvSets; mesh->GetUVSetNames(uvSets);
        if (uvSets.GetCount() > 0) uvSetName = uvSets[0];
    }

    if (mesh->GetElementTangentCount() == 0 && uvSetName)
        mesh->GenerateTangentsData(uvSetName);

    // Punteros a los canales de datos
    const FbxGeometryElementUV* uvElem = (mesh->GetElementUVCount() > 0) ? mesh->GetElementUV(0) : nullptr;
    const FbxGeometryElementTangent* tanElem = (mesh->GetElementTangentCount() > 0) ? mesh->GetElementTangent(0) : nullptr;
    const FbxGeometryElementBinormal* binElem = (mesh->GetElementBinormalCount() > 0) ? mesh->GetElementBinormal(0) : nullptr;

    std::vector<SimpleVertex> vertices;
    std::vector<unsigned int> indices;
    vertices.reserve(mesh->GetPolygonCount() * 3);
    indices.reserve(mesh->GetPolygonCount() * 3);

    // Funciones Lambda (Helpers) para leer los datos del complicado formato de FBX
    auto readV2 = [](const FbxGeometryElementUV* elem, int cpIdx, int pvIdx) -> FbxVector2 {
        if (!elem) return FbxVector2(0, 0);
        using E = FbxGeometryElement;
        int idx;
        if (elem->GetMappingMode() == E::eByControlPoint)
            idx = (elem->GetReferenceMode() == E::eIndexToDirect) ? elem->GetIndexArray().GetAt(cpIdx) : cpIdx;
        else
            idx = (elem->GetReferenceMode() == E::eIndexToDirect) ? elem->GetIndexArray().GetAt(pvIdx) : pvIdx;
        return elem->GetDirectArray().GetAt(idx);
        };

    auto readV4 = [](auto* elem, int cpIdx, int pvIdx) -> FbxVector4 {
        if (!elem) return FbxVector4(0, 0, 0, 0);
        using E = FbxGeometryElement;
        int idx;
        if (elem->GetMappingMode() == E::eByControlPoint)
            idx = (elem->GetReferenceMode() == E::eIndexToDirect) ? elem->GetIndexArray().GetAt(cpIdx) : cpIdx;
        else
            idx = (elem->GetReferenceMode() == E::eIndexToDirect) ? elem->GetIndexArray().GetAt(pvIdx) : pvIdx;
        return elem->GetDirectArray().GetAt(idx);
        };

    // --- Construcción del vértice por esquina (Corner approach) ---
    for (int p = 0; p < mesh->GetPolygonCount(); ++p) {
        const int polySize = mesh->GetPolygonSize(p);
        std::vector<unsigned> cornerIdx; cornerIdx.reserve(polySize);

        for (int v = 0; v < polySize; ++v) {
            const int cpIndex = mesh->GetPolygonVertex(p, v);
            const int pvIndex = mesh->GetPolygonVertexIndex(p) + v;

            SimpleVertex out{};

            // 1. Posición local
            FbxVector4 P = mesh->GetControlPointAt(cpIndex);
            out.Position = { (float)P[0], (float)P[1], (float)P[2] };

            // 2. Normal por esquina
            FbxVector4 N(0, 1, 0, 0);
            mesh->GetPolygonVertexNormal(p, v, N);
            N.Normalize();
            out.Normal = { (float)N[0], (float)N[1], (float)N[2] };

            // 3. UV (Invertimos la V para DirectX)
            if (uvElem && uvSetName) {
                int uvIdx = mesh->GetTextureUVIndex(p, v);
                FbxVector2 uv = (uvIdx >= 0) ? uvElem->GetDirectArray().GetAt(uvIdx) : readV2(uvElem, cpIndex, pvIndex);
                out.TextureCoordinate = { (float)uv[0], 1.0f - (float)uv[1] };
            }
            else {
                out.TextureCoordinate = { 0.0f, 0.0f };
            }

            // 4. Tangente y Bitangente (Para el Normal Mapping)
            if (tanElem) {
                FbxVector4 T = readV4(tanElem, cpIndex, pvIndex);
                out.Tangent = { (float)T[0], (float)T[1], (float)T[2] };
            }
            else out.Tangent = { 0,0,0 };

            if (binElem) {
                FbxVector4 B = readV4(binElem, cpIndex, pvIndex);
                out.Bitangent = { (float)B[0], (float)B[1], (float)B[2] };
            }
            else out.Bitangent = { 0,0,0 };

            cornerIdx.push_back((unsigned)vertices.size());
            vertices.push_back(out);
        }

        // Triangulación "en abanico" (CW por defecto)
        for (int k = 1; k + 1 < polySize; ++k) {
            indices.push_back(cornerIdx[0]);
            indices.push_back(cornerIdx[k + 1]);
            indices.push_back(cornerIdx[k]);
        }
    }

    // --- Autodetección de espejo global (Previene normales invertidas en la escala negativa) ---
    bool autoDetectMirror = true;
    bool forceFlipWinding = false;
    bool mirrored = false;

    if (autoDetectMirror) {
        FbxAMatrix geo;
        geo.SetT(node->GetGeometricTranslation(FbxNode::eSourcePivot));
        geo.SetR(node->GetGeometricRotation(FbxNode::eSourcePivot));
        geo.SetS(node->GetGeometricScaling(FbxNode::eSourcePivot));
        FbxAMatrix world = node->EvaluateGlobalTransform() * geo;

        FbxVector4 S = world.GetS();
        double detScale = S[0] * S[1] * S[2];
        mirrored = (detScale < 0.0); // Si el producto de la escala es negativo, está espejado
    }

    if (mirrored || forceFlipWinding) {
        // Invertir el orden de dibujado (Winding)
        for (size_t i = 0; i + 2 < indices.size(); i += 3)
            std::swap(indices[i + 1], indices[i + 2]);

        // Invertir vectores direccionales
        for (auto& v : vertices) {
            v.Normal = { v.Normal.x, v.Normal.y, v.Normal.z };
            v.Tangent = { v.Tangent.x, v.Tangent.y, v.Tangent.z };
            v.Bitangent = { v.Bitangent.x, v.Bitangent.y, v.Bitangent.z };
        }
    }

    // --- Empaquetado Final ---
    MeshComponent mc;
    mc.m_name = node->GetName();
    mc.m_vertex = std::move(vertices);
    mc.m_index = std::move(indices);
    mc.m_numVertex = (int)mc.m_vertex.size();
    mc.m_numIndex = (int)mc.m_index.size();
    m_meshes.push_back(std::move(mc));
}

// Extrae los nombres de las texturas asignadas al modelo en el programa 3D (Opcional para autocaragado)
void Model3D::ProcessFBXMaterials(FbxSurfaceMaterial* material) {
    if (!material) return;

    FbxProperty prop = material->FindProperty(FbxSurfaceMaterial::sDiffuse);
    if (prop.IsValid()) {
        int textureCount = prop.GetSrcObjectCount<FbxTexture>();
        for (int i = 0; i < textureCount; ++i) {
            FbxTexture* texture = FbxCast<FbxTexture>(prop.GetSrcObject<FbxTexture>(i));
            if (texture) {
                textureFileNames.push_back(texture->GetName());
            }
        }
    }
}