#pragma once
#include "Prerequisites.h"

class Device;
class DeviceContext;
class Texture;

/**
 * @class DepthStencilView
 * @brief Encapsula la vista de Profundidad y Plantilla (Depth-Stencil View) en DirectX 11.
 *
 * Esta clase administra la interfaz ID3D11DepthStencilView. Es fundamental para el renderizado 3D,
 * ya que permite al pipeline realizar:
 * - **Depth Test (Z-Buffering):** Determinar si un píxel está delante o detrás de otro (ocultación).
 * - **Stencil Test:** Operaciones de enmascarado para efectos avanzados (espejos, sombras, recortes).
 */
class DepthStencilView {
public:

    /**
     * @brief Constructor por defecto.
     */
    DepthStencilView() = default;

    /**
     * @brief Destructor por defecto.
     */
    ~DepthStencilView() = default;

    /**
     * @brief Inicializa la vista de profundidad y stencil.
     *
     * Crea el recurso en la GPU enlazándolo a una textura existente que servirá
     * como almacén de datos de profundidad.
     *
     * @param device Referencia al dispositivo (Factory) de DirectX para crear el recurso.
     * @param depthStencil Referencia a la Textura que actuará como buffer de profundidad.
     * @param format Formato DXGI de la vista (ej: DXGI_FORMAT_D24_UNORM_S8_UINT).
     * @return HRESULT S_OK si la inicialización fue exitosa.
     */
    HRESULT
        init(Device& device, Texture& depthStencil, DXGI_FORMAT format);

    /**
     * @brief Actualiza la lógica de la vista (si fuera necesario).
     *
     * @note Actualmente no realiza ninguna operación, pero se mantiene por consistencia de interfaz.
     */
    void
        update() {};

    /**
     * @brief Gestiona el uso de la vista durante el renderizado (usualmente Limpieza).
     *
     * Dependiendo de la implementación interna, este método suele llamar a
     * ClearDepthStencilView para borrar el buffer de profundidad antes de dibujar el nuevo frame.
     *
     * @param deviceContext Contexto del dispositivo para emitir los comandos.
     */
    void
        render(DeviceContext& deviceContext);

    /**
     * @brief Libera los recursos de DirectX asociados y resetea el puntero.
     */
    void
        destroy();

public:
    /// Puntero nativo a la interfaz de vista de profundidad de D3D11.
    ID3D11DepthStencilView* m_depthStencilView = nullptr;

};