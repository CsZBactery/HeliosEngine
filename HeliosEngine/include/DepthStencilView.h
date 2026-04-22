/**
 * @file DepthStencilView.h
 * @brief Gestión del Buffer de Profundidad (Z-Buffer) y Plantilla (Stencil) para HeliosEngine.
 * @ingroup core
 */

#pragma once
#include "Prerequisites.h"

class Device;
class DeviceContext;
class Texture;

/**
 * @class DepthStencilView
 * @brief Encapsula la vista de Profundidad y Plantilla (Depth-Stencil View) en DirectX 11.
 * @details Esta clase administra la interfaz ID3D11DepthStencilView, esencial para:
 * 1. **Depth Test (Z-Buffering):** Determina la visibilidad de los píxeles basándose en su distancia
 *    a la cámara, evitando que objetos lejanos se dibujen sobre los cercanos.
 * 2. **Stencil Test:** Permite realizar máscaras de dibujado para efectos como espejos, portales o contornos.
 *
 * @note No administra directamente la vida de la Texture ni del DeviceContext.
 */
class DepthStencilView {
public:
    /** @brief Constructor por defecto. No reserva memoria en GPU. */
    DepthStencilView() = default;

    /** @brief Destructor por defecto. Se debe liberar manualmente con destroy(). */
    ~DepthStencilView() = default;

    /**
     * @brief Inicializa la vista de profundidad vinculándola a una textura.
     * @param device Dispositivo DirectX para la creación del recurso.
     * @param depthStencil Textura que servirá como almacén de datos (debe tener el bind D3D11_BIND_DEPTH_STENCIL).
     * @param format Formato DXGI de la vista (ej: DXGI_FORMAT_D24_UNORM_S8_UINT).
     * @return S_OK si la creación fue exitosa.
     * @post Si retorna S_OK, m_depthStencilView != nullptr.
     */
    HRESULT init(Device& device, Texture& depthStencil, DXGI_FORMAT format);

    /**
     * @brief Inicialización avanzada con especificación de dimensión de vista.
     * @details Útil para configuraciones específicas de hardware (como MSAA) o efectos de post-procesado.
     * @param device Dispositivo DirectX.
     * @param depthStencil Textura de origen.
     * @param format Formato de la vista.
     * @param viewDimension Define cómo el pipeline accede al recurso (Texture2D, Texture2DMS, etc.).
     */
    HRESULT init(Device& device,
        Texture& depthStencil,
        DXGI_FORMAT format,
        D3D11_DSV_DIMENSION viewDimension);

    /** @brief Actualización lógica de la vista (Placeholder para cambios dinámicos). */
    void update() {};

    /**
     * @brief Asigna la vista de profundidad al pipeline de renderizado activo.
     * @details Internamente prepara el buffer para que el Output-Merger pueda realizar pruebas de oclusión.
     * @param deviceContext Contexto donde se emiten los comandos de renderizado.
     * @pre m_depthStencilView debe haberse creado exitosamente con init().
     */
    void render(DeviceContext& deviceContext);

    /**
     * @brief Libera el recurso ID3D11DepthStencilView de la GPU de forma segura.
     * @details Método idempotente: puede llamarse múltiples veces sin riesgo de error.
     * @post m_depthStencilView == nullptr.
     */
    void destroy();

public:
    /**
     * @brief Puntero nativo a la interfaz de vista de profundidad de Direct3D 11.
     * @details Válido tras un init() exitoso; nullptr después de destroy().
     */
    ID3D11DepthStencilView* m_depthStencilView = nullptr;
};