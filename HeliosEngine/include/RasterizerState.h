/**
 * @file RasterizerState.h
 * @brief Encapsula el estado de rasterización para el pipeline gráfico de DirectX 11.
 */

#pragma once
#include "Prerequisites.h"

class Device;
class DeviceContext;

/**
 * @class RasterizerState
 * @brief Encapsula un @c ID3D11RasterizerState para configurar la etapa de rasterización.
 *
 * @details La etapa de rasterización en Direct3D 11 define cómo se transforman las primitivas
 * (triángulos, líneas, puntos) en fragmentos (píxeles) antes de pasar al Pixel Shader.
 *
 * Esta clase administra la creación, aplicación y destrucción de un estado de rasterización,
 * permitiendo configurar opciones vitales como el modo de relleno (Wireframe o Sólido),
 * el Culling (descarte de caras para optimización) y el recorte (clipping) de profundidad.
 */
class
    RasterizerState {
public:
    /**
     * @brief Constructor por defecto.
     */
    RasterizerState() = default;

    /**
     * @brief Destructor por defecto.
     * @warning No libera automáticamente el recurso COM de la GPU; se debe llamar explícitamente a destroy().
     */
    ~RasterizerState() = default;

    /**
     * @brief Inicializa el Rasterizer State con una configuración estándar.
     *
     * @details Crea un @c ID3D11RasterizerState con la configuración predeterminada del motor
     * (usualmente @c D3D11_FILL_SOLID y @c D3D11_CULL_BACK).
     *
     * @param device Dispositivo con el que se creará el recurso en hardware.
     * @return @c S_OK si la creación fue exitosa; código @c HRESULT en caso de error.
     *
     * @post Si retorna @c S_OK, @c m_rasterizerState != nullptr.
     */
    HRESULT
        init(Device device);

    /**
     * @brief Inicializa el Rasterizer State con parámetros personalizados.
     *
     * @details Útil para crear estados alternativos, como uno específico para ver mallas
     * en modo Wireframe o uno sin Culling para dibujar el interior del Skybox.
     *
     * @param device Referencia al dispositivo con el que se creará el recurso.
     * @param fill Modo de relleno de los polígonos (ej. D3D11_FILL_SOLID o D3D11_FILL_WIREFRAME).
     * @param cull Modo de descarte de caras (ej. D3D11_CULL_NONE, D3D11_CULL_FRONT, D3D11_CULL_BACK).
     * @param frontCCW Define si el orden de los vértices en sentido antihorario (Counter-Clockwise) se considera la cara frontal.
     * @param depthClip Habilita el recorte (clipping) de píxeles basándose en el Z-Buffer.
     * @return @c S_OK si la creación fue exitosa; código @c HRESULT en caso de error.
     */
    HRESULT
        init(Device& device,
            D3D11_FILL_MODE fill,
            D3D11_CULL_MODE cull,
            bool frontCCW,
            bool depthClip);

    /**
     * @brief Actualiza parámetros internos del Rasterizador de forma dinámica.
     *
     * @note Método diseñado como marcador para futuras implementaciones donde se requiera
     * modificar el estado sin destruirlo por completo. Actualmente no realiza ninguna operación.
     */
    void
        update();

    /**
     * @brief Aplica este estado de rasterización al pipeline activo.
     *
     * @details Invoca @c ID3D11DeviceContext::RSSetState. Todos los modelos que se dibujen
     * después de esta llamada obedecerán las reglas de culling y relleno aquí definidas.
     *
     * @param deviceContext Contexto del dispositivo responsable de enviar los comandos a la GPU.
     *
     * @pre @c m_rasterizerState debe haberse creado previamente con init().
     */
    void
        render(DeviceContext& deviceContext);

    /**
     * @brief Libera el recurso gráfico y limpia la memoria.
     *
     * @details Destruye de forma segura el puntero COM. Es idempotente, lo que significa que
     * puede llamarse varias veces sin causar errores de corrupción de memoria.
     *
     * @post @c m_rasterizerState == nullptr.
     */
    void
        destroy();

private:
    /**
     * @brief Puntero directo a la interfaz del estado de rasterización de Direct3D 11.
     * @details Válido después de ejecutarse init(); vuelve a ser nulo tras invocar destroy().
     */
    ID3D11RasterizerState* m_rasterizerState = nullptr;
};