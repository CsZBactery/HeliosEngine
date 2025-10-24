#pragma once
// ../include/
#include "../include/Prerequisites.h"
#include "../include/MeshComponent.h"

class Device;
class DeviceContext;

/**
 * @class Buffer
 * @brief Envoltura de ID3D11Buffer para vértices, índices o constantes.
 */
class Buffer {
public:
    Buffer() = default;
    ~Buffer() = default;

    // Prohíbe copia (evita dobles Release)
    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;

    /**
     * @brief Inicializa como Vertex o Index Buffer a partir de un MeshComponent.
     * @param device     Dispositivo.
     * @param mesh       Datos fuente.
     * @param bindFlag   D3D11_BIND_VERTEX_BUFFER o D3D11_BIND_INDEX_BUFFER.
     */
    HRESULT init(Device& device, const MeshComponent& mesh, unsigned int bindFlag);

    /**
     * @brief Inicializa como Constant Buffer (ByteWidth alineado a 16 recomendable).
     */
    HRESULT init(Device& device, unsigned int ByteWidth);

    /**
     * @brief Actualiza el contenido (UpdateSubresource).
     */
    void update(DeviceContext& deviceContext,
        ID3D11Resource* pDstResource,
        unsigned int    DstSubresource,
        const D3D11_BOX* pDstBox,
        const void* pSrcData,
        unsigned int    SrcRowPitch,
        unsigned int    SrcDepthPitch);

    /**
     * @brief Enlaza el buffer al pipeline según su tipo.
     * @param deviceContext
     * @param StartSlot
     * @param NumBuffers
     * @param setPixelShader  Si es CB, también lo enlaza a PS.
     * @param format          (Opcional) formato de índice. Si es UNKNOWN usa m_indexFormat.
     */
    void render(DeviceContext& deviceContext,
        unsigned int   StartSlot,
        unsigned int   NumBuffers,
        bool           setPixelShader = false,
        DXGI_FORMAT    format = DXGI_FORMAT_UNKNOWN);

    /// Libera el recurso y resetea metadatos
    void destroy();

    /// Helper para factorizar la creación
    HRESULT createBuffer(Device& device,
        D3D11_BUFFER_DESC& desc,
        D3D11_SUBRESOURCE_DATA* initData);

public:
    ID3D11Buffer* m_buffer = nullptr;      ///< recurso COM
    unsigned int  m_stride = 0;            ///< stride de vértice o tamaño de CB
    unsigned int  m_offset = 0;            ///< offset IA para VB/IB
    unsigned int  m_bindFlag = 0;          ///< D3D11_BIND_*
    DXGI_FORMAT   m_indexFormat = DXGI_FORMAT_UNKNOWN; ///< sólo para IB
};
