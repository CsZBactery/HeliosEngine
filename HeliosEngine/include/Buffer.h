#pragma once
#include "Prerequisites.h"
#include "MeshComponent.h"

/**
 * @file Buffer.h
 * @brief Clase envoltorio (Wrapper) para gestionar recursos de Buffer en DirectX 11.
 * @ingroup core
 */

class Device;
class DeviceContext;

/**
 * @class Buffer
 * @brief Wrapper genérico para buffers de DirectX 11 (ID3D11Buffer).
 * @details Administra la creación, actualización y vinculación de tres tipos de recursos principales:
 * 1. **Vertex Buffers:** Almacenan los datos de los vértices (Posición, Normales, UVs, Tangentes, etc.).
 * 2. **Index Buffers:** Almacenan el orden en que se conectan los vértices para formar triángulos.
 * 3. **Constant Buffers:** Variables uniformes que cambian cada frame (Matrices de transformación, datos de luz).
 *
 * @note La instancia gestiona un solo `ID3D11Buffer` a la vez; su rol en la GPU se define en `m_bindFlag`.
 */
class Buffer {
public:
    /** @brief Constructor por defecto. No reserva memoria en GPU. */
    Buffer() = default;

    /** @brief Destructor. No libera automáticamente; llamar a destroy(). */
    ~Buffer() = default;

    /**
     * @brief Inicializa el buffer como Vertex o Index Buffer usando un MeshComponent.
     * @param device Dispositivo para crear el recurso.
     * @param mesh Componente de malla con los datos en RAM (vértices o índices).
     * @param bindFlag Bandera de enlace (`D3D11_BIND_VERTEX_BUFFER` o `D3D11_BIND_INDEX_BUFFER`).
     * @return `S_OK` si la creación en VRAM fue exitosa.
     */
    HRESULT init(Device& device, const MeshComponent& mesh, unsigned int bindFlag);

    /**
     * @brief Inicializa un buffer de tamaño fijo dinámico (típicamente un Constant Buffer o Structured Buffer).
     * @param device Dispositivo para la creación.
     * @param ByteWidth Tamaño total en bytes. Para Constant Buffers, DEBE ser múltiplo de 16.
     * @return `S_OK` si el hardware asignó la memoria correctamente.
     */
    HRESULT init(Device& device, unsigned int ByteWidth);

    /**
     * @brief Actualiza los datos dentro del buffer mediante `UpdateSubresource`.
     * @details Envía nuevos datos desde la CPU (RAM) a la GPU (VRAM). Esencial para matrices de transformación animadas, variables de interfaz o propiedades de luz.
     * @param deviceContext Contexto de ejecución D3D11.
     * @param pDstResource Recurso destino (usualmente `m_buffer`).
     * @param DstSubresource Índice de subrecurso (normalmente 0 para buffers).
     * @param pDstBox Región a actualizar (nullptr para sobrescribir todo el buffer).
     * @param pSrcData Puntero a los datos de origen en RAM.
     * @param SrcRowPitch Paso por fila (Ignorado por D3D11 en buffers 1D).
     * @param SrcDepthPitch Paso por profundidad (Ignorado por D3D11 en buffers 1D).
     */
    void update(DeviceContext& deviceContext,
        ID3D11Resource* pDstResource,
        unsigned int    DstSubresource,
        const D3D11_BOX* pDstBox,
        const void* pSrcData,
        unsigned int    SrcRowPitch,
        unsigned int    SrcDepthPitch);

    /**
     * @brief Vincula el buffer a la etapa correspondiente del pipeline gráfico para el frame actual.
     * @details El comportamiento varía internamente según `m_bindFlag` (Etapa Input Assembler para Vértices/Índices, o VS/PS para Constantes).
     * @param deviceContext Contexto donde se enlaza el buffer.
     * @param StartSlot Registro o Slot inicial de montaje en el shader.
     * @param NumBuffers Cantidad de buffers a enlazar (típicamente 1).
     * @param setPixelShader Si es `true`, enlaza también al Pixel Shader además del Vertex Shader (sólo aplicable a Constant Buffers).
     * @param format Formato del índice (Obligatorio sólo para Index Buffers, usualmente `DXGI_FORMAT_R32_UINT`).
     */
    void render(DeviceContext& deviceContext,
        unsigned int    StartSlot,
        unsigned int    NumBuffers,
        bool            setPixelShader = false,
        DXGI_FORMAT     format = DXGI_FORMAT_UNKNOWN);

    /**
     * @brief Libera de forma segura el recurso COM `ID3D11Buffer` y reinicia los metadatos internos a 0.
     */
    void destroy();

    /**
     * @brief Helper interno para invocar `D3D11CreateBuffer`.
     * @param device Envoltorio del Device.
     * @param desc Configuración del buffer.
     * @param initData Datos iniciales opcionales (puede ser nullptr si se inicializa vacío).
     * @return `S_OK` si se logró crear.
     */
    HRESULT createBuffer(Device& device, D3D11_BUFFER_DESC& desc, D3D11_SUBRESOURCE_DATA* initData);

public:
    /** @brief Puntero nativo al buffer COM de DirectX 11 administrado. */
    ID3D11Buffer* m_buffer = nullptr;

private:
    /** @brief Tamaño en bytes de un solo elemento (ej. sizeof(SimpleVertex)). Requerido exclusivamente para Vertex Buffers. */
    unsigned int m_stride = 0;

    /** @brief Offset inicial en bytes para la lectura de vértices. */
    unsigned int m_offset = 0;

    /** @brief Bandera interna que define el rol asignado al crearse (Vértice, Índice o Constante). */
    unsigned int m_bindFlag = 0;
};