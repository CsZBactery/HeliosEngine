#pragma once
#include "Prerequisites.h"

/**
 * @class Device
 * @brief Encapsula la interfaz ID3D11Device de DirectX 11.
 *
 * La clase Device actúa como una "Fábrica" de recursos. Es la responsable de
 * reservar memoria en la tarjeta gráfica para crear texturas, shaders, buffers
 * y vistas.
 *
 * A diferencia del DeviceContext (que se usa para renderizar), el Device
 * se usa principalmente durante la inicialización (Init).
 */
class Device {
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
     * @brief Inicializa el dispositivo y la cadena de intercambio (si corresponde).
     *
     * Crea el objeto ID3D11Device y ID3D11DeviceContext nativos.
     */
    void
        init();

    /**
     * @brief Actualiza el estado del dispositivo.
     *
     * Generalmente no requiere lógica por frame, pero se mantiene por estructura del motor.
     */
    void
        update();

    /**
     * @brief Renderiza o presenta información del dispositivo.
     */
    void
        render();

    /**
     * @brief Libera la memoria del dispositivo y destruye los punteros COM.
     */
    void
        destroy();

    /**
     * @brief Crea una Vista de Render Target (RTV).
     *
     * Permite usar una textura (como el BackBuffer) como destino de dibujo.
     *
     * @param pResource Puntero al recurso (Textura) sobre el que se creará la vista.
     * @param pDesc Descripción de la configuración de la vista (nullptr para defaults).
     * @param ppRTView Puntero de salida donde se guardará la dirección de la nueva vista.
     * @return HRESULT S_OK si la creación fue exitosa.
     */
    HRESULT
        CreateRenderTargetView(ID3D11Resource* pResource,
            const D3D11_RENDER_TARGET_VIEW_DESC* pDesc,
            ID3D11RenderTargetView** ppRTView);

    /**
     * @brief Crea una Textura 2D en la GPU.
     *
     * @param pDesc Configuración de la textura (ancho, alto, formato, uso).
     * @param pInitialData Datos iniciales (píxeles) para llenar la textura (opcional).
     * @param ppTexture2D Puntero de salida donde se guardará la nueva textura.
     * @return HRESULT S_OK si la creación fue exitosa.
     */
    HRESULT
        CreateTexture2D(const D3D11_TEXTURE2D_DESC* pDesc,
            const D3D11_SUBRESOURCE_DATA* pInitialData,
            ID3D11Texture2D** ppTexture2D);

    /**
     * @brief Crea una Vista de Profundidad/Stencil (DSV).
     *
     * Necesaria para activar el Z-Buffer (profundidad).
     *
     * @param pResource Recurso de textura que servirá como buffer de profundidad.
     * @param pDesc Descripción de la vista.
     * @param ppDepthStencilView Puntero de salida para la vista creada.
     * @return HRESULT S_OK si la creación fue exitosa.
     */
    HRESULT
        CreateDepthStencilView(ID3D11Resource* pResource,
            const D3D11_DEPTH_STENCIL_VIEW_DESC* pDesc,
            ID3D11DepthStencilView** ppDepthStencilView);

    /**
     * @brief Compila y crea un Vertex Shader en la GPU.
     *
     * @param pShaderBytecode Puntero al código binario compilado del shader (.cso o blob).
     * @param BytecodeLength Tamaño en bytes del código compilado.
     * @param pClassLinkage Enlace de clases dinámico (usualmente nullptr).
     * @param ppVertexShader Puntero de salida para el Shader creado.
     * @return HRESULT S_OK si la creación fue exitosa.
     */
    HRESULT
        CreateVertexShader(const void* pShaderBytecode,
            unsigned int BytecodeLength,
            ID3D11ClassLinkage* pClassLinkage,
            ID3D11VertexShader** ppVertexShader);

    /**
     * @brief Crea el Input Layout (Formato de entrada de vértices).
     *
     * Define cómo se interpretan los datos de los vértices (Posición, Normal, UV)
     * para que coincidan con el Vertex Shader.
     *
     * @param pInputElementDescs Array que describe cada elemento del vértice.
     * @param NumElements Cantidad de elementos en el array.
     * @param pShaderBytecodeWithInputSignature Código del VS para validar la firma.
     * @param BytecodeLength Tamaño del código del shader.
     * @param ppInputLayout Puntero de salida para el Layout creado.
     * @return HRESULT S_OK si la creación fue exitosa.
     */
    HRESULT
        CreateInputLayout(const D3D11_INPUT_ELEMENT_DESC* pInputElementDescs,
            UINT NumElements,
            const void* pShaderBytecodeWithInputSignature,
            unsigned int BytecodeLength,
            ID3D11InputLayout** ppInputLayout);

    /**
     * @brief Compila y crea un Pixel Shader en la GPU.
     *
     * @param pShaderBytecode Puntero al código binario compilado.
     * @param BytecodeLength Tamaño en bytes.
     * @param pClassLinkage Enlace de clases (usualmente nullptr).
     * @param ppPixelShader Puntero de salida para el Shader creado.
     * @return HRESULT S_OK si la creación fue exitosa.
     */
    HRESULT
        CreatePixelShader(const void* pShaderBytecode,
            unsigned int BytecodeLength,
            ID3D11ClassLinkage* pClassLinkage,
            ID3D11PixelShader** ppPixelShader);

    /**
     * @brief Crea un Buffer genérico (Vertex, Index o Constant).
     *
     * @param pDesc Descripción del tamaño y tipo de buffer.
     * @param pInitialData Datos iniciales para rellenar el buffer.
     * @param ppBuffer Puntero de salida para el buffer creado.
     * @return HRESULT S_OK si la creación fue exitosa.
     */
    HRESULT
        CreateBuffer(const D3D11_BUFFER_DESC* pDesc,
            const D3D11_SUBRESOURCE_DATA* pInitialData,
            ID3D11Buffer** ppBuffer);

    /**
     * @brief Crea un estado de muestreo (Sampler State).
     *
     * Define cómo se leen las texturas (filtros bilineales, repetición, clamp, etc.).
     *
     * @param pSamplerDesc Descripción de la configuración del sampler.
     * @param ppSamplerState Puntero de salida para el estado creado.
     * @return HRESULT S_OK si la creación fue exitosa.
     */
    HRESULT
        CreateSamplerState(const D3D11_SAMPLER_DESC* pSamplerDesc,
            ID3D11SamplerState** ppSamplerState);

public:
    /// Puntero nativo a la interfaz del dispositivo de DirectX 11.
    ID3D11Device* m_device = nullptr;
};