#include "../include/ModelLoader.h"
#include <iostream>
#include <fstream>   // Para leer archivos (ifstream)
#include <sstream>   // Para parsear líneas (stringstream)
#include <vector>
#include <string>
#include <map>       // Para vértices únicos
#include <tuple>     // Para la función auxiliar

// ----------------------------------------------------------------------------------
// Funciones Auxiliares (Namespace anónimo)
// ----------------------------------------------------------------------------------
namespace {
    /**
     * @brief Parsea un token de cara (ej: "1/2/3") y extrae los índices v, vt, vn.
     */
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
} // fin del namespace anónimo

// ----------------------------------------------------------------------------------
// Implementación del Parser (Adaptado a tu .h)
// ----------------------------------------------------------------------------------
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

    // ¡¡IMPORTANTE!! Añadir valores dummy en [0]
    // El formato OBJ usa índices 1-basados (empieza a contar en 1, no en 0)
    // Poner un dummy en [0] hace que podamos usar el índice 'v' directamente.
    temp_positions.push_back({ 0, 0, 0 });
    temp_texCoords.push_back({ 0, 0 });
    temp_normals.push_back({ 0, 0, 0 });

    // 2. Cache para generar el buffer indexado único
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
            if (flipV) { // ¡Usamos el parámetro flipV!
                tc.y = 1.0f - tc.y;
            }
            temp_texCoords.push_back(tc);
        }
        else if (token == "vn") { // Normales
            XMFLOAT3 norm;
            ss >> norm.x >> norm.y >> norm.z;
            temp_normals.push_back(norm);
        }
        else if (token == "f") { // Caras y Triangulación
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

            if (face_vertices.size() < 3) continue; // Ignorar líneas/puntos

            // Triangulación "Fan" (i0, i1, i2), (i0, i2, i3), ...
            for (size_t i = 0; i < face_vertices.size() - 2; ++i) {
                VertexIndices tri_indices[] = {
                    face_vertices[0],
                    face_vertices[i + 1],
                    face_vertices[i + 2]
                };

                for (int j = 0; j < 3; ++j) {
                    const auto& current_key = tri_indices[j];

                    // Validación (Asegura que los índices no se salgan del rango)
                    if (current_key.v <= 0 || (size_t)current_key.v >= temp_positions.size() ||
                        (current_key.vt < 0 || (current_key.vt > 0 && (size_t)current_key.vt >= temp_texCoords.size())) ||
                        (current_key.vn < 0 || (current_key.vn > 0 && (size_t)current_key.vn >= temp_normals.size()))) {

                        // Si faltan UVs o Normales (vt=0 o vn=0), el 'get' de abajo lo manejará.
                        // Esto es solo para índices positivos que se salen del vector.
                        if (current_key.v > 0) {
                            ERROR(L"ModelLoader", L"LoadOBJ", L"Índice de vértice fuera de rango o inválido.");
                            continue;
                        }
                    }

                    // 4. Indexación con Cache
                    auto it = vertex_cache.find(current_key);
                    if (it != vertex_cache.end()) {
                        // Vértice ya existe: reusar índice
                        outMesh.m_index.push_back(it->second);
                    }
                    else {
                        // Nuevo vértice: crear SimpleVertex
                        SimpleVertex new_vertex;
                        new_vertex.Pos = temp_positions[current_key.v];

                        // Si vt es 0 (no existe), usa (0,0). Si no, búscalo.
                        new_vertex.Tex = (current_key.vt > 0) ? temp_texCoords[current_key.vt] : XMFLOAT2(0, 0);

                        // Si vn es 0 (no existe), usa (0,0,0). Si no, búscalo.
                        new_vertex.Normal = (current_key.vn > 0) ? temp_normals[current_key.vn] : XMFLOAT3(0, 0, 0);

                        // Añadir nuevo vértice y actualizar cache
                        outMesh.m_vertex.push_back(new_vertex);
                        vertex_cache[current_key] = next_index;

                        // Añadir índice al buffer de índices
                        outMesh.m_index.push_back(next_index);
                        next_index++;
                    }
                }
            }
        }
        // Ignorar 'usemtl', 's', 'g', etc.
    }

    file.close();

    // 5. Finalización
    outMesh.m_numVertex = static_cast<int>(outMesh.m_vertex.size());
    outMesh.m_numIndex = static_cast<int>(outMesh.m_index.size());

    MESSAGE(L"ModelLoader", L"LoadOBJ", L"Parsing OBJ finalizado.");
    return true; // ¡Devolvemos true en éxito!
}