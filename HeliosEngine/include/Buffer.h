#pragma once
#include "Prerequisites.h"
#include "MeshComponent.h"

class Device;
class DeviceContext;

/**
 * @class Buffer
 * @brief Encapsula un ID3D11Buffer (VB/IB/CB) con creación, actualización y enlace.
 */
class Buffer {
public:
    Buffer() = default;
    ~Buffer() = default;

    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;

    // Crear VB/IB desde MeshComponent
    HRESULT init(Device& device, const MeshComponent& mesh, unsigned int bindFlag);

    // Crear Constant Buffer (ByteWidth se alinea a 16)
    HRESULT init(Device& device, unsigned int ByteWidth);

    // Subir datos (UpdateSubresource)
    void update(DeviceContext& deviceContext,
        ID3D11Resource* pDstResource,
        unsigned int    DstSubresource,
        const D3D11_BOX* pDstBox,
        const void* pSrcData,
        unsigned int    SrcRowPitch,
        unsigned int    SrcDepthPitch);

    // Enlazar al pipeline (IA/VS/PS según tipo)
    void render(DeviceContext& deviceContext,
        unsigned int   StartSlot,
        unsigned int   NumBuffers,
        bool           setPixelShader = false,
        DXGI_FORMAT    format = DXGI_FORMAT_UNKNOWN);

    // Liberar
    void destroy();

    // Helper genérico de creación
    HRESULT createBuffer(Device& device,
        D3D11_BUFFER_DESC& desc,
        D3D11_SUBRESOURCE_DATA* initData);

    // Getters útiles
    ID3D11Buffer* get()          const { return m_buffer; }
    unsigned int  bindFlag()     const { return m_bindFlag; }
    unsigned int  stride()       const { return m_stride; }
    DXGI_FORMAT   indexFormat()  const { return m_indexFormat; }

private:
    ID3D11Buffer* m_buffer = nullptr;
    unsigned int  m_stride = 0;                    // VB
    unsigned int  m_offset = 0;                    // VB
    unsigned int  m_bindFlag = 0;                    // D3D11_BIND_*
    DXGI_FORMAT   m_indexFormat = DXGI_FORMAT_UNKNOWN;  // IB
};
