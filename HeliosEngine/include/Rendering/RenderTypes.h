/**
 * @file RenderTypes.h
 * @brief Definiciones de tipos, estructuras y enumeradores base para el sistema de renderizado.
 */

#pragma once
#include "Prerequisites.h"

class Mesh;
class MaterialInstance;

/**
 * @enum MaterialDomain
 * @brief Define el dominio al que pertenece un material para clasificar su comportamiento en el pipeline.
 */
enum class
	MaterialDomain {
	Opaque = 0,    ///< Material sólido sin transparencia.
	Masked,        ///< Material con recorte binario mediante canal alfa (cutout).
	Transparent    ///< Material con mezcla alfa (blending).
};

/**
 * @enum BlendMode
 * @brief Modos de mezcla utilizados en la etapa de salida (Output Merger).
 */
enum class
	BlendMode {
	Opaque = 0,           ///< Sin mezcla de colores.
	Alpha,                ///< Mezcla alfa tradicional.
	Additive,             ///< Suma de colores.
	PremultipliedAlpha    ///< Mezcla con alfa pre-multiplicado.
};

/**
 * @enum RenderPassType
 * @brief Identifica los distintos pases de renderizado soportados por el motor.
 */
enum class
	RenderPassType {
	Shadow = 0,    ///< Pase para generación de mapas de sombras.
	Opaque,        ///< Pase para objetos opacos.
	Skybox,        ///< Pase para el renderizado del fondo.
	Transparent,   ///< Pase para objetos con transparencia.
	Editor         ///< Pase para elementos visuales del editor/herramientas.
};

/**
 * @enum LightType
 * @brief Tipos de fuentes de luz disponibles.
 */
enum class
	LightType {
	Directional = 0, ///< Luz infinita con dirección constante (ej. Sol).
	Point,           ///< Luz omnidireccional con origen en un punto.
	Spot             ///< Luz focalizada en forma de cono.
};

/**
 * @struct LightData
 * @brief Datos que definen las propiedades físicas y espaciales de una luz.
 */
struct
	LightData {
	LightType type = LightType::Directional;  ///< Tipo de iluminación.
	EU::Vector3 color = EU::Vector3(1.0f, 1.0f, 1.0f); ///< Color de la luz en RGB.
	float intensity = 1.0f;                   ///< Intensidad o brillo.

	EU::Vector3 direction = EU::Vector3(0.0f, -1.0f, 0.0f); ///< Dirección (para luces direccionales y spot).
	float range = 0.0f;                       ///< Alcance (para luces puntuales y spot).

	EU::Vector3 position = EU::Vector3(0.0f, 0.0f, 0.0f); ///< Posición en el mundo.
	float spotAngle = 0.0f;                   ///< Ángulo de apertura (para luces spot).
};

/**
 * @struct MaterialParams
 * @brief Parámetros numéricos configurables de un material PBR.
 */
struct
	MaterialParams {
	XMFLOAT4 baseColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f); ///< Color base y opacidad (RGBA).
	float metallic = 1.0f;            ///< Factor metálico.
	float roughness = 1.0f;           ///< Factor de rugosidad.
	float ao = 1.0f;                  ///< Factor de oclusión ambiental.
	float normalScale = 1.0f;         ///< Multiplicador de intensidad del mapa de normales.
	float emissiveStrength = 1.0f;    ///< Intensidad de la auto-iluminación.
	float alphaCutoff = 0.5f;         ///< Umbral para materiales Masked.
};

/**
 * @struct CBPerFrame
 * @brief Datos constantes de la escena actualizados una vez por fotograma (Frame Buffer).
 */
struct
	CBPerFrame {
	XMFLOAT4X4 View{};                ///< Matriz de Vista de la cámara.
	XMFLOAT4X4 Projection{};          ///< Matriz de Proyección de la cámara.
	EU::Vector3 CameraPos{};          ///< Posición de la cámara en el mundo.
	float pad0 = 0.0f;                ///< Relleno para alineación de 16 bytes.
	EU::Vector3 LightDir = EU::Vector3(0.0f, -1.0f, 0.0f); ///< Dirección de la luz principal.
	float pad1 = 0.0f;                ///< Relleno para alineación de 16 bytes.
	EU::Vector3 LightColor = EU::Vector3(1.0f, 1.0f, 1.0f); ///< Color de la luz principal.
	float pad2 = 0.0f;                ///< Relleno para alineación de 16 bytes.
};

/**
 * @struct CBPerObject
 * @brief Datos constantes específicos de una instancia de objeto (Object Buffer).
 */
struct
	CBPerObject {
	XMFLOAT4X4 World{};               ///< Matriz de transformación al espacio de mundo.
};

/**
 * @struct CBPerMaterial
 * @brief Datos constantes de propiedades de material enviados a la GPU (Material Buffer).
 */
struct
	CBPerMaterial {
	XMFLOAT4 BaseColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f); ///< Color base del material.
	float Metallic = 1.0f;            ///< Propiedad metálica.
	float Roughness = 1.0f;           ///< Propiedad de rugosidad.
	float AO = 1.0f;                  ///< Factor de oclusión.
	float NormalScale = 1.0f;         ///< Escala de normales.
	float EmissiveStrength = 1.0f;    ///< Fuerza de emisión.
	float AlphaCutoff = 0.0f;         ///< Punto de corte para transparencia binaria.
	float pad0 = 0.0f;                ///< Padding técnico para alineación HLSL.
	float pad1 = 0.0f;                ///< Padding técnico para alineación HLSL.
	float pad2 = 0.0f;                ///< Padding técnico para alineación HLSL.
	float pad3 = 0.0f;                ///< Padding técnico para alineación HLSL.
	float pad4 = 0.0f;                ///< Padding técnico para alineación HLSL.
	float pad5 = 0.0f;                ///< Padding técnico para alineación HLSL.
};

/**
 * @struct RenderObject
 * @brief Encapsula toda la información necesaria para que el renderizador procese una entidad visible.
 */
struct
	RenderObject {
	Mesh* mesh = nullptr;                      ///< Malla geométrica.
	MaterialInstance* materialInstance = nullptr; ///< Instancia de material principal.
	std::vector<MaterialInstance*> materialInstances; ///< Lista de materiales para mallas de múltiples slots.
	XMMATRIX world = XMMatrixIdentity();       ///< Matriz de transformación de mundo actual.
	bool castShadow = true;                    ///< Indica si el objeto proyecta sombras.
	bool transparent = false;                  ///< Indica si el objeto requiere procesamiento de transparencia.
	float distanceToCamera = 0.0f;             ///< Distancia a la cámara para el ordenamiento de renderizado.
};