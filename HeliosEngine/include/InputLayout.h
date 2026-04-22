/**
 * @file InputLayout.h
 * @brief Gestión del formato de entrada de vértices para el pipeline de DirectX 11.
 * @ingroup core
 */

#pragma once
#include "Prerequisites.h"

class Device;
class DeviceContext;

/**
 * @class InputLayout
 * @brief Define el formato de los datos de los vértices en el pipeline gráfico.
 * @details El Input Layout actúa como un "puente" o "diccionario" entre los datos crudos
 * de los Vertex Buffers (C++) y las variables de entrada que espera el Vertex Shader (HLSL).
 *
 * Sin esto, la GPU no sabría interpretar si un grupo de bytes corresponde a una
 * Posición (float3), a una Normal o a Coordenadas de Textura (UV). Administra el ciclo
 * de vida del recurso COM ID3D11InputLayout.
 */
class InputLayout {
public:
    /** @brief Constructor por defecto. */
    InputLayout() = default;

    /** @brief Destructor por defecto. Se requiere llamar a destroy() para liberar recursos COM. */
    ~InputLayout() = default;

    /**
     * @brief Inicializa el Input Layout en la GPU mediante puntero y conteo.
     * @param device Dispositivo con el que se crea el recurso.
     * @param layoutDesc Arreglo de descripciones de elementos de entrada.
     * @param layoutCount Número de elementos en el arreglo.
     * @param vertexShaderData Bytecode del Vertex Shader para validar la firma de entrada.
     * @return S_OK si la creación fue exitosa.
     * @post Si retorna S_OK, m_inputLayout != nullptr.
     */
    HRESULT init(Device& device,
        const D3D11_INPUT_ELEMENT_DESC* layoutDesc,
        UINT layoutCount,
        ID3DBlob* vertexShaderData);

    /**
     * @brief Sobrecarga de inicialización usando std::vector (Amigable para HeliosEngine).
     * @param device Referencia al dispositivo.
     * @param layout Vector con la descripción del formato de vértices.
     * @param vertexShaderData Bytecode compilado del Vertex Shader.
     * @return S_OK si la creación fue exitosa.
     */
    HRESULT init(Device& device,
        std::vector<D3D11_INPUT_ELEMENT_DESC>& layout,
        ID3DBlob* vertexShaderData);

    /**
     * @brief Actualización lógica del layout (Placeholder).
     * @note Actualmente no realiza ninguna operación.
     */
    void update();

    /**
     * @brief Aplica el Input Layout al pipeline gráfico (etapa Input Assembler).
     * @details Invoca IASetInputLayout. A partir de este momento, la GPU interpretará
     * los buffers de vértices siguiendo este formato específico.
     * @param deviceContext Contexto donde se establecerá el layout.
     * @pre m_inputLayout debe haberse creado exitosamente con init().
     */
    void render(DeviceContext& deviceContext);

    /**
     * @brief Libera el recurso ID3D11InputLayout de la memoria de video de forma segura.
     * @details Es un método idempotente; puede llamarse varias veces sin riesgo.
     * @post m_inputLayout == nullptr.
     */
    void destroy();

public:
    /**
     * @brief Recurso COM de Direct3D 11 que representa el Input Layout.
     * @details Válido tras init(); nullptr después de destroy().
     */
    ID3D11InputLayout* m_inputLayout = nullptr;
};