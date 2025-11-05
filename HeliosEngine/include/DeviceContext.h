#pragma once
#include "Prerequisites.h"

/**
 * @file DeviceContext.h
 * @brief Declaración de la clase DeviceContext que encapsula el ID3D11DeviceContext y
 * operaciones comunes del pipeline de DirectX 11 (IA, VS, PS, RS, OM).
 *
 * Proporciona métodos para fijar render targets, viewports, input layout,
 * buffers (vértices/índices/constantes), shaders, samplers, SRVs, estados
 * (rasterizer/blend) y para emitir draw calls indexadas.
 */

 /**
  * @brief Encapsula el contexto de dispositivo de DirectX 11.
  *
  * La clase DeviceContext se encarga de administrar los estados,
  * buffers, shaders y recursos asociados al pipeline de renderizado.
  */
class
    DeviceContext {
public:
    /**
     * @brief Constructor por defecto.
     */
    DeviceContext() = default;

    /**
     * @brief Destructor por defecto.
     */
    ~DeviceContext() = default;

    /**
     * @brief Inicializa el contexto de dispositivo.
     *
     * @note La implementación concreta depende de la aplicación.
     */
    void
        init();

    /**
     * @brief Actualiza el estado del contexto de dispositivo.
     *
     * @note Punto de extensión para lógica por frame si se requiere.
     */
    void
        update();

    /**
     * @brief Ejecuta las operaciones de renderizado con el contexto.
     *
     * @note No emite draw calls por sí mismo; se usa como hook.
     */
    void
        render();

    /**
     * @brief Libera los recursos asociados al contexto.
     *
     * Libera el objeto subyacente @c ID3D11DeviceContext.
     */
    void
        destroy();

    /**
     * @brief Establece los render targets y el depth-stencil en el pipeline (OM).
     *
     * @param NumViews Número de vistas de render a fijar.
     * @param ppRenderTargetViews Array de vistas de render target (tamaño @p NumViews).
     * @param pDepthStencilView Vista de profundidad/stencil (puede ser nullptr).
     */
    void
        OMSetRenderTargets(unsigned int NumViews,
            ID3D11RenderTargetView* const* ppRenderTargetViews,
            ID3D11DepthStencilView* pDepthStencilView);

    /**
     * @brief Define los viewports activos en el rasterizador (RS).
     *
     * @param NumViewports Número de viewports.
     * @param pViewports Array de viewports (tamaño @p NumViewports).
     */
    void
        RSSetViewports(unsigned int NumViewports,
            const D3D11_VIEWPORT* pViewports);

    /**
     * @brief Establece el Input Layout para la etapa de ensamblado de entrada (IA).
     *
     * @param pInputLayout Puntero al input layout válido.
     */
    void
        IASetInputLayout(ID3D11InputLayout* pInputLayout);

    /**
     * @brief Asigna buffers de vértices al pipeline (IA).
     *
     * @param StartSlot Slot inicial.
     * @param NumBuffers Número de buffers.
     * @param ppVertexBuffers Array de buffers de vértices.
     * @param pStrides Array con el tamaño (stride) de cada vértice por buffer.
     * @param pOffsets Array con los desplazamientos iniciales por buffer.
     */
    void
        IASetVertexBuffers(unsigned int StartSlot,
            unsigned int NumBuffers,
            ID3D11Buffer* const* ppVertexBuffers,
            const unsigned int* pStrides,
            const unsigned int* pOffsets);

    /**
     * @brief Asigna un buffer de índices al pipeline (IA).
     *
     * @param pIndexBuffer Buffer de índices.
     * @param Format Formato de los índices (p. ej., @c DXGI_FORMAT_R32_UINT).
     * @param Offset Desplazamiento inicial en bytes.
     */
    void
        IASetIndexBuffer(ID3D11Buffer* pIndexBuffer,
            DXGI_FORMAT Format,
            unsigned int Offset);

    /**
     * @brief Establece la topología de primitivas para el ensamblador de entrada (IA).
     *
     * @param Topology Tipo de primitiva (p. ej., @c D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST).
     */
    void
        IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY Topology);

    /**
     * @brief Copia datos desde CPU a un recurso de GPU.
     *
     * @param pDstResource Recurso de destino.
     * @param DstSubresource Subrecurso de destino.
     * @param pDstBox Región a actualizar (puede ser nullptr para todo el recurso).
     * @param pSrcData Puntero a los datos de origen.
     * @param SrcRowPitch Número de bytes por fila de datos.
     * @param SrcDepthPitch Número de bytes por capa de datos (texturas 3D).
     *
     * @see ID3D11DeviceContext::UpdateSubresource
     */
    void
        UpdateSubresource(ID3D11Resource* pDstResource,
            unsigned int DstSubresource,
            const D3D11_BOX* pDstBox,
            const void* pSrcData,
            unsigned int SrcRowPitch,
            unsigned int SrcDepthPitch);

    /**
     * @brief Limpia un render target con un color específico (OM).
     *
     * @param pRenderTargetView Vista del render target a limpiar.
     * @param ColorRGBA Array de 4 floats con el color RGBA.
     */
    void
        ClearRenderTargetView(ID3D11RenderTargetView* pRenderTargetView,
            const float ColorRGBA[4]);

    /**
     * @brief Limpia un buffer de profundidad y/o stencil (OM).
     *
     * @param pDepthStencilView Vista de profundidad/stencil.
     * @param ClearFlags Banderas de limpieza (p. ej., @c D3D11_CLEAR_DEPTH | @c D3D11_CLEAR_STENCIL).
     * @param Depth Valor de profundidad inicial (0..1).
     * @param Stencil Valor inicial del stencil.
     */
    void
        ClearDepthStencilView(ID3D11DepthStencilView* pDepthStencilView,
            unsigned int ClearFlags,
            FLOAT Depth,
            UINT8 Stencil);

    /**
     * @brief Asigna un shader de vértices al pipeline (VS).
     *
     * @param pVertexShader Shader de vértices.
     * @param ppClassInstances Array de instancias de clase (opcional).
     * @param NumClassInstances Número de instancias de clase.
     */
    void
        VSSetShader(ID3D11VertexShader* pVertexShader,
            ID3D11ClassInstance* const* ppClassInstances,
            UINT NumClassInstances);

    /**
     * @brief Asigna buffers constantes a la etapa de vertex shader (VS).
     *
     * @param StartSlot Slot inicial.
     * @param NumBuffers Número de buffers.
     * @param ppConstantBuffers Array de buffers constantes.
     */
    void
        VSSetConstantBuffers(UINT StartSlot,
            UINT NumBuffers,
            ID3D11Buffer* const* ppConstantBuffers);

    /**
     * @brief Asigna un shader de píxeles al pipeline (PS).
     *
     * @param pPixelShader Shader de píxeles.
     * @param ppClassInstances Array de instancias de clase (opcional).
     * @param NumClassInstances Número de instancias de clase.
     */
    void
        PSSetShader(ID3D11PixelShader* pPixelShader,
            ID3D11ClassInstance* const* ppClassInstances,
            UINT NumClassInstances);

    /**
     * @brief Asigna buffers constantes a la etapa de pixel shader (PS).
     *
     * @param StartSlot Slot inicial.
     * @param NumBuffers Número de buffers.
     * @param ppConstantBuffers Array de buffers constantes.
     */
    void
        PSSetConstantBuffers(UINT StartSlot,
            UINT NumBuffers,
            ID3D11Buffer* const* ppConstantBuffers);

    /**
     * @brief Asigna recursos de textura a la etapa de pixel shader (PS).
     *
     * @param StartSlot Slot inicial.
     * @param NumViews Número de vistas (SRVs).
     * @param ppShaderResourceViews Array de vistas de recursos.
     */
    void
        PSSetShaderResources(UINT StartSlot,
            UINT NumViews,
            ID3D11ShaderResourceView* const* ppShaderResourceViews);

    /**
     * @brief Asigna estados de muestreo a la etapa de pixel shader (PS).
     *
     * @param StartSlot Slot inicial.
     * @param NumSamplers Número de samplers.
     * @param ppSamplers Array de estados de muestreo.
     */
    void
        PSSetSamplers(UINT StartSlot,
            UINT NumSamplers,
            ID3D11SamplerState* const* ppSamplers);

    /**
     * @brief Dibuja primitivas indexadas.
     *
     * @param IndexCount Número de índices a dibujar.
     * @param StartIndexLocation Índice inicial.
     * @param BaseVertexLocation Desplazamiento base de vértices.
     *
     * @see ID3D11DeviceContext::DrawIndexed
     */
    void
        DrawIndexed(UINT IndexCount,
            UINT StartIndexLocation,
            INT BaseVertexLocation);

    /**
     * @brief Establece el estado del rasterizador (RS).
     *
     * @param pRasterizerState Estado del rasterizador.
     */
    void
        RSSetState(ID3D11RasterizerState* pRasterizerState);

    /**
     * @brief Establece el estado de blending para el pipeline (OM).
     *
     * @param pBlendState Estado de blending.
     * @param BlendFactor Factores de mezcla RGBA (4 floats).
     * @param SampleMask Máscara de muestras.
     */
    void
        OMSetBlendState(ID3D11BlendState* pBlendState,
            const float BlendFactor[4],
            unsigned int SampleMask);

public:
    /** @brief Puntero al contexto de dispositivo de DirectX subyacente. */
    ID3D11DeviceContext* m_deviceContext = nullptr; /**< Puntero al contexto de dispositivo de DirectX. */
};
