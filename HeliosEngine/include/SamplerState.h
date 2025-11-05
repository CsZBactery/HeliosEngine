#pragma once
#include "Prerequisites.h"
#include "Device.h"
#include "DeviceContext.h"

class Device;
class DeviceContext;

/**
 * @class SamplerState
 * @brief Encapsula la creación/gestión de un ID3D11SamplerState.
 *
 * Provee métodos para inicializar, enlazar al pipeline y liberar
 * el estado de muestreo usado por el Pixel Shader (o etapas que apliquen).
 */
class
    SamplerState {
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
     * @brief Inicializa el sampler state.
     * @param device Dispositivo de DirectX usado para crear el recurso.
     * @return HRESULT Código de estado (S_OK en éxito).
     */
    HRESULT
        init(Device& device);

    /**
     * @brief Actualiza el estado interno del sampler.
     * @note Actualmente no realiza ninguna operación.
     */
    void
        update();

    /**
     * @brief Enlaza el sampler al Pixel Shader.
     * @param deviceContext Contexto del dispositivo.
     * @param StartSlot Primer slot de sampler a asignar.
     * @param NumSamplers Número de samplers a asignar.
     */
    void
        render(DeviceContext& deviceContext,
            unsigned int StartSlot,
            unsigned int NumSamplers);

    /**
     * @brief Libera los recursos asociados al sampler.
     */
    void
        destroy();

public:
    ID3D11SamplerState* m_sampler = nullptr; /**< Puntero al objeto ID3D11SamplerState. */
};
