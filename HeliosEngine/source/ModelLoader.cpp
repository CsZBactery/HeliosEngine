#include "../include/ModelLoader.h"
#include <iostream>
#include <fstream> 
#include <sstream> 
#include <vector>
#include <string>
#include <map> 
#include <tuple>

namespace {
    std::tuple<int, int, int> parseFaceIndex(const std::string& token) {
        int v = 0, vt = 0, vn = 0;
        std::stringstream ss(token);
        std::string segment;

        // v
        std::getline(ss, segment, '/');
        if (!segment.empty()) v = std::stoi(segment);

        // vt
        if (std::getline(ss, segment, '/')) {
            if (!segment.empty()) vt = std::stoi(segment);
        }

        // vn
        if (std::getline(ss, segment, '/')) {
            if (!segment.empty()) vn = std::stoi(segment);
        }

        return std::make_tuple(v, vt, vn);
    }
} 

// Implementación del Parser
bool
ModelLoader::LoadOBJ(const std::string& objPath, MeshComponent& outMesh, bool flipV)
{
    outMesh.m_name = objPath;
    outMesh.m_vertex.clear();
    outMesh.m_index.clear();

    // 1. Estructuras temporales para datos RAW
    std::vector<XMFLOAT3> temp_positions;
    std::vector<XMFLOAT2> temp_texCoords;
    std::vector<XMFLOAT3> temp_normals;

    temp_positions.push_back({ 0, 0, 0 });
    temp_texCoords.push_back({ 0, 0 });
    temp_normals.push_back({ 0, 0, 0 });

    // 2 Generar el buffer
    std::map<VertexIndices, unsigned int> vertex_cache;
    unsigned int next_index = 0;

    std::ifstream file(objPath);
    std::string line;

    if (!file.is_open()) {
        ERROR(L"ModelLoader", L"LoadOBJ", L"No se pudo abrir el archivo .obj");
        return false;
    }

    MESSAGE(L"ModelLoader", L"LoadOBJ", L"Iniciando parsing manual...");

    // 3. Lectura línea por línea
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        std::stringstream ss(line);
        std::string token;
        ss >> token;

        if (token == "v") { // Posiciones
            XMFLOAT3 pos;
            ss >> pos.x >> pos.y >> pos.z;
            temp_positions.push_back(pos);
        }
        else if (token == "vt") { // Coordenadas de textura
            XMFLOAT2 tc;
            ss >> tc.x >> tc.y;
            if (flipV) {
                tc.y = 1.0f - tc.y;
            }
            temp_texCoords.push_back(tc);
        }
        else if (token == "vn") {
            XMFLOAT3 norm;
            ss >> norm.x >> norm.y >> norm.z;
            temp_normals.push_back(norm);
        }
        else if (token == "f") {
            std::vector<VertexIndices> face_vertices;
            std::string face_token;
            while (ss >> face_token) {
                auto indices = parseFaceIndex(face_token);
                face_vertices.push_back({
                    std::get<0>(indices),
                    std::get<1>(indices),
                    std::get<2>(indices)
                    });
            }

            if (face_vertices.size() < 3) continue;

            // Triangulación
            for (size_t i = 0; i < face_vertices.size() - 2; ++i) {
                VertexIndices tri_indices[] = {
                    face_vertices[0],
                    face_vertices[i + 1],
                    face_vertices[i + 2]
                };

                for (int j = 0; j < 3; ++j) {
                    const auto& current_key = tri_indices[j];

                    // Validación
                    if (current_key.v <= 0 || (size_t)current_key.v >= temp_positions.size() ||
                        (current_key.vt < 0 || (current_key.vt > 0 && (size_t)current_key.vt >= temp_texCoords.size())) ||
                        (current_key.vn < 0 || (current_key.vn > 0 && (size_t)current_key.vn >= temp_normals.size()))) {


                        if (current_key.v > 0) {
                            ERROR(L"ModelLoader", L"LoadOBJ", L"Índice de vértice fuera de rango o inválido.");
                            continue;
                        }
                    }

                    
                    auto it = vertex_cache.find(current_key);
                    if (it != vertex_cache.end()) {
                       
                        outMesh.m_index.push_back(it->second);
                    }
                    else {
                        // Nuevo vértice
                        SimpleVertex new_vertex;
                        new_vertex.Pos = temp_positions[current_key.v];

                        // Si vt es 0 (no existe), usa (0,0). Si no, búscalo.
                        new_vertex.Tex = (current_key.vt > 0) ? temp_texCoords[current_key.vt] : XMFLOAT2(0, 0);

                        // Si vn es 0 (no existe), usa (0,0,0). Si no, búscalo.
                        new_vertex.Normal = (current_key.vn > 0) ? temp_normals[current_key.vn] : XMFLOAT3(0, 0, 0);

                        // Añadir nuevo vértice
                        outMesh.m_vertex.push_back(new_vertex);
                        vertex_cache[current_key] = next_index;

                        // Añadir índice
                        outMesh.m_index.push_back(next_index);
                        next_index++;
                    }
                }
            }
        }
        
    }

    file.close();

    outMesh.m_numVertex = static_cast<int>(outMesh.m_vertex.size());
    outMesh.m_numIndex = static_cast<int>(outMesh.m_index.size());

    MESSAGE(L"ModelLoader", L"LoadOBJ", L"Parsing OBJ finalizado.");
    return true; 
}