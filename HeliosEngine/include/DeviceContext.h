#pragma once
#include "Prerequisites.h"

/**
 * @class DeviceContext
 * @brief Encapsula la interfaz ID3D11DeviceContext de DirectX 11.
 *
 * Esta clase actúa como el "Pintor" o el "Director de Orquesta" del pipeline gráfico.
 * Mientras que la clase 'Device' crea los recursos (memoria), el 'DeviceContext'
 * se encarga de usarlos: enlaza buffers, asigna shaders, actualiza variables
 * y emite los comandos de dibujo (Draw Calls).
 */
class DeviceContext {
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
     * @brief Inicializa el contexto de dispositivo (generalmente vacío o reservado).
     */
    void
        init();

    /**
     * @brief Actualiza el estado del contexto (si fuera necesario por frame).
     */
    void
        update();

    /**
     * @brief Ejecuta operaciones generales de renderizado.
     */
    void
        render();

    /**
     * @brief Libera la interfaz ID3D11DeviceContext y limpia la memoria.
     */
    void
        destroy();

    /**
     * @brief [OM Stage] Establece dónde se dibujarán los píxeles.
     *
     * Configura la Etapa de Fusión de Salida (Output Merger). Define en qué texturas
     * se guardará el color (RenderTarget) y la profundidad (DepthStencil).
     *
     * @param NumViews Número de vistas de render target a enlazar.
     * @param ppRenderTargetViews Array de punteros a las vistas de render target.
     * @param pDepthStencilView Puntero a la vista de profundidad/stencil (Z-Buffer).
     */
    void
        OMSetRenderTargets(unsigned int NumViews,
            ID3D11RenderTargetView* const* ppRenderTargetViews,
            ID3D11DepthStencilView* pDepthStencilView);

    /**
     * @brief [RS Stage] Define el área de la ventana donde se dibujará.
     *
     * Configura la Etapa del Rasterizador. Mapea las coordenadas normalizadas
     * del dispositivo a píxeles de la pantalla.
     *
     * @param NumViewports Número de viewports.
     * @param pViewports Array de estructuras de configuración de viewport.
     */
    void
        RSSetViewports(unsigned int NumViewports,
            const D3D11_VIEWPORT* pViewports);

    /**
     * @brief [IA Stage] Define cómo leer los vértices de la memoria.
     *
     * Configura el Input Assembler. Le dice a la GPU qué formato tienen los datos
     * (ej: Posición (float3) + Color (float4)).
     *
     * @param pInputLayout Puntero al objeto Input Layout creado previamente.
     */
    void
        IASetInputLayout(ID3D11InputLayout* pInputLayout);

    /**
     * @brief [IA Stage] Enlaza los buffers de geometría (Vértices).
     *
     * @param StartSlot Slot de entrada (normalmente 0).
     * @param NumBuffers Número de buffers a enlazar.
     * @param ppVertexBuffers Array de buffers de vértices.
     * @param pStrides Array con el tamaño en bytes de un solo vértice (Estructura).
     * @param pOffsets Array con el desplazamiento inicial en bytes.
     */
    void
        IASetVertexBuffers(unsigned int StartSlot,
            unsigned int NumBuffers,
            ID3D11Buffer* const* ppVertexBuffers,
            const unsigned int* pStrides,
            const unsigned int* pOffsets);

    /**
     * @brief [IA Stage] Enlaza el buffer de índices.
     *
     * Permite reutilizar vértices mediante indexación.
     *
     * @param pIndexBuffer Puntero al buffer de índices.
     * @param Format Formato de los índices (DXGI_FORMAT_R32_UINT o R16_UINT).
     * @param Offset Desplazamiento inicial en bytes.
     */
    void
        IASetIndexBuffer(ID3D11Buffer* pIndexBuffer,
            DXGI_FORMAT Format,
            unsigned int Offset);

