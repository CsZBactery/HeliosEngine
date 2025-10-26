#include "../include/ModelLoader.h"
#include "../include/Device.h"
#include "../include/DeviceContext.h"
#include <fstream>
#include <sstream>

struct Triple { int v = -1, t = -1, n = -1; };
static bool parseFaceToken(const std::string& tok, Triple& out)
{
  // Formats: v, v/t, v//n, v/t/n
  out = {};
  int slash1 = (int)tok.find('/');
  if (slash1 == -1) { out.v = std::stoi(tok); return true; }
  int slash2 = (int)tok.find('/', slash1 + 1);

  out.v = std::stoi(tok.substr(0, slash1));
  if (slash2 == -1) {
    out.t = std::stoi(tok.substr(slash1 + 1));
    return true;
  }
  if (slash2 == slash1 + 1) {
    out.n = std::stoi(tok.substr(slash2 + 1));
    return true;
  }
  out.t = std::stoi(tok.substr(slash1 + 1, slash2 - (slash1 + 1)));
  out.n = std::stoi(tok.substr(slash2 + 1));
  return true;
}

void Model::bind(DeviceContext& ctx, UINT slot, UINT stride, UINT offset)
{
  if (!m_vb || !m_ib) return;
  ctx.IASetVertexBuffers(slot, 1, &m_vb, &stride, &offset);
  ctx.IASetIndexBuffer(m_ib, DXGI_FORMAT_R32_UINT, 0);
}

void Model::draw(DeviceContext& ctx)
{
  if (!m_ib) return;
  ctx.DrawIndexed(m_indexCount, 0, 0);
}

void Model::destroy()
{
  if (m_vb) { m_vb->Release(); m_vb = nullptr; }
  if (m_ib) { m_ib->Release(); m_ib = nullptr; }
  m_indexCount = 0;
}

HRESULT ModelLoader::LoadOBJ(Device& device, const std::string& path, Model& outModel)
{
  std::ifstream in(path);
  if (!in.is_open()) return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);

  std::vector<DirectX::XMFLOAT3> positions;
  std::vector<DirectX::XMFLOAT3> normals;
  std::vector<DirectX::XMFLOAT2> uvs;

  std::vector<VertexPNCT> vertices;
  std::vector<uint32_t>   indices;
  std::unordered_map<std::string, uint32_t> map; // token "v/t/n" -> index

  std::string line;
  while (std::getline(in, line))
  {
    if (line.empty() || line[0] == '#') continue;
    std::istringstream ss(line);
    std::string id; ss >> id;

    if (id == "v") {
      float x, y, z; ss >> x >> y >> z;
      positions.emplace_back(x, y, z);
    }
    else if (id == "vt") {
      float u, v; ss >> u >> v;
      uvs.emplace_back(u, 1.0f - v); // invierte V (convención DX)
    }
    else if (id == "vn") {
      float x, y, z; ss >> x >> y >> z;
      normals.emplace_back(x, y, z);
    }
    else if (id == "f") {
      std::string t1, t2, t3, t4;
      ss >> t1 >> t2 >> t3 >> t4;
      std::string toks[4] = { t1,t2,t3,t4 };
      int faceVerts = t4.empty() ? 3 : 4;

      auto getIndex = [&](const std::string& key)->uint32_t {
        auto it = map.find(key);
        if (it != map.end()) return it->second;

        Triple tr;
        parseFaceToken(key, tr);
        VertexPNCT vtx{};
        if (tr.v != -1) vtx.pos = positions[(tr.v > 0 ? tr.v - 1 : (int)positions.size() + tr.v)];
        if (tr.t != -1 && !uvs.empty()) vtx.uv = uvs[(tr.t > 0 ? tr.t - 1 : (int)uvs.size() + tr.t)];
        if (tr.n != -1 && !normals.empty()) vtx.nrm = normals[(tr.n > 0 ? tr.n - 1 : (int)normals.size() + tr.n)];

        uint32_t idx = (uint32_t)vertices.size();
        vertices.push_back(vtx);
        map.emplace(key, idx);
        return idx;
        };

      uint32_t i0 = getIndex(t1);
      uint32_t i1 = getIndex(t2);
      uint32_t i2 = getIndex(t3);
      indices.push_back(i0); indices.push_back(i1); indices.push_back(i2);
      if (faceVerts == 4) {
        uint32_t i3 = getIndex(t4);
        indices.push_back(i0); indices.push_back(i2); indices.push_back(i3);
      }
    }
  }
  in.close();

  if (vertices.empty() || indices.empty()) return E_FAIL;

  // Crear VB
  D3D11_BUFFER_DESC vbDesc{};
  vbDesc.Usage = D3D11_USAGE_DEFAULT;
  vbDesc.ByteWidth = (UINT)(vertices.size() * sizeof(VertexPNCT));
  vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

  D3D11_SUBRESOURCE_DATA vbData{};
  vbData.pSysMem = vertices.data();

  ID3D11Buffer* vb = nullptr;
  HRESULT hr = device.CreateBuffer(&vbDesc, &vbData, &vb);
  if (FAILED(hr)) return hr;

  // Crear IB
  D3D11_BUFFER_DESC ibDesc{};
  ibDesc.Usage = D3D11_USAGE_DEFAULT;
  ibDesc.ByteWidth = (UINT)(indices.size() * sizeof(uint32_t));
  ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

  D3D11_SUBRESOURCE_DATA ibData{};
  ibData.pSysMem = indices.data();

  ID3D11Buffer* ib = nullptr;
  hr = device.CreateBuffer(&ibDesc, &ibData, &ib);
  if (FAILED(hr)) { vb->Release(); return hr; }

  outModel.destroy();
  outModel.m_vb = vb;
  outModel.m_ib = ib;
  outModel.m_indexCount = (UINT)indices.size();
  return S_OK;
}
