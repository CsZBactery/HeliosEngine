#pragma once
#include "Prerequisites.h"

class Device;
class DeviceContext;

/**
 * @class InputLayout
 * @brief Define el formato de los datos de los vértices en el pipeline gráfico.
 *
 * El Input Layout actúa como un "puente" o "diccionario" entre los datos crudos
 * que enviamos en los Vertex Buffers (C++) y las variables de entrada que espera
 * recibir el Vertex Shader (HLSL).
 *
 * Sin esto, la GPU no sabría que los primeros 12 bytes corresponden a la Posición,
 * los siguientes 8 a las Coordenadas de Textura, etc.
 */
class InputLayout {
public:
    /**
     * @brief Constructor por defecto.
     */
    InputLayout() = default;

    /**
     * @brief Destructor por defecto.
     */
    ~InputLayout() = default;

    /**
     * @brief Crea e inicializa el Input Layout en la GPU.
     *
     * @param device Referencia al dispositivo para crear el recurso.
     * @param Layout Vector que describe cada elemento del vértice (Semántica, formato, slot).
     * @param VertexShaderData Blob con el código compilado del Vertex Shader.
     * Es necesario para validar que el Layout de C++ coincide con el del Shader.
     * @return HRESULT S_OK si la creación fue exitosa.
     */
    HRESULT
        init(Device& device,
            std::vector<D3D11_INPUT_ELEMENT_DESC>& Layout,
            ID3DBlob* VertexShaderData);

    /**
     * @brief Actualiza la lógica del layout (si fuera necesario).
     * @note Generalmente no se utiliza en layouts estáticos.
     */
    void
        update();

    /**
     * @brief Activa este Input Layout en el contexto de renderizado.
     *
     * Llama internamente a IASetInputLayout. A partir de este momento, la GPU
     * interpretará los buffers de vértices siguiendo este formato.
     *
     * @param deviceContext El contexto donde se realizará el dibujo.
     */
    void
        render(DeviceContext& deviceContext);

    /**
     * @brief Libera el recurso de memoria de la GPU.
     */
    void
        destroy();

public:
    /// Puntero nativo a la interfaz de Input Layout de DirectX 11.
    ID3D11InputLayout* m_inputLayout = nullptr;
};