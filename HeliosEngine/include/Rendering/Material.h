/**
 * @file Material.h
 * @brief Definición base de materiales para el motor de renderizado.
 */

#pragma once
#include "Prerequisites.h"
#include "Rendering/RenderTypes.h"

class ShaderProgram;
class RasterizerState;
class DepthStencilState;
class SamplerState;

/**
 * @class Material
 * @brief Representa la configuración global del pipeline gráfico para una superficie.
 * @details Un Material no contiene texturas o colores específicos (eso lo hace MaterialInstance).
 * En su lugar, define las "reglas maestras" de cómo se debe procesar la geometría:
 * qué Shader usar, cómo se mezclan las transparencias (Blend Mode) y cómo interactúa con el Z-Buffer.
 */
class Material {
public:
    // =========================================================================
    // SETTERS
    // =========================================================================

    /** @brief Asigna el programa de sombreado (Vertex/Pixel Shader) maestro. */
    void setShader(ShaderProgram* shader) { m_shader = shader; }

    /** @brief Asigna las reglas de dibujado de polígonos (Culling, Wireframe). */
    void setRasterizerState(RasterizerState* state) { m_rasterizerState = state; }

    /** @brief Asigna las reglas de prueba de profundidad (Z-Buffer). */
    void setDepthStencilState(DepthStencilState* state) { m_depthStencilState = state; }

    /** @brief Asigna cómo se leen los píxeles de las texturas (Filtros, Repetición). */
    void setSamplerState(SamplerState* state) { m_samplerState = state; }

    /** @brief Define si el material pertenece a la cola de opacos o transparentes. */
    void setDomain(MaterialDomain domain) { m_domain = domain; }

    /** @brief Define la matemática para combinar colores (Alpha, Aditivo, etc.). */
    void setBlendMode(BlendMode blendMode) { m_blendMode = blendMode; }

    // =========================================================================
    // GETTERS
    // =========================================================================

    ShaderProgram* getShader() const { return m_shader; }
    RasterizerState* getRasterizerState() const { return m_rasterizerState; }
    DepthStencilState* getDepthStencilState() const { return m_depthStencilState; }
    SamplerState* getSamplerState() const { return m_samplerState; }
    MaterialDomain getDomain() const { return m_domain; }
    BlendMode getBlendMode() const { return m_blendMode; }

private:
    ShaderProgram* m_shader = nullptr;             /**< Puntero al shader base. */
    RasterizerState* m_rasterizerState = nullptr;    /**< Puntero al estado del rasterizador. */
    DepthStencilState* m_depthStencilState = nullptr;  /**< Puntero al estado de profundidad. */
    SamplerState* m_samplerState = nullptr;       /**< Puntero al muestreador de texturas. */
    MaterialDomain     m_domain = MaterialDomain::Opaque; /**< Tipo de dominio visual. */
    BlendMode          m_blendMode = BlendMode::Opaque;   /**< Modo de mezcla gráfica. */
};