#include "../include/ModelLoader.h"
#include "../include/OBJ_Loader.h"
void
ModelLoader::init()
{
    // Inicializa cualquier recurso necesario antes de cargar modelos.
}

void
ModelLoader::update()
{
    // Actualiza el estado interno del cargador si es necesario.
}

void
ModelLoader::render()
{
    // Renderiza el modelo cargado (si se implementa un render directo).
}

void
ModelLoader::destroy()
{
    // Libera los recursos usados por el cargador o modelos cargados.
}

LoadData
ModelLoader::Load(std::string objFileName)
{
    LoadData LD;                       // Estructura donde se almacenar�n los datos del modelo.
    objl::Loader m_loader;              // Instancia temporal del cargador OBJ.

    // Intenta cargar el archivo OBJ especificado.
    if (!m_loader.LoadFile(objFileName)) {
        ERROR("ModelLoader", "Load", "No se pudo cargar el archivo .obj");
        return LD;                       // Retorna estructura vac�a si la carga falla.
    }

    LD.name = objFileName;              // Guarda el nombre del modelo cargado.

    size_t vertexCount = m_loader.LoadedVertices.size();  // N�mero total de v�rtices cargados.
    LD.vertex.resize(vertexCount);      // Redimensiona el vector de v�rtices.

    // Recorre todos los v�rtices cargados y copia su informaci�n.
    for (int i = 0; i < LD.vertex.size(); i++)
    {
        LD.vertex[i].Pos.x = m_loader.LoadedVertices[i].Position.X;
        LD.vertex[i].Pos.y = m_loader.LoadedVertices[i].Position.Y;
        LD.vertex[i].Pos.z = m_loader.LoadedVertices[i].Position.Z;

        LD.vertex[i].Tex.x = m_loader.LoadedVertices[i].TextureCoordinate.X;
        LD.vertex[i].Tex.y = m_loader.LoadedVertices[i].TextureCoordinate.Y;

        LD.vertex[i].Normal.x = m_loader.LoadedVertices[i].Normal.X;
        LD.vertex[i].Normal.y = m_loader.LoadedVertices[i].Normal.Y;
        LD.vertex[i].Normal.z = m_loader.LoadedVertices[i].Normal.Z;
    }

    size_t indexCount = m_loader.LoadedIndices.size();   // N�mero total de �ndices cargados.
    LD.index.resize(indexCount);                         // Redimensiona el vector de �ndices.
    LD.index = m_loader.LoadedIndices;                   // Copia los �ndices cargados.

    LD.numVertex = (int)vertexCount;                     // Guarda la cantidad de v�rtices.
    LD.numIndex = (int)indexCount;                       // Guarda la cantidad de �ndices.

    return LD;                                           // Retorna la estructura con los datos del modelo cargado.
}