#pragma once
#include "Prerequisites.h"

/**
 * @file InputLayout.h
 * @brief Declaración de la clase @c InputLayout que encapsula la creación, uso y destrucción
 *        de un @c ID3D11InputLayout para describir el formato de vértices en DirectX 11.
 */

class
    Device;

class
    DeviceContext;

/**
 * @class InputLayout
 * @brief Encapsula un @c ID3D11InputLayout y operaciones básicas de ciclo de vida.
 *
 * Proporciona métodos para:
 * - Inicializar el input layout a partir de una lista de @c D3D11_INPUT_ELEMENT_DESC
 *   y el blob del vertex shader (firma de entrada).
 * - Hacer @c bind del layout al pipeline (IA) durante el render.
 * - Destruir y liberar los recursos asociados.
 */
class
    InputLayout {
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
     * @brief Crea el @c ID3D11InputLayout a partir de la descripción de elementos y la firma del VS.
     *
     * @param device        Referencia al dispositivo de DirectX 11.
     * @param Layout        Vector con la descripción de los elementos de entrada (semánticas, formatos, etc.).
     * @param VertexShaderData Blob del vertex shader que contiene la firma de entrada (input signature).
     * @return @c S_OK en caso de éxito, o un código @c HRESULT de error.
     */
    HRESULT
        init(Device& device,
            std::vector<D3D11_INPUT_ELEMENT_DESC>& Layout,
            ID3DBlob* VertexShaderData);

    /**
     * @brief Punto de extensión para actualizar el estado del layout.
     *
     * Actualmente no realiza ninguna operación.
     */
    void
        update();

    /**
     * @brief Establece el @c ID3D11InputLayout en el pipeline (IA) mediante el @c DeviceContext.
     *
     * @param deviceContext Contexto del dispositivo usado para fijar el layout activo.
     */
    void
        render(DeviceContext& deviceContext);

    /**
     * @brief Libera el recurso @c ID3D11InputLayout asociado.
     */
    void
        destroy();


public:
    /** @brief Puntero al objeto @c ID3D11InputLayout subyacente. */
    ID3D11InputLayout* m_inputLayout = nullptr;
};
