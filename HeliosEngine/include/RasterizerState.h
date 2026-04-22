/**
 * @file RasterizerState.h
 * @brief Gestión de la etapa de rasterización y configuración de visualización de polígonos.
 * @ingroup core
 */

#pragma once
#include "Prerequisites.h"

class Device;
class DeviceContext;

/**
 * @class RasterizerState
 * @brief Encapsula un @c ID3D11RasterizerState para configurar cómo se transforman primitivas en fragmentos (píxeles).
 * @details En HeliosEngine, esta clase administra la etapa de rasterización del pipeline gráfico, permitiendo:
 * 1. **Modo de Relleno:** Alternar entre @c SOLID (visualización normal) y @c WIREFRAME (útil para depuración de mallas).
 * 2. **Culling:** Optimizar el rendimiento descartando caras ocultas (Back, Front o None). Vital para el renderizado de interiores o el Skybox.
 * 3. **Clipping:** Gestión del recorte de profundidad basándose en el volumen de visión (View Frustum).
 *
 * @note La etapa de rasterización ocurre justo antes del Pixel Shader.
 */
class RasterizerState {
public:
    /** @brief Constructor por defecto. No reserva memoria en GPU. */
    RasterizerState() = default;

    /** @brief Destructor por defecto. Se requiere llamar a destroy() para liberar recursos COM. */
    ~RasterizerState() = default;

    /**
     * @brief Inicializa el Rasterizer State con la configuración por defecto del motor.
     * @details Usualmente configura @c D3D11_FILL_SOLID y @c D3D11_CULL_BACK.
     * @param device Dispositivo con el que se creará el recurso en la GPU.
     * @return S_OK si la creación fue exitosa.
     * @post Si retorna S_OK, m_rasterizerState != nullptr.
     */
    HRESULT init(Device device);

    /**
     * @brief Inicializa el Rasterizer State con parámetros técnicos específicos.
     * @details Útil para estados especiales como el del Skybox (Cull None) o Debug (Wireframe).
     * @param device Referencia al dispositivo físico.
     * @param fill Modo de relleno (Solid/Wireframe).
     * @param cull Modo de descarte de caras (None/Front/Back).
     * @param frontCCW Si es true, el sentido antihorario define la cara frontal (DirectX usa típicamente sentido horario).
     * @param depthClip Habilita o deshabilita el recorte de píxeles fuera del rango de profundidad.
     * @return S_OK si la configuración fue aceptada por el hardware.
     */
    HRESULT init(Device& device,
        D3D11_FILL_MODE fill,
        D3D11_CULL_MODE cull,
        bool frontCCW,
        bool depthClip);

    /**
     * @brief Actualización dinámica de parámetros (Placeholder).
     * @note Actualmente no realiza ninguna operación.
     */
    void update();

    /**
     * @brief Aplica la configuración de este estado al pipeline activo.
     * @details Invoca @c RSSetState en el contexto del dispositivo para establecer el rasterizador activo.
     * @param deviceContext Contexto encargado de enviar los comandos a la GPU.
     * @pre m_rasterizerState debe haberse creado exitosamente con init().
     */
    void render(DeviceContext& deviceContext);

    /**
     * @brief Libera el recurso ID3D11RasterizerState y limpia el puntero de forma segura.
     * @details Es un método idempotente.
     * @post m_rasterizerState == nullptr.
     */
    void destroy();

private:
    /** @brief Interfaz de estado de rasterización nativa de Direct3D 11. */
    ID3D11RasterizerState* m_rasterizerState = nullptr;
};