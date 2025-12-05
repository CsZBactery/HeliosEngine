#pragma once
#include "Prerequisites.h"
#include "InputLayout.h"

class Device;
class DeviceContext;

/**
 * @class ShaderProgram
 * @brief Gestiona un programa de sombreado completo (Vertex Shader + Pixel Shader).
 *
 * Esta clase se encarga de:
 * 1. Leer archivos de código HLSL (.fx, .hlsl).
 * 2. Compilarlos en tiempo de ejecución (o cargar binarios).
 * 3. Crear los objetos VertexShader y PixelShader en la GPU.
 * 4. Gestionar el Input Layout que conecta los vértices de C++ con el Shader.
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
     * @brief Inicializa el programa de shaders completo.
     *
     * Compila los shaders desde un archivo y genera el Input Layout.
     *
     * @param device Referencia al dispositivo (Factory).
     * @param fileName Nombre o ruta del archivo de shader (.fx / .hlsl).
     * @param Layout Vector con la descripción del formato de vértices (Posición, UV, Normal, etc.).
     * @return HRESULT S_OK si todo se compiló y cargó correctamente.
     */
    HRESULT
        init(Device& device,
            const std::string& fileName,
            std::vector < D3D11_INPUT_ELEMENT_DESC> Layout);

    /**
     * @brief Actualiza la lógica del shader (Placeholder).
     */
    void
        update();

    /**
     * @brief Activa ambos shaders (Vertex y Pixel) y el Input Layout en el pipeline.
     *
     * @param deviceContext Contexto donde se realizará el renderizado.
     */
    void
        render(DeviceContext& deviceContext);

    /**
     * @brief Activa SOLAMENTE un tipo de shader específico (VS o PS).
     *
     * Útil si, por ejemplo, quieres desactivar el Pixel Shader para un pase de solo profundidad.
     *
     * @param deviceContext Contexto de renderizado.
     * @param type Tipo de shader a activar (ShaderType::Vertex o ShaderType::Pixel).
     */
    void
        render(DeviceContext& deviceContext, ShaderType type);

    /**
     * @brief Libera la memoria de los shaders y los blobs de datos.
     */
    void
        destroy();

    /**
     * @brief Crea el Input Layout basado en la firma del Vertex Shader compilado.
     *
     * @param device Dispositivo DirectX.
     * @param Layout Descripción de los elementos del vértice.
     * @return HRESULT S_OK si la creación es exitosa.
     */
    HRESULT
        CreateInputLayout(Device& device,
            std::vector<D3D11_INPUT_ELEMENT_DESC> Layout);

    /**
     * @brief Crea el objeto de shader en la GPU a partir de los datos compilados internos.
     *
     * @param device Dispositivo DirectX.
     * @param type Tipo de shader a crear (Vertex o Pixel).
     * @return HRESULT S_OK si la creación es exitosa.
     */
    HRESULT
        CreateShader(Device& device, ShaderType type);

    /**
     * @brief Crea un shader compilándolo desde un archivo específico (sobrecarga).
     *
     * @param device Dispositivo DirectX.
     * @param type Tipo de shader.
     * @param fileName Ruta del archivo.
     * @return HRESULT Resultado de la operación.
     */
    HRESULT
        CreateShader(Device& device, ShaderType type, const std::string& fileName);

    /**
     * @brief Compila el código HLSL desde un archivo en disco.
     *
     * Utiliza la API D3DCompileFromFile. Es el paso crítico que convierte el texto
     * del shader en código binario (bytecode) que la GPU entiende.
     *
     * @param szFileName Ruta del archivo.
     * @param szEntryPoint Nombre de la función principal en el shader (ej: "VS", "PS").
     * @param szShaderModel Versión del shader model (ej: "vs_4_0", "ps_4_0").
     * @param ppBlobOut Puntero doble donde se almacenará el código binario compilado.
     * @return HRESULT S_OK si compila sin errores.
     */
    HRESULT
        CompileShaderFromFile(char* szFileName,
            LPCSTR szEntryPoint,
            LPCSTR szShaderModel,
            ID3DBlob** ppBlobOut);

public:
    /// Puntero al Vertex Shader en la GPU.
    ID3D11VertexShader* m_VertexShader = nullptr;

    /// Puntero al Pixel Shader en la GPU.
    ID3D11PixelShader* m_PixelShader = nullptr;

    /// Objeto que define el formato de los datos de entrada.
    InputLayout m_inputLayout;

private:

    /// Nombre del archivo del shader cargado.
    std::string m_shaderFileName;

    /// Blob de datos binarios del Vertex Shader (necesario para el Input Layout).
    ID3DBlob* m_vertexShaderData = nullptr;

    /// Blob de datos binarios del Pixel Shader.
    ID3DBlob* m_pixelShaderData = nullptr;

};