// ======================================================================================
// Archivo: Model3D.cpp
// Implementación del gestor de recursos para modelos tridimensionales (OBJ y FBX).
// ======================================================================================

#include "Model3D.h"

// NOTA: El destructor fue eliminado de aquí porque en Model3D.h ya lo definimos 
// como "~Model3D() = default;". Esto soluciona el error "already has a body".

// Inicia el proceso estándar de carga desde el sistema de archivos
bool Model3D::load(const std::string& path) {
    SetPath(path);
    SetState(ResourceState::Loading);

    // Intentamos procesar el archivo y extraer sus mallas
    if (init()) {
        SetState(ResourceState::Loaded);
        return true;
    }

    SetState(ResourceState::Failed);
    return false;
}

// Inicia la conversión del archivo 3D a datos que el motor puede usar
bool Model3D::init() {
    // Si ya hay mallas cargadas (por ejemplo si hacemos un recargado), las limpiamos
    m_meshes.clear();

    // Enviamos la ruta al procesador de FBX (el cual también puede leer OBJs)
    LoadFBXModel(m_filePath);

    // Si el vector de mallas no está vacío, la carga fue un éxito
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

// Calcula cuánta memoria RAM/VRAM ocupan todos los vértices e índices de este modelo
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
    // Si ya existe, no hacemos nada para evitar fugas de memoria
    if (lSdkManager) return true;

    lSdkManager = FbxManager::Create();
    if (!lSdkManager) {
        ERROR("Model3D", "InitializeFBXManager", "Unable to create FBX Manager!");
        return false;
    }

    FbxIOSettings* ios = FbxIOSettings::Create(lSdkManager, IOSROOT);
    lSdkManager->SetIOSettings(ios);

    return true;
}

// Extrae toda la información de un archivo FBX u OBJ y la convierte en MeshComponents
std::vector<MeshComponent> Model3D::LoadFBXModel(const std::string& filePath) {
    if (!InitializeFBXManager()) return m_meshes;

    // El importador es el objeto encargado de leer los bytes del archivo en el disco
    FbxImporter* lImporter = FbxImporter::Create(lSdkManager, "");
    if (!lImporter) {
        ERROR("Model3D", "LoadFBXModel", "Unable to create FBX Importer!");
        return m_meshes;
    }

    // El parámetro "-1" permite que el SDK adivine el formato de archivo automáticamente
    if (!lImporter->Initialize(filePath.c_str(), -1, lSdkManager->GetIOSettings())) {
        ERROR("Model3D", "LoadFBXModel", ("Unable to initialize Importer for: " + filePath + " Error: " + lImporter->GetStatus().GetErrorString()).c_str());
        lImporter->Destroy();
        return m_meshes;
    }

    // La Escena es donde se almacenará toda la jerarquía de nodos, mallas y materiales
    lScene = FbxScene::Create(lSdkManager, "MyScene");

    if (!lImporter->Import(lScene)) {
        ERROR("Model3D", "LoadFBXModel", "Unable to import FBX Scene!");
        lImporter->Destroy();
        return m_meshes;
    }

    // Destruimos el importador para liberar memoria, ya que la info ya está en lScene
    lImporter->Destroy();

    // MUY IMPORTANTE: Convertimos el modelo de FBX (usualmente Right-Handed) 
    // al sistema que utiliza DirectX (Left-Handed)
    FbxAxisSystem::DirectX.ConvertScene(lScene);

    // CRUCIAL PARA DIRECTX: Si el artista hizo el modelo usando Cuadrados (Quads),
    // esta función convierte todo obligatoriamente a Triángulos.
    FbxGeometryConverter geometryConverter(lSdkManager);
    geometryConverter.Triangulate(lScene, /*replace*/true);

    // Empezamos a buscar mallas desde el nodo principal
    FbxNode* lRootNode = lScene->GetRootNode();
    if (lRootNode) {
        ProcessFBXNode(lRootNode);
    }

    // Retornamos la lista de mallas que pudimos extraer
    return m_meshes;
}

// Recorre recursivamente el árbol de objetos dentro del archivo 3D
void Model3D::ProcessFBXNode(FbxNode* node) {
    if (!node) return;

    // Verificamos si este "nodo" es en realidad una geometría (Malla)
    FbxNodeAttribute* attribute = node->GetNodeAttribute();
    if (attribute && attribute->GetAttributeType() == FbxNodeAttribute::eMesh) {
        ProcessFBXMesh(node);
    }

    // Volvemos a llamar a esta función por cada "hijo" que tenga este nodo
    for (int i = 0; i < node->GetChildCount(); i++) {
        ProcessFBXNode(node->GetChild(i));
    }
}

