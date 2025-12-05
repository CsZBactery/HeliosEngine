#pragma once
#include "Prerequisites.h"

/**
 * @enum ResourceType
 * @brief Categorías de recursos disponibles en el motor.
 *
 * Ayuda al ResourceManager a clasificar y almacenar los activos en los contenedores correctos.
 */
enum class ResourceType {
    Unknown,    ///< Tipo no definido o error.
    Model3D,    ///< Mallas geométricas (.obj, .fbx).
    Texture,    ///< Imágenes y texturas (.png, .dds, .jpg).
    Sound,      ///< Archivos de audio.
    Shader,     ///< Programas de sombreado (.fx, .hlsl).
    Material    ///< Definiciones de superficies y propiedades ópticas.
};

/**
 * @enum ResourceState
 * @brief Máquina de estados para el ciclo de vida de un recurso.
 *
 * Útil para la carga asíncrona:
 * - Unloaded: No está en memoria.
 * - Loading: Se está leyendo del disco (thread secundario).
 * - Loaded: Listo para usarse.
 * - Failed: Hubo un error (archivo no encontrado, formato inválido).
 */
enum class ResourceState {
    Unloaded,
    Loading,
    Loaded,
    Failed
};

/**
 * @class IResource
 * @brief Interfaz base (Clase Padre) para todos los recursos gestionables.
 *
 * Define el contrato que deben cumplir todos los assets (Texturas, Modelos, Audio).
 * Proporciona un sistema de identificación único (ID), gestión de nombres y estados.
 *
 * Cualquier clase que quiera ser gestionada por el ResourceManager debe heredar de esta.
 */
class IResource {
public:
    /**
     * @brief Constructor.
     * @param name Nombre único o identificador del recurso (suele ser el nombre del archivo sin ruta).
     */
    IResource(const std::string& name)
        : m_name(name)
        , m_filePath("")
        , m_type(ResourceType::Unknown)
        , m_state(ResourceState::Unloaded)
        , m_id(GenerateID())
    {
    }

    /**
     * @brief Destructor virtual.
     */
    virtual ~IResource() = default;

    /**
     * @brief Inicializa el recurso en la API Gráfica (GPU).
     *
     * Este paso toma los datos cargados en RAM (por load) y crea los buffers,
     * texturas o vistas en DirectX/OpenGL.
     *
     * @return true si la creación en GPU fue exitosa.
     */
    virtual bool init() = 0;

    /**
     * @brief Carga los datos crudos desde el disco duro a la memoria RAM.
     *
     * @param filename Ruta absoluta o relativa al archivo.
     * @return true si el archivo se leyó correctamente.
     */
    virtual bool load(const std::string& filename) = 0;

    /**
     * @brief Libera la memoria (RAM y VRAM) y resetea el estado a Unloaded.
     */
    virtual void unload() = 0;

    /**
     * @brief Devuelve el tamaño estimado del recurso en bytes.
     *
     * Útil para herramientas de perfilado (Profiler) y gestión de presupuesto de memoria.
     * @return Tamaño en bytes (size_t).
     */
    virtual size_t getSizeInBytes() const = 0;

    // ------------------------------------------------------------------------
    // SETTERS
    // ------------------------------------------------------------------------

    /** @brief Establece la ruta del archivo original. */
    void SetPath(const std::string& path) { m_filePath = path; }

    /** @brief Establece el tipo de recurso (generalmente en el constructor de la clase hija). */
    void SetType(ResourceType t) { m_type = t; }

    /** @brief Actualiza el estado actual del recurso (Cargando, Cargado, Error). */
    void SetState(ResourceState s) { m_state = s; }

    // ------------------------------------------------------------------------
    // GETTERS
    // ------------------------------------------------------------------------

    /** @brief Obtiene el nombre del recurso. */
    const std::string& GetName() const { return m_name; }

    /** @brief Obtiene la ruta del archivo en disco. */
    const std::string& GetPath() const { return m_filePath; }

    /** @brief Obtiene el tipo de recurso. */
    ResourceType GetType() const { return m_type; }

    /** @brief Obtiene el estado actual de carga. */
    ResourceState GetState() const { return m_state; }

    /** @brief Obtiene el ID único generado automáticamente. */
    uint64_t GetID() const { return m_id; }

protected:
    std::string m_name;      ///< Nombre identificativo.
    std::string m_filePath;  ///< Ruta completa del archivo.
    ResourceType m_type;     ///< Tipo de asset.
    ResourceState m_state;   ///< Estado actual.
    uint64_t m_id;           ///< Identificador numérico único.

private:
    /**
     * @brief Generador de IDs secuenciales.
     *
     * Método estático interno que asegura que cada recurso tenga un número único
     * durante la ejecución del programa.
     * @return Nuevo ID.
     */
    static uint64_t GenerateID()
    {
        static uint64_t nextID = 1;
        return nextID++;
    }
};