    /**
     * @brief [IA Stage] Define cómo interpretar los vértices (Topología).
     *
     * @param Topology Tipo de primitiva: Lista de Triángulos, Tira de Triángulos, Líneas, Puntos.
     */
    void
        IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY Topology);

    /**
     * @brief Actualiza datos en la GPU desde la CPU.
     *
     * Fundamental para animaciones o cambios de estado. Copia datos de memoria del sistema
     * a un recurso (Buffer o Textura) en la tarjeta gráfica.
     *
     * @param pDstResource Recurso de destino en GPU.
     * @param DstSubresource Índice del subrecurso (0 si no hay mipmaps/arrays).
     * @param pDstBox Caja que define la región a actualizar (nullptr para todo).
     * @param pSrcData Puntero a los datos en CPU.
     * @param SrcRowPitch Ancho de fila en bytes (importante para texturas).
     * @param SrcDepthPitch Ancho de profundidad en bytes (para texturas 3D).
     */
    void
        UpdateSubresource(ID3D11Resource* pDstResource,
            unsigned int DstSubresource,
            const D3D11_BOX* pDstBox,
            const void* pSrcData,
            unsigned int SrcRowPitch,
            unsigned int SrcDepthPitch);

    /**
     * @brief Limpia el lienzo (Pantalla) con un color sólido.
     *
     * @param pRenderTargetView Vista del objetivo a limpiar.
     * @param ColorRGBA Array de 4 floats [R, G, B, A] con el color de fondo.
     */
    void
        ClearRenderTargetView(ID3D11RenderTargetView* pRenderTargetView,
            const float ColorRGBA[4]);

    /**
     * @brief Limpia el buffer de profundidad y/o stencil.
     *
     * Necesario al inicio de cada frame para resetear la información de oclusión.
     *
     * @param pDepthStencilView Vista del buffer de profundidad.
     * @param ClearFlags Qué limpiar (D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL).
     * @param Depth Valor de profundidad por defecto (usualmente 1.0f = lo más lejos).
     * @param Stencil Valor de stencil por defecto.
     */
    void
        ClearDepthStencilView(ID3D11DepthStencilView* pDepthStencilView,
            unsigned int ClearFlags,
            FLOAT Depth,
            UINT8 Stencil);

    /**
     * @brief [VS Stage] Activa un Vertex Shader.
     *
     * @param pVertexShader Puntero al Vertex Shader.
     * @param ppClassInstances Instancias de enlace dinámico (usualmente nullptr).
     * @param NumClassInstances Número de instancias.
     */
    void
        VSSetShader(ID3D11VertexShader* pVertexShader,
            ID3D11ClassInstance* const* ppClassInstances,
            UINT NumClassInstances);

    /**
     * @brief [VS Stage] Envía variables uniformes al Vertex Shader.
     *
     * Se usa para pasar matrices de transformación (World, View, Projection).
     *
     * @param StartSlot Slot del registro (b0, b1, etc.).
     * @param NumBuffers Cantidad de buffers.
     * @param ppConstantBuffers Array de buffers constantes.
     */
    void
        VSSetConstantBuffers(UINT StartSlot,
            UINT NumBuffers,
            ID3D11Buffer* const* ppConstantBuffers);

    /**
     * @brief [PS Stage] Activa un Pixel Shader.
     *
     * @param pPixelShader Puntero al Pixel Shader.
     * @param ppClassInstances Instancias de enlace dinámico.
     * @param NumClassInstances Número de instancias.
     */
    void
        PSSetShader(ID3D11PixelShader* pPixelShader,
            ID3D11ClassInstance* const* ppClassInstances,
            UINT NumClassInstances);

    /**
     * @brief [PS Stage] Envía variables uniformes al Pixel Shader.
     *
     * Se usa para pasar colores, propiedades de material, posición de luces, etc.
     *
     * @param StartSlot Slot del registro.
     * @param NumBuffers Cantidad de buffers.
     * @param ppConstantBuffers Array de buffers constantes.
     */
    void
        PSSetConstantBuffers(UINT StartSlot,
            UINT NumBuffers,
            ID3D11Buffer* const* ppConstantBuffers);

    /**
     * @brief [PS Stage] Enlaza TEXTURAS al Pixel Shader.
     *
     * Asigna Shader Resource Views (SRV) para que el shader pueda leer texturas.
     *
     * @param StartSlot Slot de textura (t0, t1, etc.).
     * @param NumViews Número de texturas.
     * @param ppShaderResourceViews Array de vistas de recursos (texturas).
     */
    void
        PSSetShaderResources(UINT StartSlot,
            UINT NumViews,
            ID3D11ShaderResourceView* const* ppShaderResourceViews);

    /**
     * @brief [PS Stage] Enlaza SAMPLERS al Pixel Shader.
     *
     * Define cómo se filtran las texturas enlazadas anteriormente.
     *
     * @param StartSlot Slot del sampler (s0, s1, etc.).
     * @param NumSamplers Número de samplers.
     * @param ppSamplers Array de estados de muestreo.
     */
    void
        PSSetSamplers(UINT StartSlot,
            UINT NumSamplers,
            ID3D11SamplerState* const* ppSamplers);

    /**
     * @brief Ejecuta el comando de dibujo (Draw Call) usando índices.
     *
     * Es la función que realmente "dibuja" la geometría configurada en pantalla.
     *
     * @param IndexCount Cantidad de índices a dibujar.
     * @param StartIndexLocation Posición del primer índice a leer.
     * @param BaseVertexLocation Valor sumado a cada índice antes de leer el vértice.
     */
    void
        DrawIndexed(UINT IndexCount,
            UINT StartIndexLocation,
            INT BaseVertexLocation);

    /**
     * @brief [RS Stage] Configura estados fijos del Rasterizador.
     *
     * Controla el Culling (Back/Front face), Wireframe vs Solid, etc.
     *
     * @param pRasterizerState Puntero al estado del rasterizador.
     */
    void
        RSSetState(ID3D11RasterizerState* pRasterizerState);

    /**
     * @brief [OM Stage] Configura la mezcla de colores (Transparencia).
     *
     * Define cómo se combina el píxel nuevo con el que ya existe en el buffer.
     *
     * @param pBlendState Estado de blending.
     * @param BlendFactor Factor de mezcla manual (para ciertos modos de blend).
     * @param SampleMask Máscara de bits para muestras (MSAA).
     */
    void
        OMSetBlendState(ID3D11BlendState* pBlendState,
            const float BlendFactor[4],
            unsigned int SampleMask);

public:
    /// Puntero nativo a la interfaz de contexto de DirectX 11.
    ID3D11DeviceContext* m_deviceContext = nullptr;
};