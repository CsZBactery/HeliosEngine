/**
 * @file MaterialInstance.h
 * @brief Definición de la clase MaterialInstance para la gestión de instancias de materiales PBR.
 */

#pragma once
#include "Prerequisites.h"
#include "Rendering/RenderTypes.h"

class Material;
class DeviceContext;
class Texture;

/**
 * @class MaterialInstance
 * @brief Representa una instancia única de un material, permitiendo variaciones de texturas y parámetros físicos.
 *
 * Esta clase actúa como un contenedor para los mapas de textura y parámetros (Metálico, Rugosidad, AO, etc.)
 * que definen la apariencia de un objeto específico utilizando un material base.
 */
class
	MaterialInstance {
public:
	/** @brief Asigna el material base. @param material Puntero al material. */
	void setMaterial(Material* material) { m_material = material; }
	/** @brief Define la textura de Albedo (Color base). @param texture Puntero a la textura. */
	void setAlbedo(Texture* texture) { m_albedo = texture; }
	/** @brief Define el mapa de normales. @param texture Puntero a la textura. */
	void setNormal(Texture* texture) { m_normal = texture; }
	/** @brief Define el mapa de propiedades metálicas. @param texture Puntero a la textura. */
	void setMetallic(Texture* texture) { m_metallic = texture; }
	/** @brief Define el mapa de rugosidad. @param texture Puntero a la textura. */
	void setRoughness(Texture* texture) { m_roughness = texture; }
	/** @brief Define el mapa de oclusión ambiental. @param texture Puntero a la textura. */
	void setAO(Texture* texture) { m_ao = texture; }
	/** @brief Define el mapa de iluminación emisiva. @param texture Puntero a la textura. */
	void setEmissive(Texture* texture) { m_emissive = texture; }

	/** @brief Obtiene el material base. @return Puntero al material. */
	Material* getMaterial() const { return m_material; }
	/** @brief Obtiene la textura de Albedo. @return Puntero a la textura. */
	Texture* getAlbedo() const { return m_albedo; }
	/** @brief Obtiene el mapa de normales. @return Puntero a la textura. */
	Texture* getNormal() const { return m_normal; }
	/** @brief Obtiene el mapa de metalicidad. @return Puntero a la textura. */
	Texture* getMetallic() const { return m_metallic; }
	/** @brief Obtiene el mapa de rugosidad. @return Puntero a la textura. */
	Texture* getRoughness() const { return m_roughness; }
	/** @brief Obtiene el mapa de oclusión ambiental. @return Puntero a la textura. */
	Texture* getAO() const { return m_ao; }
	/** @brief Obtiene el mapa de emisión. @return Puntero a la textura. */
	Texture* getEmissive() const { return m_emissive; }

	/** @brief Accede a los parámetros del material. @return Referencia a MaterialParams. */
	MaterialParams& getParams() { return m_params; }
	/** @brief Accede a los parámetros del material (lectura). @return Referencia constante a MaterialParams. */
	const MaterialParams& getParams() const { return m_params; }

	/**
	 * @brief Vincula los recursos de textura al pipeline de DirectX 11.
	 * @param deviceContext Contexto del dispositivo para realizar el binding de los recursos de sombreado.
	 */
	void bindTextures(DeviceContext& deviceContext) const;

private:
	Material* m_material = nullptr;      /**< Puntero al recurso de material base asociado. */
	Texture* m_albedo = nullptr;        /**< Mapa de color base. */
	Texture* m_normal = nullptr;        /**< Mapa de normales para detalle de superficie. */
	Texture* m_metallic = nullptr;      /**< Mapa de propiedades metálicas. */
	Texture* m_roughness = nullptr;     /**< Mapa de rugosidad. */
	Texture* m_ao = nullptr;            /**< Mapa de oclusión ambiental. */
	Texture* m_emissive = nullptr;      /**< Mapa para efectos de auto-iluminación. */
	MaterialParams m_params;            /**< Estructura que almacena los parámetros físicos del material. */
};