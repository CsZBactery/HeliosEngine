#pragma once
#include "Prerequisites.h"
#include "MeshComponent.h"

class Device;
class DeviceContext;

/**
 * @file Buffer.h
 * @brief Declaración del wrapper de buffers de Direct3D 11 (VB/IB/CB).
 *
 * Esta clase encapsula la creación, actualización, enlace (bind) y liberación
 * de buffers en GPU para vértices, índices y constantes.
 */

 /**
  * @class Buffer
  * @brief Encapsula un ID3D11Buffer y su modo de uso (vertex/index/constant).
  *
  * Responsabilidades:
  * - Crear buffers con datos iniciales (vértices/índices) o vacíos (constant buffer).
  * - Actualizar su contenido vía UpdateSubresource.
  * - Enlazar el buffer a la etapa correspondiente del pipeline (IA/VS/PS).
  * - Liberar el recurso cuando ya no se necesita.
  */
class
    Buffer {
public:
    /**
     * @brief Constructor por defecto.
     */
    Buffer() = default;

    /**
     * @brief Destructor por defecto.
     */
    ~Buffer() = default;

    /**
     * @brief Inicializa el buffer como VB o IB a partir de un MeshComponent.
     * @param device Dispositivo de D3D11.
     * @param mesh   Fuente de datos en CPU (vértices/índices).
     * @param bindFlag Banderas de enlace (D3D11_BIND_VERTEX_BUFFER o D3D11_BIND_INDEX_BUFFER).
     * @return S_OK en éxito o HRESULT de error.
     */
    HRESULT
        init(Device& device, const MeshComponent& mesh, unsigned int bindFlag);

    /**
     * @brief Inicializa un Constant Buffer (CB) con el tamaño indicado.
     * @param device Dispositivo de D3D11.
     * @param ByteWidth Tamaño en bytes del buffer.
     * @return S_OK en éxito o HRESULT de error.
     */
    HRESULT
        init(Device& device, unsigned int ByteWidth);

    /**
     * @brief Actualiza el contenido del buffer en GPU.
     * @param deviceContext Contexto inmediato de D3D11.
     * @param pDstResource Recurso destino (típicamente m_buffer).
     * @param DstSubresource Índice de subrecurso.
     * @param pDstBox Región a actualizar (o nullptr para copiar todo).
     * @param pSrcData Puntero a los datos en CPU.
     * @param SrcRowPitch Bytes por fila (texturas) o 0 para buffers.
     * @param SrcDepthPitch Bytes por capa (texturas) o 0 para buffers.
     */
    void
        update(DeviceContext& deviceContext,
            ID3D11Resource* pDstResource,
            unsigned int    DstSubresource,
            const D3D11_BOX* pDstBox,
            const void* pSrcData,
            unsigned int    SrcRowPitch,
            unsigned int    SrcDepthPitch);

    /**
     * @brief Enlaza el buffer al pipeline según su tipo.
     * @param deviceContext Contexto inmediato de D3D11.
     * @param StartSlot Slot inicial (VB/CB/PS).
     * @param NumBuffers Número de buffers a enlazar.
     * @param setPixelShader Si true y es CB, también lo enlaza a PS.
     * @param format Formato del índice (si es IB), por defecto UNKNOWN.
     */
    void
        render(DeviceContext& deviceContext,
            unsigned int   StartSlot,
            unsigned int   NumBuffers,
            bool           setPixelShader = false,
            DXGI_FORMAT    format = DXGI_FORMAT_UNKNOWN);

    /**
     * @brief Libera el recurso de GPU asociado al buffer.
     */
    void
        destroy();

    /**
     * @brief Crea internamente el ID3D11Buffer con la descripción y datos dados.
     * @param device Dispositivo de D3D11.
     * @param desc   Descripción del buffer.
     * @param initData Datos iniciales (puede ser nullptr).
     * @return S_OK en éxito o HRESULT de error.
     */
    HRESULT
        createBuffer(Device& device,
            D3D11_BUFFER_DESC& desc,
            D3D11_SUBRESOURCE_DATA* initData);

private:
    /** @brief Recurso de buffer en GPU. */
    ID3D11Buffer* m_buffer = nullptr;

    /** @brief Tamaño de un elemento (stride) para VB/IB o ByteWidth para CB. */
    unsigned int m_stride = 0;

    /** @brief Desplazamiento inicial al hacer bind (típicamente 0). */
    unsigned int m_offset = 0;

    /** @brief Bandera de enlace usada al crear el buffer (VB/IB/CB). */
    unsigned int m_bindFlag = 0;
};
