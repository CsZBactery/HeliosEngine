/**
 * @file MaterialInstance.h
 * @brief Definición de la clase MaterialInstance para la gestión de instancias de materiales.
 */

#pragma once
#include "Prerequisites.h"
#include "Rendering/RenderTypes.h"

class Material;
class DeviceContext;
class Texture;

/**
 * @class MaterialInstance
 * @brief Clase que representa una instancia específica de un material con sus propias texturas y parámetros.
 *
 * Permite que diferentes objetos compartan un mismo Material base (Shader/Estados) pero
 * utilicen distintas texturas (Albedo, Normal, etc.) y valores de configuración.
 */
class
	MaterialInstance {
public:
	/**
	 * @brief Asigna el material base (Shader y configuraciones de pipeline).
	 * @param material Puntero al material base.
	 */
	void setMaterial(Material* material) { m_material = material; }

	/** @brief Asigna la textura de Albedo (Color base). @param texture Puntero a la textura. */
	void setAlbedo(Texture* texture) { m_albedo = texture; }

	/** @brief Asigna la textura de Normales. @param texture Puntero a la textura. */
	void setNormal(Texture* texture) { m_normal = texture; }

	/** @brief Asigna la textura de Metalicidad. @param texture Puntero a la textura. */
	void setMetallic(Texture* texture) { m_metallic = texture; }

	/** @brief Asigna la textura de Rugosidad (Roughness). @param texture Puntero a la textura. */
	void setRoughness(Texture* texture) { m_roughness = texture; }

	/** @brief Asigna la textura de Oclusión Ambiental (AO). @param texture Puntero a la textura. */
	void setAO(Texture* texture) { m_ao = texture; }

	/** @brief Asigna la textura de Emisivo. @param texture Puntero a la textura. */
	void setEmissive(Texture* texture) { m_emissive = texture; }

	/** @brief Obtiene el material base. @return Puntero al material. */
	Material* getMaterial() const { return m_material; }

	/** @brief Obtiene la textura de Albedo. @return Puntero a la textura. */
	Texture* getAlbedo() const { return m_albedo; }

	/** @brief Obtiene la textura de Normales. @return Puntero a la textura. */
	Texture* getNormal() const { return m_normal; }

	/** @brief Obtiene la textura de Metalicidad. @return Puntero a la textura. */
	Texture* getMetallic() const { return m_metallic; }

	/** @brief Obtiene la textura de Rugosidad. @return Puntero a la textura. */
	Texture* getRoughness() const { return m_roughness; }

	/** @brief Obtiene la textura de Oclusión Ambiental. @return Puntero a la textura. */
	Texture* getAO() const { return m_ao; }

	/** @brief Obtiene la textura de Emisivo. @return Puntero a la textura. */
	Texture* getEmissive() const { return m_emissive; }

	/** @brief Obtiene los parámetros numéricos del material (metálico, rugosidad, etc). @return Referencia a MaterialParams. */
	MaterialParams& getParams() { return m_params; }

	/** @brief Obtiene los parámetros numéricos del material (versión constante). @return Referencia constante a MaterialParams. */
	const MaterialParams& getParams() const { return m_params; }

	/**
	 * @brief Vincula todas las texturas de la instancia a los slots correspondientes del pipeline de DirectX.
	 * @param deviceContext Contexto del dispositivo para realizar la vinculación.
	 */
	void bindTextures(DeviceContext& deviceContext) const;

private:
	Material* m_material = nullptr;      /**< Puntero al recurso de material base. */
	Texture* m_albedo = nullptr;        /**< Textura de color base. */
	Texture* m_normal = nullptr;        /**< Mapa de normales. */
	Texture* m_metallic = nullptr;      /**< Mapa de propiedades metálicas. */
	Texture* m_roughness = nullptr;     /**< Mapa de rugosidad de la superficie. */
	Texture* m_ao = nullptr;            /**< Mapa de oclusión ambiental. */
	Texture* m_emissive = nullptr;      /**< Mapa de iluminación emisiva. */
	MaterialParams m_params;            /**< Estructura con parámetros PBR numéricos. */
};