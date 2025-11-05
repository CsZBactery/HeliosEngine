#pragma once
#include "Prerequisites.h"
#include "InputLayout.h"
#include <string>
#include <vector>

/**
 * @file ShaderProgram.h
 * @brief Declaración de la clase @c ShaderProgram, un contenedor de shaders HLSL para Direct3D 11.
 *
 * Proporciona utilidades para compilar (desde archivo o cadena en memoria),
 * crear y enlazar shaders de vértices y de píxeles, así como generar el
 * Input Layout asociado a la firma del VS.
 */

 // Forward correcto (NO uses ID3D10Blob)

class Device;
class DeviceContext;

/**
 * @class ShaderProgram
 * @brief Gestiona la vida de un par de shaders (VS/PS) y su @c InputLayout en D3D11.
 *
 * Esta clase permite:
 * - Compilar y crear shaders desde archivo (.hlsl/.fx) o desde código fuente en memoria.
 * - Crear el @c ID3D11InputLayout a partir del blob del Vertex Shader.
 * - Enlazar (render) los shaders y el input layout al pipeline.
 * - Liberar recursos asociados (destroy).
 */
class ShaderProgram {
public:
    /**
     * @brief Constructor por defecto.
     */
    ShaderProgram() = default;

    /**
     * @brief Destructor por defecto.
     */
    ~ShaderProgram() = default;

    /**
     * @brief Inicializa el programa de shaders a partir de un archivo HLSL/.fx.
     *
     * Compila y crea el Vertex Shader (VS), genera el @c InputLayout usando
     * la firma del VS y, posteriormente, compila y crea el Pixel Shader (PS).
     *
     * @param device Dispositivo D3D11 para la creación de recursos.
     * @param fileName Ruta al archivo fuente HLSL/.fx.
     * @param Layout Descriptores de entrada (atributos de vértice) para el Input Layout.
     * @return @c S_OK en éxito, o un @c HRESULT de error en caso contrario.
     */
    HRESULT init(Device& device,
        const std::string& fileName,
        std::vector<D3D11_INPUT_ELEMENT_DESC> Layout);

    /**
     * @brief Inicializa el programa de shaders compilando desde una cadena HLSL en memoria.
     *
     * Compila el VS y PS desde el buffer proporcionado, crea los objetos
     * @c ID3D11VertexShader / @c ID3D11PixelShader y genera el @c InputLayout
     * a partir del blob del VS.
     *
     * @param device Dispositivo D3D11 para la creación de recursos.
     * @param hlslSource Código fuente HLSL a compilar.
     * @param Layout Descriptores de entrada (atributos de vértice) para el Input Layout.
     * @return @c S_OK en éxito, o un @c HRESULT de error en caso contrario.
     */
    HRESULT initFromSource(Device& device,
        const std::string& hlslSource,
        const std::vector<D3D11_INPUT_ELEMENT_DESC>& Layout);

    /**
     * @brief Enlaza el VS, PS e Input Layout al pipeline gráfico.
     *
     * @param deviceContext Contexto D3D11 donde se asignarán los shaders.
     */
    void    render(DeviceContext& deviceContext);

    /**
     * @brief Enlaza únicamente el shader indicado (VS o PS) al pipeline.
     *
     * @param deviceContext Contexto D3D11 donde se asignará el shader.
     * @param type Tipo de shader a enlazar (VERTEX_SHADER o PIXEL_SHADER).
     */
    void    render(DeviceContext& deviceContext, ShaderType type);

    /**
     * @brief Libera todos los recursos asociados (VS, PS, blobs e Input Layout).
     */
    void    destroy();

    /**
     * @brief Crea el @c InputLayout a partir del blob del Vertex Shader.
     *
     * @note Requiere que @c m_vertexShaderData esté disponible (previamente
     *       configurado tras compilar/crear el VS).
     *
     * @param device Dispositivo D3D11.
     * @param Layout Descriptores de entrada a registrar en el Input Layout.
     * @return @c S_OK en éxito, o un @c HRESULT de error en caso contrario.
     */
    HRESULT CreateInputLayout(Device& device,
        std::vector<D3D11_INPUT_ELEMENT_DESC> Layout);

    /**
     * @brief Compila y crea el shader indicado (VS o PS) usando el archivo establecido en @c m_shaderFileName.
     *
     * @param device Dispositivo D3D11.
     * @param type Tipo de shader a crear (VERTEX_SHADER o PIXEL_SHADER).
     * @return @c S_OK en éxito, o un @c HRESULT de error en caso contrario.
     */
    HRESULT CreateShader(Device& device, ShaderType type);

    /**
     * @brief Compila y crea el shader indicado tomando el archivo especificado.
     *
     * Establece @c m_shaderFileName al valor de @p fileName y delega en @c CreateShader.
     *
     * @param device Dispositivo D3D11.
     * @param type Tipo de shader a crear (VERTEX_SHADER o PIXEL_SHADER).
     * @param fileName Ruta al archivo HLSL/.fx.
     * @return @c S_OK en éxito, o un @c HRESULT de error en caso contrario.
     */
    HRESULT CreateShader(Device& device, ShaderType type, const std::string& fileName);

    /**
     * @brief Compila un shader HLSL desde archivo usando D3DCompile (sin D3DCompileFromFile).
     *
     * @param szFileName Ruta wide del archivo HLSL/.fx.
     * @param szEntryPoint Nombre de la función de entrada (ej. "VS" o "PS").
     * @param szShaderModel Perfil de compilación (ej. "vs_4_0", "ps_4_0").
     * @param ppBlobOut [out] Blob resultante con el bytecode compilado.
     * @return @c S_OK en éxito, o un @c HRESULT de error en caso contrario.
     */
    HRESULT CompileShaderFromFile(const wchar_t* szFileName,
        const char* szEntryPoint,
        const char* szShaderModel,
        ID3DBlob** ppBlobOut);

    /**
     * @brief Compila un shader HLSL desde un buffer en memoria usando D3DCompile.
     *
     * @param source Puntero al código HLSL en memoria.
     * @param length Tamaño en bytes del código fuente.
     * @param szEntryPoint Nombre de la función de entrada (ej. "VS" o "PS").
     * @param szShaderModel Perfil de compilación (ej. "vs_4_0", "ps_4_0").
     * @param ppBlobOut [out] Blob resultante con el bytecode compilado.
     * @return @c S_OK en éxito, o un @c HRESULT de error en caso contrario.
     */
    HRESULT CompileShaderFromMemory(const char* source, size_t length,
        const char* szEntryPoint,
        const char* szShaderModel,
        ID3DBlob** ppBlobOut);

public:
    ID3D11VertexShader* m_VertexShader = nullptr;  /**< Shader de vértices (VS). */
    ID3D11PixelShader* m_PixelShader = nullptr;  /**< Shader de píxeles (PS).  */
    InputLayout         m_inputLayout;             /**< Input Layout asociado al VS. */

private:
    std::string m_shaderFileName;      /**< Ruta del archivo HLSL/.fx actual (si aplica). */
    ID3DBlob* m_vertexShaderData = nullptr; /**< Blob con bytecode del VS (usado para crear el Input Layout). */
    ID3DBlob* m_pixelShaderData = nullptr; /**< Blob con bytecode del PS (opcional, para depuración/inspección). */
};