// Convierte un FbxMesh (formato de Autodesk) a nuestro formato de motor (MeshComponent)
void Model3D::ProcessFBXMesh(FbxNode* node) {
    FbxMesh* mesh = node->GetMesh();
    if (!mesh) return;

    // Asegura que las normales existan calculándolas automáticamente si el archivo no las traía
    mesh->GenerateNormals(true, true);

    MeshComponent mc;
    mc.m_name = node->GetName();

    // Buscamos los canales de UVs y Normales dentro de la malla
    const FbxGeometryElementUV* uvElement = (mesh->GetElementUVCount() > 0) ? mesh->GetElementUV(0) : nullptr;
    const FbxGeometryElementNormal* normalElement = (mesh->GetElementNormalCount() > 0) ? mesh->GetElementNormal(0) : nullptr;

    int polygonCount = mesh->GetPolygonCount();
    int vertexCounter = 0;

    // Iteramos por cada triángulo (polígono) de la malla
    for (int i = 0; i < polygonCount; i++) {
        int polygonSize = mesh->GetPolygonSize(i); // Como triangulamos arriba, esto siempre será 3

        // Iteramos por cada uno de los 3 vértices del triángulo
        for (int j = 0; j < polygonSize; j++) {
            int controlPointIndex = mesh->GetPolygonVertex(i, j);

            SimpleVertex vertex;

            // 1. EXTRAER POSICIÓN (X, Y, Z)
            FbxVector4 pos = mesh->GetControlPointAt(controlPointIndex);
            vertex.Pos.x = (float)pos.mData[0];
            vertex.Pos.y = (float)pos.mData[1];
            vertex.Pos.z = (float)pos.mData[2];

            // 2. EXTRAER UVs (Coordenadas de Textura)
            if (uvElement) {
                FbxVector2 uv;
                int uvIndex = -1;

                // El SDK de FBX guarda los datos de formas muy extrañas, aquí descubrimos 
                // dónde está guardado el UV exacto de este vértice en específico.
                if (uvElement->GetMappingMode() == FbxGeometryElement::eByControlPoint) {
                    uvIndex = (uvElement->GetReferenceMode() == FbxGeometryElement::eDirect)
                        ? controlPointIndex
                        : uvElement->GetIndexArray().GetAt(controlPointIndex);
                }
                else if (uvElement->GetMappingMode() == FbxGeometryElement::eByPolygonVertex) {
                    uvIndex = (uvElement->GetReferenceMode() == FbxGeometryElement::eDirect)
                        ? mesh->GetTextureUVIndex(i, j)
                        : mesh->GetTextureUVIndex(i, j);
                }

                if (uvIndex != -1) {
                    uv = uvElement->GetDirectArray().GetAt(uvIndex);
                    vertex.Tex.x = (float)uv.mData[0];
                    vertex.Tex.y = 1.0f - (float)uv.mData[1]; // INVERTIMOS LA V porque DirectX lee las imágenes al revés que OpenGL/Maya
                }
            }
            else {
                vertex.Tex = { 0.0f, 0.0f }; // Si no tiene UV, rellenamos con 0 para evitar fallos
            }

            // 3. EXTRAER NORMALES (Crucial para que la iluminación funcione)
            if (normalElement) {
                FbxVector4 normal;
                int normalIndex = -1;

                // Misma lógica de búsqueda extraña del SDK que aplicamos en los UVs
                if (normalElement->GetMappingMode() == FbxGeometryElement::eByControlPoint) {
                    normalIndex = (normalElement->GetReferenceMode() == FbxGeometryElement::eDirect)
                        ? controlPointIndex
                        : normalElement->GetIndexArray().GetAt(controlPointIndex);
                }
                else if (normalElement->GetMappingMode() == FbxGeometryElement::eByPolygonVertex) {
                    normalIndex = (normalElement->GetReferenceMode() == FbxGeometryElement::eDirect)
                        ? vertexCounter
                        : normalElement->GetIndexArray().GetAt(vertexCounter);
                }

                if (normalIndex != -1) {
                    normal = normalElement->GetDirectArray().GetAt(normalIndex);
                    vertex.Normal.x = (float)normal.mData[0];
                    vertex.Normal.y = (float)normal.mData[1];
                    vertex.Normal.z = (float)normal.mData[2];
                }
            }
            else {
                // Si el modelo de plano no tiene normales y falló la autogeneración, apuntamos hacia arriba
                vertex.Normal = { 0.0f, 1.0f, 0.0f };
            }

            // 4. GUARDAR VÉRTICE E ÍNDICE
            mc.m_vertex.push_back(vertex);
            mc.m_index.push_back(vertexCounter);
            vertexCounter++;
        }
    }

    // Guardamos los contadores finales para que la GPU sepa cuánto dibujar
    mc.m_numVertex = (int)mc.m_vertex.size();
    mc.m_numIndex = (int)mc.m_index.size();

    // Registramos la malla resultante en nuestro arreglo de la clase
    m_meshes.push_back(mc);
}

// Extrae los nombres de las texturas asignadas al modelo desde el programa 3D (Ej. Blender/Maya)
void Model3D::ProcessFBXMaterials(FbxSurfaceMaterial* material) {
    if (!material) return;

    // Buscamos si el material tiene un canal "Difuso" (Color base)
    FbxProperty prop = material->FindProperty(FbxSurfaceMaterial::sDiffuse);
    if (prop.IsValid()) {
        int textureCount = prop.GetSrcObjectCount<FbxTexture>();
        for (int i = 0; i < textureCount; ++i) {
            FbxTexture* texture = FbxCast<FbxTexture>(prop.GetSrcObject<FbxTexture>(i));
            if (texture) {
                // Guardamos el nombre en un vector por si luego queremos hacer carga automática
                textureFileNames.push_back(texture->GetName());
            }
        }
    }
}