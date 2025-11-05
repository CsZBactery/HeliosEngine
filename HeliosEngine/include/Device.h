#pragma once
#include "Prerequisites.h"

/**
 * @file Device.h
 * @brief Declaración de la clase Device que encapsula el ID3D11Device y
 * utilidades de creación de recursos en DirectX 11.
 *
 * Proporciona métodos para crear vistas (RTV/DSV), texturas, shaders, buffers
 * y estados de muestreo, además de las rutinas de ciclo de vida básicas.
 */

 /**
  * @brief Encapsula el dispositivo de DirectX 11.
  *
  * La clase Device se encarga de inicializar, actualizar, renderizar
  * y destruir el dispositivo, así como de crear recursos gráficos
  * fundamentales como shaders, buffers, texturas y estados.
  */
class
    Device {
public:
    /**
     * @brief Constructor por defecto.
     */
    Device() = default;

    /**
     * @brief Destructor por defecto.
     */
    ~Device() = default;

    /**
     * @brief Inicializa el dispositivo de DirectX.
     *
     * @note La implementación concreta depende de la aplicación.
     */
    void
        init();

    /**
     * @brief Actualiza el estado interno del dispositivo.
     *
     * @param None Sin parámetros.
     * @note Método de hook para lógica por-frame si se requiere.
     */
    void
        update();

    /**
     * @brief Realiza las operaciones de renderizado con el dispositivo.
     *
     * @note Este método no crea draw calls por sí mismo; es un punto de extensión.
     */
    void
        render();

    /**
     * @brief Libera los recursos asociados al dispositivo.
     *
     * Libera el objeto subyacente @c ID3D11Device.
     */
    void
        destroy();

    /**
     * @brief Crea una vista de render target (RTV).
     *
     * @param pResource Recurso de DirectX (por ejemplo, una textura).
     * @param pDesc Descriptor de la vista del render target (puede ser nullptr para vista por defecto).
     * @param ppRTView Puntero de salida que recibe la vista creada.
     * @return HRESULT Código de estado de la operación.
     * @see ID3D11Device::CreateRenderTargetView
     */
    HRESULT
        CreateRenderTargetView(ID3D11Resource* pResource,
            const D3D11_RENDER_TARGET_VIEW_DESC* pDesc,
            ID3D11RenderTargetView** ppRTView);

    /**
     * @brief Crea una textura 2D.
     *
     * @param pDesc Descriptor de la textura.
     * @param pInitialData Datos iniciales para poblarla (puede ser nullptr).
     * @param ppTexture2D Puntero de salida que recibe la textura creada.
     * @return HRESULT Código de estado de la operación.
     * @see ID3D11Device::CreateTexture2D
     */
    HRESULT
        CreateTexture2D(const D3D11_TEXTURE2D_DESC* pDesc,
            const D3D11_SUBRESOURCE_DATA* pInitialData,
            ID3D11Texture2D** ppTexture2D);

    /**
     * @brief Crea una vista de profundidad y stencil (DSV).
     *
     * @param pResource Recurso de DirectX (p. ej., textura de depth/stencil).
     * @param pDesc Descriptor de la vista de profundidad/stencil (puede ser nullptr para vista por defecto).
     * @param ppDepthStencilView Puntero de salida que recibe la vista creada.
     * @return HRESULT Código de estado de la operación.
     * @see ID3D11Device::CreateDepthStencilView
     */
    HRESULT
        CreateDepthStencilView(ID3D11Resource* pResource,
            const D3D11_DEPTH_STENCIL_VIEW_DESC* pDesc,
            ID3D11DepthStencilView** ppDepthStencilView);

    /**
     * @brief Crea un shader de vértices (VS).
     *
     * @param pShaderBytecode Código compilado del shader (blob).
     * @param BytecodeLength Tamaño en bytes del código compilado.
     * @param pClassLinkage Objeto de enlace de clases (opcional, puede ser nullptr).
     * @param ppVertexShader Puntero de salida que recibe el shader creado.
     * @return HRESULT Código de estado de la operación.
     * @see ID3D11Device::CreateVertexShader
     */
    HRESULT
        CreateVertexShader(const void* pShaderBytecode,
            unsigned int BytecodeLength, //unsigned
            ID3D11ClassLinkage* pClassLinkage,
            ID3D11VertexShader** ppVertexShader);

    /**
     * @brief Crea un diseño de entrada (Input Layout).
     *
     * @param pInputElementDescs Array de descriptores de elementos de entrada.
     * @param NumElements Número de elementos en el array.
     * @param pShaderBytecodeWithInputSignature Firma de entrada del shader (bytecode VS).
     * @param BytecodeLength Tamaño en bytes del código compilado.
     * @param ppInputLayout Puntero de salida que recibe el layout creado.
     * @return HRESULT Código de estado de la operación.
     * @see ID3D11Device::CreateInputLayout
     */
    HRESULT
        CreateInputLayout(const D3D11_INPUT_ELEMENT_DESC* pInputElementDescs,
            UINT NumElements,
            const void* pShaderBytecodeWithInputSignature,
            unsigned int BytecodeLength, //unsigned
            ID3D11InputLayout** ppInputLayout);

    /**
     * @brief Crea un shader de píxeles (PS).
     *
     * @param pShaderBytecode Código compilado del shader (blob).
     * @param BytecodeLength Tamaño en bytes del código compilado.
     * @param pClassLinkage Objeto de enlace de clases (opcional, puede ser nullptr).
     * @param ppPixelShader Puntero de salida que recibe el shader creado.
     * @return HRESULT Código de estado de la operación.
     * @see ID3D11Device::CreatePixelShader
     */
    HRESULT
        CreatePixelShader(const void* pShaderBytecode,
            unsigned int BytecodeLength, //unsigned
            ID3D11ClassLinkage* pClassLinkage,
            ID3D11PixelShader** ppPixelShader);

    /**
     * @brief Crea un buffer genérico (VB/IB/CB).
     *
     * @param pDesc Descriptor del buffer.
     * @param pInitialData Datos iniciales (puede ser nullptr).
     * @param ppBuffer Puntero de salida que recibe el buffer creado.
     * @return HRESULT Código de estado de la operación.
     * @see ID3D11Device::CreateBuffer
     */
    HRESULT
        CreateBuffer(const D3D11_BUFFER_DESC* pDesc,
            const D3D11_SUBRESOURCE_DATA* pInitialData,
            ID3D11Buffer** ppBuffer);

    /**
     * @brief Crea un estado de muestreo (Sampler State).
     *
     * @param pSamplerDesc Descriptor del sampler.
     * @param ppSamplerState Puntero de salida que recibe el estado creado.
     * @return HRESULT Código de estado de la operación.
     * @see ID3D11Device::CreateSamplerState
     */
    HRESULT
        CreateSamplerState(const D3D11_SAMPLER_DESC* pSamplerDesc,
            ID3D11SamplerState** ppSamplerState);

public:
    /** @brief Puntero al dispositivo de DirectX subyacente. */
    ID3D11Device* m_device = nullptr; /**< Puntero al dispositivo de DirectX. */
};
