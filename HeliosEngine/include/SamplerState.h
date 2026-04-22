/**
 * @file SamplerState.h
 * @brief Gestión del estado de muestreo de texturas (Sampler State) en DirectX 11.
 * @ingroup core
 */

#pragma once
#include "Prerequisites.h"

 // Forward Declarations para reducir tiempos de compilación
class Device;
class DeviceContext;

/**
 * @class SamplerState
 * @brief Encapsula un @c ID3D11SamplerState para la etapa de muestreo en el pipeline gráfico.
 * @details El Sampler State define cómo el Pixel Shader lee (muestrea) los datos de una textura,
 * controlando dos aspectos fundamentales del HeliosEngine:
 *
 * 1. **Filtrado (Filtering):** Define el comportamiento en magnificación o minificación.
 *    Ej: Lineal (suave), Punto (estilo retro) o Anisotrópico (nitidez en ángulos oblicuos).
 * 2. **Direccionamiento (Addressing):** Define qué ocurre cuando las coordenadas UV exceden el rango [0, 1].
 *    Ej: Wrap (repetir), Clamp (estirar bordes) o Mirror (espejo).
 */
class SamplerState {
public:
    /** @brief Constructor por defecto. No reserva memoria en GPU. */
    SamplerState() = default;

    /** @brief Destructor por defecto. Se debe liberar manualmente con destroy(). */
    ~SamplerState() = default;

    /**
     * @brief Inicializa el Sampler State con una configuración predeterminada.
     * @details Generalmente configura un filtro lineal/anisotrópico y modo @c Wrap
     * para que las texturas se repitan correctamente sobre las mallas.
     * @param device Referencia al dispositivo para crear el recurso COM.
     * @return S_OK si la creación en VRAM fue exitosa.
     * @post Si retorna S_OK, m_sampler != nullptr.
     */
    HRESULT init(Device& device);

    /**
     * @brief Actualiza los parámetros del sampler.
     * @note Los SamplerStates son inmutables en D3D11; este método es un placeholder
     * para mantener la consistencia de la interfaz en HeliosEngine.
     */
    void update();

    /**
     * @brief Vincula el Sampler al Pipeline Gráfico (etapa de Pixel Shader).
     * @details Invoca internamente @c PSSetSamplers, permitiendo que los registros
     * de sampler en HLSL (s0, s1, etc.) utilicen este criterio de filtrado.
     * @param deviceContext Contexto donde se emitirán las órdenes de dibujo.
     * @param StartSlot Ranura inicial de registro en el shader.
     * @param NumSamplers Cantidad de samplers a vincular simultáneamente.
     * @pre m_sampler debe haberse creado exitosamente con init().
     */
    void render(DeviceContext& deviceContext,
        unsigned int StartSlot,
        unsigned int NumSamplers);

    /**
     * @brief Libera el recurso ID3D11SamplerState y limpia el puntero de forma segura.
     * @details Método idempotente.
     * @post m_sampler == nullptr.
     */
    void destroy();

public:
    /**
     * @brief Puntero nativo a la interfaz de estado de muestreo de Direct3D 11.
     * @details Válido tras init(); nullptr después de destroy().
     */
    ID3D11SamplerState* m_sampler = nullptr;
};