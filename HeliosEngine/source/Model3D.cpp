#include "Model3D.h"

Model3D::~Model3D() {
    unload();
}

bool Model3D::load(const std::string& path) {
    SetPath(path);
    SetState(ResourceState::Loading);

    // Intentar inicializar
    if (init()) {
        SetState(ResourceState::Loaded);
        return true;
    }

    SetState(ResourceState::Failed);
    return false;
}

bool Model3D::init() {
    // Si ya hay mallas cargadas, limpiamos
    m_meshes.clear();

    // Cargar el modelo según la ruta
    LoadFBXModel(m_filePath);

    return !m_meshes.empty();
}

void Model3D::unload() {
    m_meshes.clear();
    textureFileNames.clear();

    // Destruir el Scene y Manager de FBX si existen
    if (lScene) {
        lScene->Destroy();
        lScene = nullptr;
    }
    if (lSdkManager) {
        lSdkManager->Destroy();
        lSdkManager = nullptr;
    }

    SetState(ResourceState::Unloaded);
}

size_t Model3D::getSizeInBytes() const {
    size_t size = 0;
    for (const auto& mesh : m_meshes) {
        size += mesh.m_vertex.size() * sizeof(SimpleVertex);
        size += mesh.m_index.size() * sizeof(unsigned int);
    }
    return size;
}

bool Model3D::InitializeFBXManager() {
    if (lSdkManager) return true; // Ya inicializado

    lSdkManager = FbxManager::Create();
    if (!lSdkManager) {
        ERROR("Model3D", "InitializeFBXManager", "Unable to create FBX Manager!");
        return false;
    }

    FbxIOSettings* ios = FbxIOSettings::Create(lSdkManager, IOSROOT);
    lSdkManager->SetIOSettings(ios);

    return true;
}

void Model3D::LoadFBXModel(const std::string& filePath) {
    if (!InitializeFBXManager()) return;

    // Crear importador
    FbxImporter* lImporter = FbxImporter::Create(lSdkManager, "");
    if (!lImporter) {
        ERROR("Model3D", "LoadFBXModel", "Unable to create FBX Importer!");
        return;
    }

    // Inicializar importador con el archivo
    // NOTA: -1 permite que el SDK detecte el formato automáticamente
    if (!lImporter->Initialize(filePath.c_str(), -1, lSdkManager->GetIOSettings())) {
        ERROR("Model3D", "LoadFBXModel", ("Unable to initialize Importer for: " + filePath + " Error: " + lImporter->GetStatus().GetErrorString()).c_str());
        lImporter->Destroy();
        return;
    }

    // Crear escena
    lScene = FbxScene::Create(lSdkManager, "MyScene");

    // Importar contenido
    if (!lImporter->Import(lScene)) {
        ERROR("Model3D", "LoadFBXModel", "Unable to import FBX Scene!");
        lImporter->Destroy();
        return;
    }

    lImporter->Destroy(); // Ya no necesitamos el importador

    // Convertir sistema de coordenadas a DirectX (Left Handed)
    FbxAxisSystem::DirectX.ConvertScene(lScene);

    // Triangular la malla (Crucial para DirectX si el modelo viene en Quads)
    FbxGeometryConverter geometryConverter(lSdkManager);
    geometryConverter.Triangulate(lScene, /*replace*/true);

    // Procesar nodo raíz
    FbxNode* lRootNode = lScene->GetRootNode();
    if (lRootNode) {
        ProcessFBXNode(lRootNode);
    }
}

void Model3D::ProcessFBXNode(FbxNode* node) {
    if (!node) return;

    // Verificar si el nodo contiene atributos de malla
    FbxNodeAttribute* attribute = node->GetNodeAttribute();
    if (attribute && attribute->GetAttributeType() == FbxNodeAttribute::eMesh) {
        ProcessFBXMesh(node);
    }

    // Procesar hijos recursivamente
    for (int i = 0; i < node->GetChildCount(); i++) {
        ProcessFBXNode(node->GetChild(i));
    }
}

void Model3D::ProcessFBXMesh(FbxNode* node) {
    FbxMesh* mesh = node->GetMesh();
    if (!mesh) return;

    MeshComponent mc;
    mc.m_name = node->GetName();

    // Obtener UV sets si existen
    const FbxGeometryElementUV* uvElement = nullptr;
    if (mesh->GetElementUVCount() > 0) {
        uvElement = mesh->GetElementUV(0);
    }

    // Iterar sobre polígonos (triángulos, ya que triangulamos antes)
    int polygonCount = mesh->GetPolygonCount();
    int vertexCounter = 0;

    for (int i = 0; i < polygonCount; i++) {
        int polygonSize = mesh->GetPolygonSize(i); // Debería ser 3

        for (int j = 0; j < polygonSize; j++) {
            int controlPointIndex = mesh->GetPolygonVertex(i, j);

            SimpleVertex vertex;

            // 1. POSICIÓN
            FbxVector4 pos = mesh->GetControlPointAt(controlPointIndex);
            vertex.Pos.x = (float)pos.mData[0];
            vertex.Pos.y = (float)pos.mData[1];
            vertex.Pos.z = (float)pos.mData[2];

            // 2. UVs (COORDENADAS DE TEXTURA)
            if (uvElement) {
                FbxVector2 uv;
                // Mapeo: ByControlPoint o ByPolygonVertex
                switch (uvElement->GetMappingMode()) {
                case FbxGeometryElement::eByControlPoint:
                    switch (uvElement->GetReferenceMode()) {
                    case FbxGeometryElement::eDirect:
                        uv = uvElement->GetDirectArray().GetAt(controlPointIndex);
                        break;
                    case FbxGeometryElement::eIndexToDirect:
                    {
                        int id = uvElement->GetIndexArray().GetAt(controlPointIndex);
                        uv = uvElement->GetDirectArray().GetAt(id);
                    }
                    break;
                    }
                    break;

                case FbxGeometryElement::eByPolygonVertex:
                {
                    int textureUVIndex = mesh->GetTextureUVIndex(i, j);
                    switch (uvElement->GetReferenceMode()) {
                    case FbxGeometryElement::eDirect:
                    case FbxGeometryElement::eIndexToDirect:
                        uv = uvElement->GetDirectArray().GetAt(textureUVIndex);
                        break;
                    }
                }
                break;
                }

                vertex.Tex.x = (float)uv.mData[0];
                vertex.Tex.y = 1.0f - (float)uv.mData[1]; // Invertir V para DirectX
            }
            else {
                vertex.Tex = { 0.0f, 0.0f };
            }

            // Agregar vértice e índice
            mc.m_vertex.push_back(vertex);
            mc.m_index.push_back(vertexCounter);
            vertexCounter++;
        }
    }

    mc.m_numVertex = (int)mc.m_vertex.size();
    mc.m_numIndex = (int)mc.m_index.size();

    // Guardar la malla procesada
    m_meshes.push_back(mc);
}

void Model3D::ProcessFBXMaterials(FbxSurfaceMaterial* material) {
    // Implementación básica para extraer nombres de texturas si fuera necesario
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