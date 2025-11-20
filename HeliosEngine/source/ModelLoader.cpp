#include "../include/ModelLoader.h" 
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <tuple>
#include <cmath>

// -----------------------------
// Helpers (namespace anónimo)
// -----------------------------
namespace
{
    inline void trim(std::string& s) {
        size_t b = s.find_first_not_of(" \t\r\n");
        size_t e = s.find_last_not_of(" \t\r\n");
        if (b == std::string::npos) { s.clear(); return; }
        s = s.substr(b, e - b + 1);
    }

    
    inline std::tuple<int, int, int> parseFaceIndex(const std::string& token) {
        int v = 0, vt = 0, vn = 0;
        std::stringstream ss(token);
        std::string a, b, c;

        if (!std::getline(ss, a, '/')) return std::make_tuple(0, 0, 0);
        if (!a.empty()) v = std::stoi(a);

        if (std::getline(ss, b, '/')) {
            if (!b.empty()) vt = std::stoi(b);
            if (std::getline(ss, c, '/')) {
                if (!c.empty()) vn = std::stoi(c);
            }
        }
        return std::make_tuple(v, vt, vn);
    }

    // Convierte índice OBJ
    // Tenemos slot 0 reservado en los vectores.
    inline int resolveIndex(int idx, size_t size) {
        if (idx > 0)       return idx;              
        else if (idx < 0)  return int(size) + idx; 
        else               return 0;    
    }

    inline XMFLOAT3 normalize(const XMFLOAT3& v) {
        float len = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
        if (len <= 1e-8f) return XMFLOAT3(0, 0, 0);
        return XMFLOAT3(v.x / len, v.y / len, v.z / len);
    }
}

