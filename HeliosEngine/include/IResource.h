#pragma once
#include "Prerequisites.h"
#include <string>
#include <cstdint> 
#include <cstddef> 

// ------------------------------------------------------------
// Tipos de recurso y estado de carga
// ------------------------------------------------------------
enum class ResourceType {
    Unknown = 0,
    Model3D,
    Texture,
    Sound,
    Shader,
    Material
};

enum class ResourceState {
    Unloaded = 0,
    Loading,
    Loaded,
    Failed
};

// ------------------------------------------------------------
// Interfaz base para cualquier recurso del engine
// ------------------------------------------------------------
class IResource {
public:
    // name: nombre lógico del recurso
    explicit IResource(const std::string& name,
        ResourceType type = ResourceType::Unknown)
        : m_name(name)
        , m_type(type)
        , m_state(ResourceState::Unloaded)
        , m_id(GenerateID()) {
    }

    virtual ~IResource() = default;

    // evita duplicar handlers
    IResource(const IResource&) = delete;
    IResource& operator=(const IResource&) = delete;

    // Movible si lo necesitas
    IResource(IResource&&) = default;
    IResource& operator=(IResource&&) = default;

    // ---- API mínima del ciclo de vida ----
    // Crear recursos GPU
    virtual bool init() = 0;

    // Cargar desde disco
    virtual bool load(const std::string& filename) = 0;

    // Liberar memoria/GPU
    virtual void unload() = 0;

    // Para profiler/estadísticas
    virtual size_t getSizeInBytes() const = 0;

    // ---- Getters comunes ----
    const std::string& GetName()   const { return m_name; }
    const std::string& GetPath()   const { return m_filePath; }
    ResourceType       GetType()   const { return m_type; }
    ResourceState      GetState()  const { return m_state; }
    uint64_t           GetID()     const { return m_id; }

protected:
    // Úsalos desde las clases derivadas
    void        SetPath(const std::string& path) { m_filePath = path; }
    void        SetType(ResourceType t) { m_type = t; }
    void        SetState(ResourceState s) { m_state = s; }

protected:
    std::string   m_name;       // nombre lógico 
    std::string   m_filePath;   // ruta en disco
    ResourceType  m_type;       // tipo de recurso
    ResourceState m_state;      // estado de carga
    uint64_t      m_id;         // identificador único

private:
    static uint64_t GenerateID() {
        static uint64_t nextID = 1; 
        return nextID++;
    }
};
