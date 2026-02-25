/**
 * @file Buffer.h
 * @brief Clase envoltorio (Wrapper) para gestionar recursos de Buffer en DirectX 11.
 */

#pragma once
#include "Prerequisites.h"
#include "MeshComponent.h"

class Device;
class DeviceContext;

/**
 * @class Buffer
 * @brief Wrapper genérico para buffers de DirectX 11 (ID3D11Buffer).
 *
 * @details Esta clase gestiona la creación, actualización y vinculación (binding) de diferentes
 * tipos de buffers en la memoria de la tarjeta gráfica (GPU), incluyendo:
 * - Vertex Buffers (Datos de vértices)
 * - Index Buffers (Índices para dibujar primitivas)
 * - Constant Buffers (Variables uniformes para Shaders)
 * * @note La instancia gestiona un solo ID3D11Buffer a la vez; el tipo efectivo se deduce de m_bindFlag.
 * @warning No realiza copias profundas automáticas del recurso COM de DirectX.
 */
class
    Buffer {
public:
    /**
     * @brief Constructor por defecto (no crea recursos en GPU).
     */
    Buffer() = default;

    /**
     * @brief Destructor por defecto.
     * @warning No libera automáticamente el recurso COM; se debe llamar explícitamente a destroy().
     */
    ~Buffer() = default;

    /**
     * @brief Inicializa el buffer a partir de un MeshComponent (Vertex o Index Buffer).
     *
     * @details Utiliza los datos contenidos en el MeshComponent (vértices o índices) para
     * configurar y crear el buffer en la GPU utilizando D3D11_BIND_VERTEX_BUFFER o D3D11_BIND_INDEX_BUFFER.
     *
     * @param device Referencia al Device para crear el recurso.
     * @param mesh Referencia al componente de malla que contiene los datos en RAM.
     * @param bindFlag Bandera que indica el rol del buffer.
     * @return S_OK si la creación fue exitosa; código HRESULT de error en caso contrario.
     * * @post Si retorna S_OK, m_buffer != nullptr y m_bindFlag se actualiza.
     */
    HRESULT
        init(Device& device, const MeshComponent& mesh, unsigned int bindFlag);

    /**
     * @brief Inicializa un buffer de tamaño fijo, típicamente un Constant Buffer.
     *
     * @details Generalmente utilizado para enviar variables a los Shaders donde el tamaño total
     * es conocido en tiempo de compilación mediante `sizeof(struct)` y se espera que cambie.
     *
     * @param device Referencia al Device.
     * @param ByteWidth Tamaño en bytes del buffer. Por regla de DirectX, debe ser múltiplo de 16 para Constant Buffers.
     * @return S_OK si la creación fue exitosa; código HRESULT de error en caso contrario.
     */
    HRESULT
        init(Device& device, unsigned int ByteWidth);

    /**
     * @brief Actualiza los datos dentro del buffer (equivalente a Subresource Update).
     *
     * @details Envía nuevos datos desde la CPU a la memoria de la GPU. Es una operación esencial
     * para actualizar matrices (Mundo, Vista, Proyección), luces o colores en tiempo real cada frame.
     *
     * @param deviceContext Contexto del dispositivo para ejecutar la orden de actualización.
     * @param pDstResource Puntero al recurso destino en GPU (generalmente m_buffer).
     * @param DstSubresource Índice del subrecurso (usualmente 0 para buffers simples).
     * @param pDstBox Caja opcional que define una porción específica a actualizar (nullptr para actualizar todo).
     * @param pSrcData Puntero a los nuevos datos residentes en memoria CPU.
     * @param SrcRowPitch Paso de fila (se ignora en buffers 1D).
     * @param SrcDepthPitch Paso de profundidad (se ignora en buffers 1D).
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
     * @brief Vincula el buffer a la etapa correspondiente del pipeline de renderizado.
     *
     * @details Dependiendo del valor almacenado en m_bindFlag, esta función inyecta el buffer en la GPU:
     * - D3D11_BIND_VERTEX_BUFFER: Llama a IASetVertexBuffers.
     * - D3D11_BIND_INDEX_BUFFER: Llama a IASetIndexBuffer.
     * - D3D11_BIND_CONSTANT_BUFFER: Llama a VSSetConstantBuffers y/u opcionalmente a PSSetConstantBuffers.
     *
     * @param deviceContext Contexto para realizar la vinculación.
     * @param StartSlot Registro/Slot de inicio donde se montará el buffer.
     * @param NumBuffers Número de buffers consecutivos a vincular (usualmente 1).
     * @param setPixelShader Si es true y el tipo es Constant Buffer, también lo vincula a la etapa Pixel Shader.
     * @param format Formato de datos (obligatorio para Index Buffers, ej: DXGI_FORMAT_R32_UINT).
     */
    void
        render(DeviceContext& deviceContext,
            unsigned int   StartSlot,
            unsigned int   NumBuffers,
            bool           setPixelShader = false,
            DXGI_FORMAT    format = DXGI_FORMAT_UNKNOWN);

    /**
     * @brief Libera el recurso ID3D11Buffer en la GPU y limpia la metadata interna.
     * @post m_buffer se vuelve nullptr, m_stride, m_offset y m_bindFlag se reinician a 0.
     */
    void
        destroy();

    /**
     * @brief Función interna (Helper) para ejecutar D3D11CreateBuffer.
     *
     * @param device Referencia al envoltorio del Device.
     * @param desc Descriptor con la configuración del buffer (tamaño, tipo, uso).
     * @param initData Puntero a los datos iniciales para inyectar al crearlo (puede ser nullptr).
     * @return S_OK si el hardware asignó la memoria correctamente.
     */
    HRESULT
        createBuffer(Device& device,
            D3D11_BUFFER_DESC& desc,
            D3D11_SUBRESOURCE_DATA* initData);

public:
    /**
     * @brief Puntero nativo al recurso Buffer de DirectX 11 administrado por esta clase.
     */
    ID3D11Buffer* m_buffer = nullptr;

private:
    /**
     * @brief Tamaño de un elemento individual en bytes (Stride).
     * @details Fundamental para Vertex Buffers (ej. sizeof(SimpleVertex)). Cero cuando no aplica.
     */
    unsigned int m_stride = 0;

    /**
     * @brief Desplazamiento inicial en bytes (Offset).
     * @details Usado al vincular Vertex Buffers para indicar desde dónde empezar a leer. Cero cuando no aplica.
     */
    unsigned int m_offset = 0;

    /**
     * @brief Bandera de enlace (D3D11_BIND_FLAG).
     * @details Define el rol del buffer en el pipeline de renderizado (Vértices, Índices o Constantes).
     */
    unsigned int m_bindFlag = 0;
};