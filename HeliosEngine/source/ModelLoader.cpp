// --- En ModelLoader.cpp ---

#include "../include/ModelLoader.h"
#include <iostream>
#include <fstream>   // Para leer archivos (ifstream)
#include <sstream>   // Para parsear líneas (stringstream)
#include <vector>
#include <map>       // Para vértices únicos (¡la clave de la robustez!)

// Implementación de la función LoadOBJ
bool ModelLoader::LoadOBJ(const std::string& objPath, MeshComponent& outMesh, bool flipV)
{
    // --- Almacenamiento temporal para los datos leídos del OBJ ---
    // El formato .obj guarda posiciones, texturas y normales en listas separadas.
    std::vector<XMFLOAT3> temp_positions;
    std::vector<XMFLOAT2> temp_texcoords;
    std::vector<XMFLOAT3> temp_normals;

    // --- Almacenamiento para las definiciones de caras ---
    // Guardamos las líneas 'f' para procesarlas al final.
    std::vector<std::string> face_definitions;

    // --- Mapa de Vértices Únicos (¡La Clave de un Buen Parser!) ---
    // Un vértice final no es solo una posición, es la combinación ÚNICA
    // de (posición / textura / normal).
    // Usamos un mapa para no duplicar vértices.
    // La clave es el string "v/vt/vn", el valor es el índice final (UINT).
    std::map<std::string, unsigned int> vertex_map;

    // Limpiamos la malla de salida por si acaso
    outMesh.m_vertex.clear();
    outMesh.m_index.clear();

    // --- 1. LEER EL ARCHIVO LÍNEA POR LÍNEA ---
    std::ifstream file(objPath);
    if (!file.is_open())
    {
        // ¡Este error debería aparecer en tu log de salida si la ruta está mal!
        OutputDebugStringA(("ERROR: ModelLoader no pudo abrir el archivo: " + objPath + "\n").c_str());
        return false;
    }

    std::string line;
    while (std::getline(file, line))
    {
        // Ignorar líneas vacías o comentarios
        if (line.empty() || line[0] == '#') {
            continue;
        }

        std::stringstream ss(line);
        std::string lineHeader;
        ss >> lineHeader; // Ej: "v", "vt", "vn", "f", "mtllib"

        if (lineHeader == "v") // Vértice de Posición
        {
            float x, y, z;
            ss >> x >> y >> z;
            temp_positions.push_back(XMFLOAT3(x, y, z));
        }
        else if (lineHeader == "vt") // Vértice de Textura
        {
            float u, v;
            ss >> u >> v;
            // Invertimos la V si es necesario (Blender/Maya vs DirectX)
            if (flipV)
            {
                v = 1.0f - v;
            }
            temp_texcoords.push_back(XMFLOAT2(u, v));
        }
        else if (lineHeader == "vn") // Vértice de Normal
        {
            float x, y, z;
            ss >> x >> y >> z;
            temp_normals.push_back(XMFLOAT3(x, y, z));
        }
        else if (lineHeader == "f") // Cara (Face)
        {
            // Guardamos la línea de la cara (sin la 'f' inicial)
            std::string face_data(line.substr(line.find(" ") + 1));
            face_definitions.push_back(face_data);
        }
        else if (lineHeader == "mtllib" || lineHeader == "usemtl" || lineHeader == "s" || lineHeader == "g" || lineHeader == "o")
        {
            // Robustez: Ignoramos líneas que no soportamos (materiales, grupos, etc.)
            // ¡Esto evita que el parser falle si el .obj tiene materiales!
        }
    }
    file.close();

    // --- 2. PROCESAMIENTO DE CARAS (Construcción de la Malla) ---
    // Ahora leemos todas las caras que guardamos.

    for (const auto& face_data : face_definitions)
    {
        std::stringstream ss_face(face_data);
        std::string vertex_def; // Ej: "1/1/1" o "1//1" o "1"

        // Guarda los índices FINALES de esta cara (ej. 3 para un triángulo, 4 para un quad)
        std::vector<unsigned int> face_indices;

        // Itera sobre cada vértice de la cara (ej. "1/1/1", "2/2/1", "3/3/1")
        while (ss_face >> vertex_def)
        {
            // Buscamos si ya hemos procesado este vértice compuesto ("v/vt/vn")
            if (vertex_map.find(vertex_def) == vertex_map.end())
            {
                // --- Vértice Nuevo ---
                // No lo tenemos, así que debemos crearlo.
                SimpleVertex new_vert{};
                std::stringstream ss_vert(vertex_def);
                std::string index_str;

                // Parseamos los índices (v, vt, vn)
                // Los índices de OBJ empiezan en 1, así que restamos 1.

                // 1. Índice de Posición (v)
                std::getline(ss_vert, index_str, '/');
                UINT v_idx = static_cast<UINT>(std::stoul(index_str)) - 1;
                new_vert.Pos = temp_positions[v_idx];

                // 2. Índice de Textura (vt)
                if (!ss_vert.eof()) // ¿Hay más después del primer '/'?
                {
                    std::getline(ss_vert, index_str, '/');
                    if (!index_str.empty()) // Si es "v//vn", index_str estará vacío
                    {
                        // Comprobación de robustez: ¿Existen coordenadas de textura?
                        if (!temp_texcoords.empty()) {
                            UINT vt_idx = static_cast<UINT>(std::stoul(index_str)) - 1;
                            new_vert.Tex = temp_texcoords[vt_idx];
                        }
                    }
                }

                // 3. Índice de Normal (vn)
                if (!ss_vert.eof()) // ¿Hay más después del segundo '/'?
                {
                    std::getline(ss_vert, index_str, '/');
                    if (!index_str.empty())
                    {
                        // Comprobación de robustez: ¿Existen normales?
                        if (!temp_normals.empty()) {
                            UINT vn_idx = static_cast<UINT>(std::stoul(index_str)) - 1;
                            new_vert.Normal = temp_normals[vn_idx];
                        }
                    }
                }

                // Añadimos el nuevo vértice a la malla final
                outMesh.m_vertex.push_back(new_vert);
                UINT new_index = (UINT)outMesh.m_vertex.size() - 1;

                // Guardamos el nuevo índice en el mapa
                vertex_map[vertex_def] = new_index;
                face_indices.push_back(new_index);
            }
            else
            {
                // --- Vértice Existente ---
                // ¡Ya lo hemos procesado! Simplemente obtenemos su índice del mapa.
                face_indices.push_back(vertex_map[vertex_def]);
            }
        } // Fin del while (procesar una cara)

        // --- 3. TRIANGULACIÓN (Fan Triangulation) ---
        // Ahora convertimos la cara (que puede tener 3, 4, o más vértices) en triángulos.
        // Si la cara es (i0, i1, i2, i3), creamos dos triángulos:
        // (i0, i1, i2) y (i0, i2, i3)
        // Esto funciona para Quads y N-Gons (siempre que sean convexos)

        for (size_t i = 1; i + 1 < face_indices.size(); ++i)
        {
            outMesh.m_index.push_back(face_indices[0]);
            outMesh.m_index.push_back(face_indices[i]);
            outMesh.m_index.push_back(face_indices[i + 1]);
        }
    } // Fin del for (procesar todas las caras)

    // Llenamos los contadores finales
    outMesh.m_numVertex = (int)outMesh.m_vertex.size();
    outMesh.m_numIndex = (int)outMesh.m_index.size();

    OutputDebugStringA("ModelLoader: Carga de OBJ robusta finalizada.\n");
    return true;
}