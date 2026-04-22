/**
 * @file ShaderProgram.h
 * @brief Orquestador de Shaders y vinculación con el Pipeline Gráfico.
 * @ingroup core
 */

#pragma once
#include "Prerequisites.h"
#include "InputLayout.h"

class Device;
class DeviceContext;
class LayoutBuilder;

/**
 * @class ShaderProgram
 * @brief Administra el ciclo de vida conjunto del Vertex Shader y Pixel Shader en Direct3D 11.
 * @details HeliosEngine utiliza esta clase para centralizar la creación y el uso de programas
 * de sombreado. Sus funciones principales son:
 * 1. **Carga y Compilación:** Traducir código fuente HLSL (.hlsl) en tiempo de ejecución.
 * 2. **Gestión de Bytecode:** Generar y almacenar los binarios (Blobs) necesarios para la GPU.
 * 3. **Input Layout:** Definir cómo se mapean los datos de los vértices desde C++ hacia
 *    los registros del Vertex Shader.
 */
class ShaderProgram {
public:
    /** @brief Constructor por defecto. No reserva recursos. */
    ShaderProgram() = default;

    /** @brief Destructor. Libera automáticamente los recursos mediante destroy(). */
    ~ShaderProgram() { destroy(); }

    /**
     * @brief Inicializa el programa de shaders y el layout de entrada desde un archivo.
     * @param device Dispositivo DirectX para la creación de recursos.
     * @param fileName Ruta del archivo que contiene el código HLSL.
     * @param layoutBuilder Objeto encargado de definir la estructura de los vértices.
     * @return S_OK si la compilación, creación y vinculación fueron exitosas.
     * @post Si retorna S_OK, los punteros a shaders y el input layout serán válidos.
     */
    HRESULT init(Device& device, const std::string& fileName, LayoutBuilder layoutBuilder);

    /**
     * @brief Actualiza parámetros internos del programa.
     * @note Actualmente es un placeholder para futuras expansiones (como hot-reloading de shaders).
     */
    void update();

    /**
     * @brief Vincula el Vertex Shader, Pixel Shader y el Input Layout al contexto.
     * @details Prepara el pipeline completo para una llamada de dibujo (Draw Call).
     * @param deviceContext Contexto donde se aplicarán los cambios del pipeline.
     * @pre Los shaders deben haberse creado con init() o CreateShader().
     */
    void render(DeviceContext& deviceContext);

    /**
     * @brief Vincula únicamente un tipo de shader específico al contexto.
     * @details Útil para pases de renderizado especializados (ej. Shadow Mapping, que a veces
     * prescinde del Pixel Shader para optimizar).
     * @param deviceContext Contexto de ejecución.
     * @param type Especifica si se activa el Vertex Shader (VS) o el Pixel Shader (PS).
     */
    void render(DeviceContext& deviceContext, ShaderType type);

    /**
     * @brief Libera de forma segura los Shaders, los Blobs de datos y el Input Layout.
     * @post Todos los punteros COM se reinician a nullptr.
     */
    void destroy();

    /**
     * @brief Crea el Input Layout basándose en la firma del Vertex Shader.
     * @param device Dispositivo DirectX.
     * @param layoutBuilder Constructor con la descripción de los elementos del vértice.
     * @return S_OK si el layout se creó y validó correctamente contra el bytecode del VS.
     */
    HRESULT CreateInputLayout(Device& device, LayoutBuilder layoutBuilder);

    /**
     * @brief Crea un objeto Shader en la GPU usando el Bytecode ya cargado internamente.
     * @param device Dispositivo DirectX.
     * @param type Tipo de shader a crear.
     */
    HRESULT CreateShader(Device& device, ShaderType type);

    /**
     * @brief Compila y crea un shader desde un archivo específico (Sobrecarga).
     * @param device Dispositivo DirectX.
     * @param type Tipo de shader (VS o PS).
     * @param fileName Ruta del archivo de código HLSL.
     */
    HRESULT CreateShader(Device& device, ShaderType type, const std::string& fileName);

    /**
     * @brief Compila código HLSL puro en Bytecode binario de GPU.
     * @details Este es el proceso crítico de traducción de lenguaje de alto nivel a
     * lenguaje de microcódigo que la tarjeta de video puede ejecutar.
     * @param szFileName Ruta del archivo.
     * @param szEntryPoint Nombre de la función principal (usualmente "VSMain" o "PSMain").
     * @param szShaderModel Perfil del shader (ej: "vs_5_0" para hardware moderno).
     * @param ppBlobOut Contenedor (ID3DBlob) para el binario resultante.
     */
    HRESULT CompileShaderFromFile(char* szFileName,
        LPCSTR szEntryPoint,
        LPCSTR szShaderModel,
        ID3DBlob** ppBlobOut);

public:
    /** @brief Vertex Shader compilado y creado en la GPU. */
    ID3D11VertexShader* m_VertexShader = nullptr;

    /** @brief Pixel Shader compilado y creado en la GPU. */
    ID3D11PixelShader* m_PixelShader = nullptr;

    /** @brief Interfaz que describe el formato de entrada de datos al Vertex Shader. */
    InputLayout m_inputLayout;

private:
    /** @brief Ruta del archivo HLSL asociado actualmente a este programa. */
    std::string m_shaderFileName;

    /** @brief Binario compilado del Vertex Shader (requerido para validar el Input Layout). */
    ID3DBlob* m_vertexShaderData = nullptr;

    /** @brief Binario compilado del Pixel Shader. */
    ID3DBlob* m_pixelShaderData = nullptr;
};