/**
 * @file LayoutBuilder.h
 * @brief Clase de utilidad para construir descriptores de Input Layout de forma fluida.
 */

#pragma once
#include "Prerequisites.h"

 /**
  * @class LayoutBuilder
  * @brief Facilita la creación de vectores D3D11_INPUT_ELEMENT_DESC.
  * * En lugar de declarar un array estático y calcular offsets manualmente, esta clase
  * permite añadir elementos (Posición, Normal, UV) uno tras otro.
  * Es especialmente útil para configurar el Instancing de forma rápida.
  */
class LayoutBuilder {
public:
    /**
     * @brief Añade un elemento estándar al layout (por defecto por cada vértice).
     * * @param semantic Nombre de la semántica en HLSL (ej: "POSITION").
     * @param format Formato de datos DXGI (ej: DXGI_FORMAT_R32G32B32_FLOAT).
     * @param semanticIndex Índice si hay múltiples semánticas iguales (ej: TEXCOORD 0, 1).
     * @param inputSlot Slot de entrada (0 para el Vertex Buffer principal).
     * @param alignedByteOffset Desplazamiento en bytes. D3D11_APPEND_ALIGNED_ELEMENT lo calcula automático.
     * @param slotClass Clasificación (Per-Vertex o Per-Instance).
     * @param instanceStepRate Cuántas instancias dibujar antes de avanzar en el buffer (0 para per-vertex).
     * @return Referencia a sí mismo para encadenar llamadas.
     */
    LayoutBuilder&
        Add(const char* semantic,
            DXGI_FORMAT format,
            UINT semanticIndex = 0,
            UINT inputSlot = 0,
            UINT alignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT,
            D3D11_INPUT_CLASSIFICATION slotClass = D3D11_INPUT_PER_VERTEX_DATA,
            UINT instanceStepRate = 0) {

        D3D11_INPUT_ELEMENT_DESC d{};
        d.SemanticName = semantic;
        d.SemanticIndex = semanticIndex;
        d.Format = format;
        d.InputSlot = inputSlot;
        d.AlignedByteOffset = alignedByteOffset;
        d.InputSlotClass = slotClass;
        d.InstanceDataStepRate = instanceStepRate;

        m_elems.push_back(d);
        return *this;
    }

    /**
     * @brief Atajo para añadir datos de Instancing (matrices de mundo por instancia, etc).
     * * Configura automáticamente el InputSlotClass como D3D11_INPUT_PER_INSTANCE_DATA.
     */
    LayoutBuilder&
        AddInstance(const char* semantic,
            DXGI_FORMAT format,
            UINT semanticIndex = 0,
            UINT inputSlot = 1,
            UINT alignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT,
            UINT instanceStepRate = 1) {

        return Add(semantic, format, semanticIndex, inputSlot, alignedByteOffset,
            D3D11_INPUT_PER_INSTANCE_DATA, instanceStepRate);
    }

    /** @brief Retorna el vector de elementos construido. */
    const std::vector<D3D11_INPUT_ELEMENT_DESC>& Get() const { return m_elems; }

    /** @brief Retorna la cantidad de elementos en el layout. */
    UINT Count() const { return (UINT)m_elems.size(); }

private:
    /** @brief Almacén temporal de las descripciones de los elementos. */
    std::vector<D3D11_INPUT_ELEMENT_DESC> m_elems;
};