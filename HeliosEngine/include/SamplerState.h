#pragma once
// ../include/
#include "../include/Prerequisites.h"

class Device;
class DeviceContext;

/**
 * @class SamplerState
 * @brief Envoltura mínima de ID3D11SamplerState para PS (y VS si lo necesitas).
 */
class SamplerState {
public:
    SamplerState() = default;
    ~SamplerState() = default;

    /**
     * @brief Crea un sampler lineal con address wrap y LOD completo.
     * @return S_OK o HRESULT de error.
     */
    HRESULT init(Device& device);

    /// (Placeholder) No hacemos updates dinámicos aquí.
    void update();

    /**
     * @brief Enlaza el sampler al PS.
     */
    void render(DeviceContext& deviceContext,
        unsigned int StartSlot,
        unsigned int NumSamplers);

    /// Libera el recurso COM.
    void destroy();

public:
    ID3D11SamplerState* m_sampler = nullptr;
};
