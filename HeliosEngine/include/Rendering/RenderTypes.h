/**
 * @file RenderTypes.h
 * @brief Definiciones de tipos, estructuras de datos y enumeradores fundamentales para el pipeline de renderizado.
 */

#pragma once
#include "Prerequisites.h"

class Mesh;
class MaterialInstance;

/**
 * @enum MaterialDomain
 * @brief Define el dominio al que pertenece un material para determinar su flujo en el pipeline.
 */
enum class MaterialDomain {
    Opaque = 0,    /**< Geometría sólida sin transparencia. */
    Masked,        /**< Geometría con recorte por máscara alpha (binario). */
    Transparent    /**< Geometría con mezcla alpha o efectos translúcidos. */
};

/**
 * @enum BlendMode
 * @brief Modos de mezcla (Blending) para la etapa de salida (Output Merger).
 */
enum class BlendMode {
    Opaque = 0,             /**< Sobrescribe el color previo. */
    Alpha,                  /**< Mezcla tradicional basada en canal alpha. */
    Additive,               /**< Suma de colores para efectos lumínicos. */
    PremultipliedAlpha      /**< Mezcla con alpha ya multiplicado en el color. */
};

/**
 * @enum RenderPassType
 * @brief Identifica los pases de renderizado activos.
 */
enum class RenderPassType {
    Shadow = 0,    /**< Pase de generación de mapa de sombras. */
    Opaque,        /**< Pase de objetos sólidos. */
    Skybox,        /**< Pase de renderizado de fondo. */
    Transparent,   /**< Pase de objetos con transparencia. */
    Editor         /**< Pase para elementos visuales del motor (Gizmos, UI). */
};

/**
 * @enum LightType
 * @brief Clasificación de fuentes de luz soportadas.
 */
enum class LightType {
    Directional = 0, /**< Luz infinita sin posición (Sol). */
    Point,           /**< Luz omnidireccional con caída por distancia. */
    Spot             /**< Luz focalizada en un cono. */
};

/**
 * @struct LightData
 * @brief Contenedor de propiedades físicas y espaciales de una fuente de luz.
 */
struct LightData {
    LightType type = LightType::Directional; /**< Tipo de iluminación. */
    EU::Vector3 color = EU::Vector3(1.0f, 1.0f, 1.0f); /**< Color de la luz (RGB). */
    float intensity = 1.0f; /**< Multiplicador de brillo. */

    EU::Vector3 direction = EU::Vector3(0.0f, -1.0f, 0.0f); /**< Vector de dirección (para Directional y Spot). */
    float range = 0.0f; /**< Radio de alcance (para Point y Spot). */

    EU::Vector3 position = EU::Vector3(0.0f, 0.0f, 0.0f); /**< Posición en el mundo. */
    float spotAngle = 0.0f; /**< Ángulo de apertura del cono (para Spot). */
};

/**
 * @struct MaterialParams
 * @brief Parámetros numéricos PBR que definen la apariencia de un material.
 */
struct MaterialParams {
    XMFLOAT4 baseColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f); /**< Tinte de color base y opacidad. */
    float metallic = 1.0f; /**< Grado de propiedad metálica (0 a 1). */
    float roughness = 1.0f; /**< Grado de rugosidad de la superficie (0 a 1). */
    float ao = 1.0f; /**< Factor de oclusión ambiental. */
    float normalScale = 1.0f; /**< Intensidad del mapa de normales. */
    float emissiveStrength = 1.0f; /**< Intensidad de auto-iluminación. */
    float alphaCutoff = 0.5f; /**< Umbral para el recorte en materiales Masked. */
};

// =========================================================================
// CORRECCIÓN: Se añade LightViewProjection para el Shadow Mapping
// =========================================================================

/**
 * @struct CBPerFrame
 * @brief Buffer constante de datos globales actualizados una vez por fotograma.
 */
struct CBPerFrame {
    XMFLOAT4X4 View{}; /**< Matriz de vista de la cámara activa. */
    XMFLOAT4X4 Projection{}; /**< Matriz de proyección de la cámara activa. */
    XMFLOAT4X4 LightViewProjection{}; // <-- ¡ESTA ES LA MATRIZ QUE FALTABA!

    EU::Vector3 CameraPos{}; /**< Posición de la cámara en el mundo. */
    float pad0 = 0.0f; /**< Relleno para alineación de 16 bytes. */

    EU::Vector3 LightDir = EU::Vector3(0.0f, -1.0f, 0.0f); /**< Dirección de la luz principal. */
    float pad1 = 0.0f; /**< Relleno para alineación de 16 bytes. */

    EU::Vector3 LightColor = EU::Vector3(1.0f, 1.0f, 1.0f); /**< Color de la luz principal. */
    float pad2 = 0.0f; /**< Relleno para alineación de 16 bytes. */
};

/**
 * @struct CBPerObject
 * @brief Buffer constante de datos específicos para cada instancia de objeto.
 */
struct CBPerObject {
    XMFLOAT4X4 World{}; /**< Matriz de transformación de espacio local a mundo. */
};

/**
 * @struct CBPerMaterial
 * @brief Buffer constante con las propiedades de material enviadas a los Shaders.
 */
struct CBPerMaterial {
    XMFLOAT4 BaseColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f); /**< Color base (RGBA). */
    float Metallic = 1.0f; /**< Parámetro metálico. */
    float Roughness = 1.0f; /**< Parámetro rugosidad. */
    float AO = 1.0f; /**< Factor de oclusión. */
    float NormalScale = 1.0f; /**< Escala de normales. */
    float EmissiveStrength = 1.0f; /**< Fuerza emisiva. */
    float AlphaCutoff = 0.0f; /**< Punto de corte alpha. */
    float pad0 = 0.0f; /**< Relleno para alineación HLSL. */
    float pad1 = 0.0f; /**< Relleno para alineación HLSL. */
    float pad2 = 0.0f; /**< Relleno para alineación HLSL. */
    float pad3 = 0.0f; /**< Relleno para alineación HLSL. */
    float pad4 = 0.0f; /**< Relleno para alineación HLSL. */
    float pad5 = 0.0f; /**< Relleno para alineación HLSL. */
};

/**
 * @struct RenderObject
 * @brief Encapsula toda la información necesaria para que el renderizador procese una entidad.
 *
 * Contiene referencias a la geometría, materiales, y estados específicos de renderizado.
 */
struct RenderObject {
    Mesh* mesh = nullptr; /**< Malla geométrica a dibujar. */
    MaterialInstance* materialInstance = nullptr; /**< Material principal (usado en mallas de un solo slot). */
    std::vector<MaterialInstance*> materialInstances; /**< Lista de materiales para mallas multi-slot. */
    XMMATRIX world = XMMatrixIdentity(); /**< Matriz de mundo para el dibujado. */
    bool castShadow = true; /**< Define si el objeto proyecta sombras en el ShadowMap. */
    bool transparent = false; /**< Define si el objeto debe procesarse en la cola de transparencia. */
    float distanceToCamera = 0.0f; /**< Distancia calculada para el ordenamiento de renderizado. */
};