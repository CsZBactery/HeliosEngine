#include "../include/Buffer.h"
#include "../include/Device.h"
#include "../include/DeviceContext.h"

// ======================================================
// VB / IB desde MeshComponent
// ======================================================
HRESULT Buffer::init(Device& device, const MeshComponent& mesh, unsigned int bindFlag)
{
    destroy();

    if (!device.m_device)                 return E_POINTER;
    if (bindFlag != D3D11_BIND_VERTEX_BUFFER &&
        bindFlag != D3D11_BIND_INDEX_BUFFER) return E_INVALIDARG;

    D3D11_BUFFER_DESC        desc{};
    D3D11_SUBRESOURCE_DATA   data{};

    if (bindFlag == D3D11_BIND_VERTEX_BUFFER)
    {
        const UINT stride = mesh.vertexStride();
        const UINT count = mesh.vertexCount();
        const void* src = mesh.vertexData();
        if (stride == 0 || count == 0 || !src) return E_INVALIDARG;

        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        desc.ByteWidth = stride * count;
        data.pSysMem = src;

        m_stride = stride;
        m_offset = 0;
        m_indexFormat = DXGI_FORMAT_UNKNOWN;
        m_bindFlag = D3D11_BIND_VERTEX_BUFFER;
    }
    else // INDEX
    {
        const UINT istride = mesh.indexStride();
        const UINT icount = mesh.indexCount();
        const void* src = mesh.indexData();
        if (istride == 0 || icount == 0 || !src) return E_INVALIDARG;

        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        desc.ByteWidth = istride * icount;
        data.pSysMem = src;

        m_stride = 0;
        m_offset = 0;
        m_indexFormat = mesh.indexFormat(); // DXGI_FORMAT_R16_UINT / R32_UINT
        m_bindFlag = D3D11_BIND_INDEX_BUFFER;
    }

    return createBuffer(device, desc, &data);
}

// ======================================================
// Constant Buffer
// ======================================================
HRESULT Buffer::init(Device& device, unsigned int ByteWidth)
{
    destroy();

    if (!device.m_device)  return E_POINTER;
    if (ByteWidth == 0)    return E_INVALIDARG;

    // Alinear a 16 bytes como recomienda D3D11 para CBs
    ByteWidth = (ByteWidth + 15u) & ~15u;

    D3D11_BUFFER_DESC desc{};
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    desc.ByteWidth = ByteWidth;

    m_stride = 0;
    m_offset = 0;
    m_indexFormat = DXGI_FORMAT_UNKNOWN;
    m_bindFlag = D3D11_BIND_CONSTANT_BUFFER;

    return createBuffer(device, desc, nullptr);
}

// ======================================================
// Update
// ======================================================
void Buffer::update(DeviceContext& deviceContext,
    ID3D11Resource* pDstResource,
    unsigned int    DstSubresource,
    const D3D11_BOX* pDstBox,
    const void* pSrcData,
    unsigned int    SrcRowPitch,
    unsigned int    SrcDepthPitch)
{
    if (!pSrcData)   return;
    ID3D11Resource* dst = pDstResource ? pDstResource : reinterpret_cast<ID3D11Resource*>(m_buffer);
    if (!dst || !deviceContext.get()) return;

    deviceContext.UpdateSubresource(dst, DstSubresource, pDstBox,
        pSrcData, SrcRowPitch, SrcDepthPitch);
}

// ======================================================
// Bind
// ======================================================
void Buffer::render(DeviceContext& deviceContext,
    unsigned int   StartSlot,
    unsigned int   NumBuffers,
    bool           setPixelShader,
    DXGI_FORMAT    format)
{
    if (!m_buffer || !deviceContext.get()) return;

    switch (m_bindFlag)
    {
    case D3D11_BIND_VERTEX_BUFFER:
    {
        ID3D11Buffer* vb = m_buffer;
        const UINT stride = m_stride;
        const UINT offset = m_offset;
        deviceContext.IASetVertexBuffers(StartSlot, NumBuffers, &vb, &stride, &offset);
        break;
    }
    case D3D11_BIND_INDEX_BUFFER:
    {
        DXGI_FORMAT fmt = (format != DXGI_FORMAT_UNKNOWN) ? format : m_indexFormat;
        if (fmt == DXGI_FORMAT_UNKNOWN) fmt = DXGI_FORMAT_R32_UINT; // fallback seguro
        deviceContext.IASetIndexBuffer(m_buffer, fmt, 0);
        break;
    }
    case D3D11_BIND_CONSTANT_BUFFER:
    {
        ID3D11Buffer* cb = m_buffer;
        deviceContext.VSSetConstantBuffers(StartSlot, NumBuffers, &cb);
        if (setPixelShader) deviceContext.PSSetConstantBuffers(StartSlot, NumBuffers, &cb);
        break;
    }
    default:
        // tipo no soportado
        break;
    }
}

// ======================================================
// Destroy
// ======================================================
void Buffer::destroy()
{
    if (m_buffer) { m_buffer->Release(); m_buffer = nullptr; }
    m_stride = m_offset = 0;
    m_bindFlag = 0;
    m_indexFormat = DXGI_FORMAT_UNKNOWN;
}

// ======================================================
// Helper de creación
// ======================================================
HRESULT Buffer::createBuffer(Device& device,
    D3D11_BUFFER_DESC& desc,
    D3D11_SUBRESOURCE_DATA* initData)
{
    if (!device.m_device) return E_POINTER;

    ID3D11Buffer* buf = nullptr;
    HRESULT hr = device.CreateBuffer(&desc, initData, &buf);
    if (FAILED(hr)) return hr;

    m_buffer = buf; // tomamos propiedad
    return S_OK;
}