// ----------------------------------------------------------
// Implementación del Parser OBJ
// ----------------------------------------------------------
bool OBJParser::LoadOBJ(const std::string& objPath, MeshComponent& outMesh, bool flipV)
{
    outMesh.m_name = objPath;
    outMesh.m_vertex.clear();
    outMesh.m_index.clear();

    // Slot 0 reservado como “vacío”
    std::vector<XMFLOAT3> temp_positions(1, XMFLOAT3(0, 0, 0));
    std::vector<XMFLOAT2> temp_texCoords(1, XMFLOAT2(0, 0));
    std::vector<XMFLOAT3> temp_normals(1, XMFLOAT3(0, 0, 0));

    // Usa el tipo anidado OBJParser::VertexIndices
    std::map<OBJParser::VertexIndices, unsigned int> vertex_cache;
    unsigned int next_index = 0;

    std::ifstream file(objPath.c_str());
    if (!file.is_open()) {
        ERROR(L"OBJParser", L"LoadOBJ", L"No se pudo abrir el archivo .obj");
        return false;
    }
    MESSAGE(L"OBJParser", L"LoadOBJ", L"Iniciando parsing manual...");

    std::string line;
    while (std::getline(file, line))
    {
        trim(line);
        if (line.empty() || line[0] == '#') continue;

        std::stringstream ls(line);
        std::string tok;
        ls >> tok;

        if (tok == "v") {
            XMFLOAT3 p; ls >> p.x >> p.y >> p.z;
            temp_positions.push_back(p);
        }
        else if (tok == "vt") {
            XMFLOAT2 t; ls >> t.x >> t.y;
            if (flipV) t.y = 1.0f - t.y;
            temp_texCoords.push_back(t);
        }
        else if (tok == "vn") {
            XMFLOAT3 n; ls >> n.x >> n.y >> n.z;
            temp_normals.push_back(n);
        }
        else if (tok == "f") {
            // Polígono de caras
            std::vector<OBJParser::VertexIndices> polygon;
            std::string vtok;

            while (ls >> vtok) {
                int iv = 0, ivt = 0, ivn = 0;
                std::tie(iv, ivt, ivn) = parseFaceIndex(vtok);

                iv = resolveIndex(iv, temp_positions.size());
                ivt = resolveIndex(ivt, temp_texCoords.size());
                ivn = resolveIndex(ivn, temp_normals.size());

                if (iv <= 0 || iv >= (int)temp_positions.size()) {
                    ERROR(L"OBJParser", L"LoadOBJ", L"Índice de posición fuera de rango");
                    continue;
                }
                if (ivt < 0 || ivt >= (int)temp_texCoords.size()) ivt = 0;
                if (ivn < 0 || ivn >= (int)temp_normals.size())   ivn = 0;

                polygon.push_back(OBJParser::VertexIndices{ iv, ivt, ivn });
            }

            if (polygon.size() < 3) continue;

            // Triangulación tipo fan: (0, i+1, i+2)
            for (size_t i = 0; i + 2 < polygon.size(); ++i) {
                OBJParser::VertexIndices tri[3] = {
                    polygon[0], polygon[i + 1], polygon[i + 2]
                };

                for (int k = 0; k < 3; ++k) {
                    const OBJParser::VertexIndices& key = tri[k];

                    auto it = vertex_cache.find(key);
                    if (it != vertex_cache.end()) {
                        // Reutiliza índice
                        outMesh.m_index.push_back(it->second);
                    }
                    else {
                        // Crea vértice nuevo
                        SimpleVertex v{};
                        v.Pos = temp_positions[key.v];
                        v.Tex = (key.vt != 0) ? temp_texCoords[key.vt] : XMFLOAT2(0, 0);
                        v.Normal = (key.vn != 0) ? temp_normals[key.vn] : XMFLOAT3(0, 0, 0);

                        outMesh.m_vertex.push_back(v);
                        vertex_cache[key] = next_index;
                        outMesh.m_index.push_back(next_index);
                        ++next_index;
                    }
                }
            }
        }
        else {
            // Ignorar otras líneas: o, g, s, usemtl, mtllib, etc.
        }
    }
    file.close();

    // Si no hay normales, calcúlalas
    bool needNormals = true;
    for (size_t i = 0; i < outMesh.m_vertex.size(); ++i) {
        const XMFLOAT3& n = outMesh.m_vertex[i].Normal;
        if (n.x != 0 || n.y != 0 || n.z != 0) { needNormals = false; break; }
    }

    if (needNormals && outMesh.m_index.size() >= 3) {
        std::vector<XMFLOAT3> acc(outMesh.m_vertex.size(), XMFLOAT3(0, 0, 0));

        for (size_t i = 0; i + 2 < outMesh.m_index.size(); i += 3) {
            unsigned ia = outMesh.m_index[i + 0];
            unsigned ib = outMesh.m_index[i + 1];
            unsigned ic = outMesh.m_index[i + 2];

            const XMFLOAT3& A = outMesh.m_vertex[ia].Pos;
            const XMFLOAT3& B = outMesh.m_vertex[ib].Pos;
            const XMFLOAT3& C = outMesh.m_vertex[ic].Pos;

            XMFLOAT3 AB{ B.x - A.x, B.y - A.y, B.z - A.z };
            XMFLOAT3 AC{ C.x - A.x, C.y - A.y, C.z - A.z };
            XMFLOAT3 N{
                AB.y * AC.z - AB.z * AC.y,
                AB.z * AC.x - AB.x * AC.z,
                AB.x * AC.y - AB.y * AC.x
            };

            acc[ia].x += N.x; acc[ia].y += N.y; acc[ia].z += N.z;
            acc[ib].x += N.x; acc[ib].y += N.y; acc[ib].z += N.z;
            acc[ic].x += N.x; acc[ic].y += N.y; acc[ic].z += N.z;
        }

        for (size_t i = 0; i < outMesh.m_vertex.size(); ++i) {
            outMesh.m_vertex[i].Normal = normalize(acc[i]);
        }
    }

    outMesh.m_numVertex = (int)outMesh.m_vertex.size();
    outMesh.m_numIndex = (int)outMesh.m_index.size();

    if (outMesh.m_numVertex == 0 || outMesh.m_numIndex == 0) {
        ERROR(L"OBJParser", L"LoadOBJ", L"El OBJ no generó vértices/índices.");
        return false;
    }

    MESSAGE(L"OBJParser", L"LoadOBJ", L"Parsing OBJ finalizado.");
    return true;
}
