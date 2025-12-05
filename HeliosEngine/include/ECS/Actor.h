#pragma once
#include "Prerequisites.h"
#include "Entity.h"
#include "Buffer.h"
#include "Texture.h"
#include "Transform.h"
#include "SamplerState.h"
//#include "Rasterizer.h"
//#include "BlendState.h"
#include "ShaderProgram.h"
//#include "DepthStencilState.h"

class Device;
class DeviceContext;
class MeshComponent;

/**
 * @class Actor
 * @brief Representa una entidad renderizable en la escena 3D.
 *
 * Hereda de @c Entity (lo que le da una posición, rotación y escala).
 * El Actor es el responsable de tomar la información geométrica (Mallas)
 * y visual (Texturas), cargarla en la GPU (Buffers) y ejecutar los comandos
 * de dibujo (Draw Calls) durante el frame.
 *
 * Además, incluye soporte para proyección de sombras (Shadow Mapping).
 */
class Actor : public Entity {
public:
    /**
     * @brief Constructor por defecto.
     */
    Actor() = default;

    /**
     * @brief Constructor que inicializa el actor y sus recursos internos.
     *
     * @param device Dispositivo necesario para crear los Constant Buffers iniciales (matrices).
     */
    Actor(Device& device);

    /**
     * @brief Destructor virtual.
     */
    virtual ~Actor() = default;

    /**
     * @brief Inicialización lógica (Heredada de Entity).
     */
    void
        init() override {}

    /**
     * @brief Actualiza la lógica del actor y sus buffers en la GPU.
     *
     * Calcula la matriz de mundo (World Matrix) basada en la transformación actual
     * y actualiza el Constant Buffer correspondiente para que el Shader sepa dónde dibujar el objeto.
     *
     * @param deltaTime Tiempo transcurrido desde el último frame.
     * @param deviceContext Contexto para actualizar los subrecursos (Buffers).
     */
    void
        update(float deltaTime, DeviceContext& deviceContext) override;

    /**
     * @brief Dibuja el objeto en la pantalla (Pase principal).
     *
     * 1. Vincula los Vertex/Index Buffers al Input Assembler.
     * 2. Vincula las texturas y Samplers al Pixel Shader.
     * 3. Ejecuta DrawIndexed.
     *
     * @param deviceContext Contexto para emitir comandos de dibujo.
     */
    void
        render(DeviceContext& deviceContext) override;

    /**
     * @brief Libera la memoria de todos los recursos gráficos (Buffers, Texturas, Shaders propios).
     */
    void
        destroy();

    /**
     * @brief Asigna la geometría al actor y crea los buffers en GPU.
     *
     * Toma los datos crudos de las mallas (vértices e índices) y crea
     * los `VertexBuffers` e `IndexBuffers` necesarios en la tarjeta gráfica.
     *
     * @param device Dispositivo para la creación de buffers.
     * @param meshes Lista de componentes de malla (submeshes).
     */
    void
        setMesh(Device& device, std::vector<MeshComponent> meshes);

    /**
     * @brief Obtiene el nombre identificativo del actor.
     * @return String con el nombre.
     */
    std::string
        getName() { return m_name; }

    /**
     * @brief Asigna un nombre al actor (útil para depuración o UI).
     * @param name Nuevo nombre.
     */
    void
        setName(const std::string& name) { m_name = name; }

    /**
     * @brief Asigna las texturas que "vestirán" al modelo.
     * @param textures Vector de objetos Texture ya cargados.
     */
    void
        setTextures(std::vector<Texture> textures) { m_textures = textures; }

    /**
     * @brief Activa o desactiva la capacidad de este objeto de proyectar sombras.
     * @param v true para proyectar sombra, false para ser ignorado por el mapa de sombras.
     */
    void
        setCastShadow(bool v) { castShadow = v; }

    /**
     * @brief Consulta si el objeto proyecta sombras.
     */
    bool
        canCastShadow() const { return castShadow; }

    /**
     * @brief Renderizado especial para el pase de Mapa de Sombras (Shadow Map).
     *
     * Dibuja el objeto desde la perspectiva de la luz, usando un Shader simplificado
     * que solo escribe profundidad.
     *
     * @param deviceContext Contexto de renderizado.
     */
    void
        renderShadow(DeviceContext& deviceContext);

private:
    // ------------------------------------------------------------------------
    // DATOS DE GEOMETRÍA Y MATERIAL
    // ------------------------------------------------------------------------
    std::vector<MeshComponent> m_meshes;        ///< Datos de la malla en CPU.
    std::vector<Texture>       m_textures;      ///< Texturas (Albedo, Normal, etc.).

    // ------------------------------------------------------------------------
    // RECURSOS GPU (HANDLES)
    // ------------------------------------------------------------------------
    std::vector<Buffer>        m_vertexBuffers; ///< Buffers de vértices en VRAM.
    std::vector<Buffer>        m_indexBuffers;  ///< Buffers de índices en VRAM.

    //BlendState m_blendstate;                  ///< (Comentado) Estado de mezcla para transparencias.
    //Rasterizer m_rasterizer;                  ///< (Comentado) Estado de raster (Cull mode, Wireframe).

    SamplerState               m_sampler;       ///< Cómo se leen las texturas (Filtro Bilineal/Wrap).

    CBChangesEveryFrame        m_model;         ///< Estructura CPU con la matriz World.
    Buffer                     m_modelBuffer;   ///< Constant Buffer GPU para la matriz World.

    // ------------------------------------------------------------------------
    // SISTEMA DE SOMBRAS (SHADOW MAPPING)
    // ------------------------------------------------------------------------
    ShaderProgram              m_shaderShadow;            ///< Shader especial para el pase de sombras.
    Buffer                     m_shaderBuffer;            ///< Buffer auxiliar para sombras.
    //BlendState               m_shadowBlendState;        ///< (Comentado) Estado Blend para sombras.
    //DepthStencilState        m_shadowDepthStencilState; ///< (Comentado) Configuración Z-Buffer sombras.
    CBChangesEveryFrame        m_cbShadow;                ///< Datos de transformación para el pase de sombras.

    XMFLOAT4                   m_LightPos;      ///< Posición de la luz (para cálculos de sombra).

    // ------------------------------------------------------------------------
    // PROPIEDADES GENERALES
    // ------------------------------------------------------------------------
    std::string                m_name = "Actor"; ///< Identificador.
    bool                       castShadow = true; ///< Flag de proyección de sombra.
};