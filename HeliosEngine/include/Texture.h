#pragma once
#include "Prerequisites.h"
#include <string>

// Forward declarations de otros wrappers que usas con Texture.
class Device;
class DeviceContext;

/**
 * @brief Tipos de origen/extension de imagen soportados al cargar desde archivo.
 * @note Ajusta/expande seg�n tu pipeline real. Si ya lo defines en otro .h,
 *       elimina este enum aqu� y #include ese archivo.
 */
enum class ExtensionType {
  DDS,     ///< Texturas en formato DDS (ruta habitual en DX)
  WIC,     ///< PNG/JPG/BMP/etc. v�a WIC
  Unknown  ///< Desconocido/no especificado
};

/**
 * @class Texture
 * @brief Wrapper m�nimo para manejar un ID3D11Texture2D y su SRV asociado.
 *
 * Provee utilidades para:
 * - Crear una textura a partir de archivo o dimensiones.
 * - Envolver una textura existente (ej.: backbuffer del swap chain).
 * - Hacer bind de la SRV al pipeline de p�xeles.
 * - Liberar correctamente los recursos.
 *
 * @attention Esta clase NO gestiona RTV/DSV; s�lo textura y SRV.
 */
class Texture {
public:
  /// Ctor/Dtor triviales (la liberaci�n real ocurre en destroy()).
  Texture() = default;
  ~Texture() = default;

  /**
   * @brief Inicializa la textura cargando desde un archivo en disco.
   * @param device        Dispositivo D3D11 a usar para crear recursos.
   * @param textureName   Ruta o nombre del archivo (relativa o absoluta).
   * @param extensionType Tipo de extensi�n/origen (DDS/WIC).
   * @return S_OK en �xito, c�digo de error HRESULT en fallo.
   *
   * @details
   * - Para @c ExtensionType::DDS se espera cargar v�a rutina DDS.
   * - Para @c ExtensionType::WIC se espera cargar v�a WIC (PNG/JPG/BMP, etc.).
   * - Si necesitas mipmaps, flags de bind, etc., a�ade par�metros u
   *   ofrece setters antes de crear la SRV.
   */
  HRESULT init(Device& device,
    const std::string& textureName,
    ExtensionType extensionType);

  /**
   * @brief Crea una textura 2D a partir de dimensiones y formato (sin datos).
   * @param device        Dispositivo D3D11.
   * @param width         Ancho en texels.
   * @param height        Alto en texels.
   * @param format        Formato DXGI (p.ej. DXGI_FORMAT_R8G8B8A8_UNORM).
   * @param BindFlags     Flags de enlace (p.ej. D3D11_BIND_SHADER_RESOURCE).
   * @param sampleCount   Multisampling sample count (1 si no MSAA).
   * @param qualityLevels Niveles de calidad MSAA (0 si no MSAA).
   * @return S_OK en �xito, HRESULT en fallo.
   *
   * @note Esta variante crea la textura vac�a; si quieres inicializar datos,
   *       usa un UpdateSubresource posterior o a�ade un par�metro con datos.
   */
  HRESULT init(Device& device,
    unsigned int width,
    unsigned int height,
    DXGI_FORMAT format,
    unsigned int BindFlags,
    unsigned int sampleCount = 1,
    unsigned int qualityLevels = 0);

  /**
   * @brief Enlaza una textura ya existente para exponer una SRV con @p format.
   * @param device      Dispositivo D3D11.
   * @param textureRef  Otra @c Texture que ya contiene un @c ID3D11Texture2D.
   * @param format      Formato de la SRV (debe ser compatible con la textura).
   * @return S_OK en �xito, HRESULT en fallo.
   *
   * @details �til para envolver el backbuffer del swap chain y crear una SRV
   *          de lectura (si el formato/flags lo permiten).
   */
  HRESULT init(Device& device,
    Texture& textureRef,
    DXGI_FORMAT format);

  /**
   * @brief Punto de extensi�n para actualizar l�gica asociada a la textura.
   * @details Vac�o por defecto; �salo si implementas streaming, anim, etc.
   */
  void update();

  /**
   * @brief Hace bind de la SRV al PS (pixel shader).
   * @param deviceContext  Contexto inmediato (o diferido) a usar.
   * @param StartSlot      Primer slot de SRV en el PS (t0, t1, ...).
   * @param NumViews       N�mero de vistas a bindear (normalmente 1).
   *
   * @note Internamente llama a @c ID3D11DeviceContext::PSSetShaderResources.
   * @warning Debes haber creado @c m_textureFromImg (SRV) previamente.
   */
  void render(DeviceContext& deviceContext,
    unsigned int StartSlot,
    unsigned int NumViews);

  /**
   * @brief Libera los recursos de D3D11 (SRV + Texture2D).
   * @details Deja los punteros a @c nullptr.
   */
  void destroy();

public: // Miembros expuestos por simplicidad (puedes encapsularlos si prefieres)
  /**
   * @brief Recurso base de textura 2D.
   * @details Propiedad: se libera en @c destroy().
   */
  ID3D11Texture2D* m_texture = nullptr;

  /**
   * @brief Shader Resource View asociada a la textura (para lectura en shaders).
   * @details Propiedad: se libera en @c destroy().
   */
  ID3D11ShaderResourceView* m_textureFromImg = nullptr;

  /**
   * @brief Nombre/Ruta de la textura (si se carg� desde archivo).
   * @details Se usa s�lo como metadato informativo.
   */
  std::string m_textureName;
};