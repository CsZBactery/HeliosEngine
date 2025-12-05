#pragma once
#include "Prerequisites.h"
#include "Device.h"
#include "DeviceContext.h"

class Device;
class DeviceContext;

/**
 * @class SamplerState
 * @brief Encapsula el estado de muestreo (Sampler State) de DirectX 11.
 *
 * El Sampler State define cómo el Pixel Shader lee (muestrea) los datos de una textura.
 * Controla dos aspectos fundamentales:
 * 1. **Filtrado (Filtering):** Qué hacer cuando una textura se ve muy cerca (magnificación)
 * o muy lejos (minificación). Ejemplos: Lineal (suave), Punto (pixel art), Anisotrópico (nítido en ángulo).
 * 2. **Direccionamiento (Addressing):** Qué hacer cuando las coordenadas UV se salen del rango 0-1.
 * Ejemplos: Wrap (repetir), Clamp (estirar el borde), Mirror (espejo).
 */
class SamplerState {
public:

    /**
     * @brief Constructor por defecto.
     */
    SamplerState() = default;

    /**
     * @brief Destructor por defecto.
     */
    ~SamplerState() = default;

    /**
     * @brief Inicializa el estado de muestreo con una configuración predeterminada.
     *
     * Generalmente configura un filtro lineal (o anisotrópico) y modo de repetición (Wrap)
     * para las texturas.
     *
     * @param device Referencia al dispositivo (Factory) para crear el recurso.
     * @return HRESULT S_OK si la creación fue exitosa.
     */
    HRESULT
        init(Device& device);

    /**
     * @brief Actualiza la lógica del sampler.
     *
     * @note Los SamplerStates suelen ser inmutables una vez creados, por lo que este
     * método suele estar vacío, pero se mantiene por consistencia de la interfaz.
     */
    void
        update();

    /**
     * @brief Vincula el Sampler al Pipeline Gráfico (Pixel Shader).
     *
     * Llama internamente a `PSSetSamplers`. Permite que los shaders accedan a este
     * criterio de filtrado.
     *
     * @param deviceContext Contexto donde se realizará el dibujo.
     * @param StartSlot Ranura inicial (registro 's0', 's1', etc. en HLSL).
     * @param NumSamplers Cantidad de samplers a vincular.
     */
    void
        render(DeviceContext& deviceContext,
            unsigned int StartSlot,
            unsigned int NumSamplers);

    /**
     * @brief Libera el recurso ID3D11SamplerState de la memoria.
     */
    void
        destroy();

public:
    /// Puntero nativo a la interfaz de estado de muestreo de DirectX 11.
    ID3D11SamplerState* m_sampler = nullptr;

};