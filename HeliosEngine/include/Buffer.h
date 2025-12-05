#pragma once
#include "Prerequisites.h"
#include "MeshComponent.h"

class Device;
class DeviceContext;

/**
 * @class Buffer
 * @brief Wrapper (envoltorio) genérico para buffers de DirectX 11 (ID3D11Buffer).
 *
 * Esta clase gestiona la creación, actualización y vinculación (binding) de diferentes
 * tipos de buffers, incluyendo:
 * - Vertex Buffers (Datos de vértices)
 * - Index Buffers (Índices para dibujar primitivas)
 * - Constant Buffers (Variables uniformes para Shaders)
 */
class Buffer {
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
     * @brief Inicializa el buffer a partir de un MeshComponent (Vertex o Index Buffer).
     *
     * Utiliza los datos contenidos en el MeshComponent (vértices o índices) para crear
     * el buffer en la GPU.
     *
     * @param device Referencia al Device para crear el recurso.
     * @param mesh Referencia al componente de malla que contiene los datos.
     * @param bindFlag Bandera que indica el tipo (D3D11_BIND_VERTEX_BUFFER o D3D11_BIND_INDEX_BUFFER).
     * @return HRESULT S_OK si la creación fue exitosa.
     */
    HRESULT
        init(Device& device, const MeshComponent& mesh, unsigned int bindFlag);

    /**
     * @brief Inicializa un buffer de tamaño fijo (Constant Buffer).
     *
     * Generalmente utilizado para Constant Buffers donde el tamaño es conocido (sizeof(struct))
     * pero los datos cambian frecuentemente.
     *
     * @param device Referencia al Device.
     * @param ByteWidth Tamaño en bytes del buffer (debe ser múltiplo de 16 para Constant Buffers).
     * @return HRESULT S_OK si la creación fue exitosa.
     */
    HRESULT
        init(Device& device, unsigned int ByteWidth);

    /**
     * @brief Actualiza los datos dentro del buffer (Subresource Update).
     *
     * Envía nuevos datos desde la CPU a la memoria de la GPU. Esencial para actualizar
     * matrices, luces o posiciones en tiempo real.
     *
     * @param deviceContext Contexto del dispositivo para ejecutar la orden.
     * @param pDstResource Puntero al recurso destino (generalmente m_buffer).
     * @param DstSubresource Índice del subrecurso (usualmente 0).
     * @param pDstBox Caja opcional que define la porción a actualizar (nullptr para todo).
     * @param pSrcData Puntero a los nuevos datos en memoria CPU.
     * @param SrcRowPitch Paso de fila (para texturas, ignorar en buffers simples).
     * @param SrcDepthPitch Paso de profundidad (para texturas 3D, ignorar aquí).
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
     * @brief Vincula el buffer al pipeline de renderizado (Render Stage).
     *
     * Dependiendo del 'bindFlag' interno, esta función llamará a:
     * - IASetVertexBuffers (si es Vertex Buffer)
     * - IASetIndexBuffer (si es Index Buffer)
     * - VSSetConstantBuffers / PSSetConstantBuffers (si es Constant Buffer)
     *
     * @param deviceContext Contexto para realizar la vinculación.
     * @param StartSlot Slot de inicio (registro) donde se bindeará el buffer.
     * @param NumBuffers Número de buffers a bindear (usualmente 1).
     * @param setPixelShader Si es true y es un CB, se bindea al Pixel Shader en lugar del Vertex Shader.
     * @param format Formato de datos (necesario solo para Index Buffers, ej: DXGI_FORMAT_R32_UINT).
     */
    void
        render(DeviceContext& deviceContext,
            unsigned int   StartSlot,
            unsigned int   NumBuffers,
            bool           setPixelShader = false,
            DXGI_FORMAT    format = DXGI_FORMAT_UNKNOWN);

    /**
     * @brief Libera el recurso ID3D11Buffer y limpia la memoria.
     */
    void
        destroy();

    /**
     * @brief Función interna helper para llamar a D3D11CreateBuffer.
     *
     * @param device Referencia al wrapper del Device.
     * @param desc Descripción de la configuración del buffer.
     * @param initData Puntero a los datos iniciales (opcional).
     * @return HRESULT S_OK si se creó correctamente.
     */
    HRESULT
        createBuffer(Device& device,
            D3D11_BUFFER_DESC& desc,
            D3D11_SUBRESOURCE_DATA* initData);

private:
    /// Puntero nativo al buffer de DirectX 11.
    ID3D11Buffer* m_buffer = nullptr;

    /// Tamaño de un elemento individual (Stride). Importante para Vertex Buffers.
    unsigned int m_stride = 0;

    /// Desplazamiento desde el inicio del buffer (usualmente 0).
    unsigned int m_offset = 0;

    /// Almacena el tipo de buffer (D3D11_BIND_FLAG) para saber cómo renderizarlo después.
    unsigned int m_bindFlag = 0;
};