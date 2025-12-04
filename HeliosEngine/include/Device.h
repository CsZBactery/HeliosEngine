#pragma once
#include "Prerequisites.h"

/**
 * @brief Encapsula el dispositivo de DirectX 11.
 *
 * La clase Device se encarga de inicializar, actualizar, renderizar
 * y destruir el dispositivo, as� como de crear recursos gr�ficos
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
     */
    void
        init();

    /**
     * @brief Actualiza el estado interno del dispositivo.
     */
    void
        update();

    /**
     * @brief Realiza las operaciones de renderizado con el dispositivo.
     */
    void
        render();

    /**
     * @brief Libera los recursos asociados al dispositivo.
     */
    void
        destroy();

    /**
     * @brief Crea una vista de render target.
     *
     * @param pResource Recurso de DirectX (ejemplo: textura).
     * @param pDesc Descriptor de la vista del render target.
     * @param ppRTView Puntero doble que recibe la vista creada.
     * @return HRESULT C�digo de estado de la operaci�n.
     */
    HRESULT
        CreateRenderTargetView(ID3D11Resource* pResource,
            const D3D11_RENDER_TARGET_VIEW_DESC* pDesc,
            ID3D11RenderTargetView** ppRTView);

    /**
     * @brief Crea una textura 2D.
     *
     * @param pDesc Descriptor de la textura.
     * @param pInitialData Datos iniciales para poblar la textura.
     * @param ppTexture2D Puntero doble que recibe la textura creada.
     * @return HRESULT C�digo de estado de la operaci�n.
     */
    HRESULT
        CreateTexture2D(const D3D11_TEXTURE2D_DESC* pDesc,
            const D3D11_SUBRESOURCE_DATA* pInitialData,
            ID3D11Texture2D** ppTexture2D);

    /**
     * @brief Crea una vista de profundidad y stencil.
     *
     * @param pResource Recurso de DirectX (ejemplo: textura de profundidad).
     * @param pDesc Descriptor de la vista de profundidad/stencil.
     * @param ppDepthStencilView Puntero doble que recibe la vista creada.
     * @return HRESULT C�digo de estado de la operaci�n.
     */
    HRESULT
        CreateDepthStencilView(ID3D11Resource* pResource,
            const D3D11_DEPTH_STENCIL_VIEW_DESC* pDesc,
            ID3D11DepthStencilView** ppDepthStencilView);

    /**
     * @brief Crea un shader de v�rtices.
     *
     * @param pShaderBytecode C�digo compilado del shader.
     * @param BytecodeLength Tama�o en bytes del c�digo compilado.
     * @param pClassLinkage Objeto de enlace de clases (opcional).
     * @param ppVertexShader Puntero doble que recibe el shader creado.
     * @return HRESULT C�digo de estado de la operaci�n.
     */
    HRESULT
        CreateVertexShader(const void* pShaderBytecode,
            unsigned int BytecodeLength, //unsigned
            ID3D11ClassLinkage* pClassLinkage,
            ID3D11VertexShader** ppVertexShader);

    /**
     * @brief Crea un dise�o de entrada (Input Layout).
     *
     * @param pInputElementDescs Array de descriptores de elementos de entrada.
     * @param NumElements N�mero de elementos en el array.
     * @param pShaderBytecodeWithInputSignature Firma de entrada del shader.
     * @param BytecodeLength Tama�o en bytes del c�digo compilado.
     * @param ppInputLayout Puntero doble que recibe el layout creado.
     * @return HRESULT C�digo de estado de la operaci�n.
     */
    HRESULT
        CreateInputLayout(const D3D11_INPUT_ELEMENT_DESC* pInputElementDescs,
            UINT NumElements,
            const void* pShaderBytecodeWithInputSignature,
            unsigned int BytecodeLength, //unsigned
            ID3D11InputLayout** ppInputLayout);

    /**
     * @brief Crea un shader de p�xeles.
     *
     * @param pShaderBytecode C�digo compilado del shader.
     * @param BytecodeLength Tama�o en bytes del c�digo compilado.
     * @param pClassLinkage Objeto de enlace de clases (opcional).
     * @param ppPixelShader Puntero doble que recibe el shader creado.
     * @return HRESULT C�digo de estado de la operaci�n.
     */
    HRESULT
        CreatePixelShader(const void* pShaderBytecode,
            unsigned int BytecodeLength, //unsigned
            ID3D11ClassLinkage* pClassLinkage,
            ID3D11PixelShader** ppPixelShader);

    /**
     * @brief Crea un buffer gen�rico.
     *
     * @param pDesc Descriptor del buffer.
     * @param pInitialData Datos iniciales para poblar el buffer.
     * @param ppBuffer Puntero doble que recibe el buffer creado.
     * @return HRESULT C�digo de estado de la operaci�n.
     */
    HRESULT
        CreateBuffer(const D3D11_BUFFER_DESC* pDesc,
            const D3D11_SUBRESOURCE_DATA* pInitialData,
            ID3D11Buffer** ppBuffer);

    /**
     * @brief Crea un estado de muestreo (Sampler State).
     *
     * @param pSamplerDesc Descriptor del sampler.
     * @param ppSamplerState Puntero doble que recibe el estado de muestreo creado.
     * @return HRESULT C�digo de estado de la operaci�n.
     */
    HRESULT
        CreateSamplerState(const D3D11_SAMPLER_DESC* pSamplerDesc,
            ID3D11SamplerState** ppSamplerState);

public:
    ID3D11Device* m_device = nullptr; /**< Puntero al dispositivo de DirectX. */
};