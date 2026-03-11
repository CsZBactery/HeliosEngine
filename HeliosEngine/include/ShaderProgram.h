#pragma once
#include "Prerequisites.h"
#include "InputLayout.h"

/**
 * @file ShaderProgram.h
 * @brief Orquestador de Shaders y vinculación con el Pipeline Gráfico.
 */

class Device;
class DeviceContext;
class LayoutBuilder;

/**
 * @class ShaderProgram
 * @brief Administra el ciclo de vida conjunto del Vertex Shader y Pixel Shader.
 * * HeliosEngine utiliza esta clase para:
 * 1. Cargar y compilar código fuente HLSL (.fx / .hlsl) en tiempo de ejecución.
 * 2. Generar el Bytecode (binario de GPU) necesario para los shaders.
 * 3. Gestionar el Input Layout, que define cómo se mapean los datos de los vértices
 * desde C++ hacia los registros del Vertex Shader.
 */
class
	ShaderProgram {
public:
	/** @brief Constructor por defecto. */
	ShaderProgram() = default;

	/** @brief Destructor por defecto. Libera memoria mediante destroy(). */
	~ShaderProgram() = default;

	/**
	 * @brief Inicializa el programa de shaders y el layout de entrada.
	 * * @param device Dispositivo DirectX para la creación de recursos.
	 * @param fileName Ruta del archivo que contiene el código HLSL.
	 * @param layoutBuilder Objeto encargado de definir la estructura de los vértices.
	 * @return HRESULT S_OK si la compilación y vinculación fueron exitosas.
	 */
	HRESULT
		init(Device& device, const std::string& fileName, LayoutBuilder layoutBuilder);

	/** @brief Actualización lógica de parámetros del programa (Placeholder). */
	void
		update();

	/**
	 * @brief Vincula el Vertex Shader, Pixel Shader y el Input Layout al contexto.
	 * @param deviceContext Contexto donde se aplicarán los cambios del pipeline.
	 */
	void
		render(DeviceContext& deviceContext);

	/**
	 * @brief Vincula únicamente un tipo de shader específico al contexto.
	 * * Útil para pases de renderizado que solo requieren procesamiento de vértices (como Shadow Mapping).
	 * * @param deviceContext Contexto de ejecución.
	 * @param type Especifica si se activa el Vertex Shader o el Pixel Shader.
	 */
	void
		render(DeviceContext& deviceContext, ShaderType type);

	/**
	 * @brief Libera los Shaders, los Blobs de datos y el Input Layout de la memoria.
	 */
	void
		destroy();

	/**
	 * @brief Crea el Input Layout basándose en la firma del Vertex Shader.
	 * @param device Dispositivo DirectX.
	 * @param layoutBuilder Constructor con la descripción de los elementos del vértice.
	 * @return HRESULT S_OK si el layout se creó correctamente.
	 */
	HRESULT
		CreateInputLayout(Device& device, LayoutBuilder layoutBuilder);

	/**
	 * @brief Crea un objeto Shader en la GPU usando el Bytecode cargado internamente.
	 * @param device Dispositivo DirectX.
	 * @param type Tipo de shader a crear (Vertex o Pixel).
	 */
	HRESULT
		CreateShader(Device& device, ShaderType type);

	/**
	 * @brief Compila y crea un shader desde un archivo específico (Sobrecarga).
	 * @param device Dispositivo DirectX.
	 * @param type Tipo de shader.
	 * @param fileName Ruta del archivo de código.
	 */
	HRESULT
		CreateShader(Device& device, ShaderType type, const std::string& fileName);

	/**
	 * @brief Compila código HLSL puro en Bytecode binario.
	 * * Este es el proceso de traducción de alto nivel (HLSL) a lenguaje de microcódigo de GPU.
	 * * @param szFileName Ruta del archivo.
	 * @param szEntryPoint Nombre de la función principal (ej: "VSMain" o "PSMain").
	 * @param szShaderModel Versión del perfil (ej: "vs_5_0" o "ps_5_0").
	 * @param ppBlobOut Contenedor para el binario resultante.
	 */
	HRESULT
		CompileShaderFromFile(char* szFileName,
			LPCSTR szEntryPoint,
			LPCSTR szShaderModel,
			ID3DBlob** ppBlobOut);

public:
	/** @brief Puntero al objeto Vertex Shader en la GPU. */
	ID3D11VertexShader* m_VertexShader = nullptr;

	/** @brief Puntero al objeto Pixel Shader en la GPU. */
	ID3D11PixelShader* m_PixelShader = nullptr;

	/** @brief Estructura que describe la entrada de datos al Vertex Shader. */
	InputLayout m_inputLayout;

private:
	/** @brief Ruta del archivo cargado actualmente. */
	std::string m_shaderFileName;

	/** @brief Binario compilado del Vertex Shader (requerido para validar el Layout). */
	ID3DBlob* m_vertexShaderData = nullptr;

	/** @brief Binario compilado del Pixel Shader. */
	ID3DBlob* m_pixelShaderData = nullptr;
};