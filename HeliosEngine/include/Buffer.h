#pragma once
#include "Prerequisites.h"
#include "MeshComponent.h"

/**
 * @file Buffer.h
 * @brief Clase envoltorio (Wrapper) para gestionar recursos de Buffer en DirectX 11.
 */

class Device;
class DeviceContext;

/**
 * @class Buffer
 * @brief Wrapper genérico para buffers de DirectX 11 (ID3D11Buffer).
 * * Administra la creación, actualización y vinculación de tres tipos de recursos:
 * 1. Vertex Buffers: Almacenan los datos de los vértices (Posición, UV, etc.).
 * 2. Index Buffers: Almacenan el orden en que se conectan los vértices.
 * 3. Constant Buffers: Variables uniformes que cambian cada frame (Matrices, Luces).
 * * @note La instancia gestiona un solo ID3D11Buffer; su función se define en m_bindFlag.
 */
class
	Buffer {
public:
	/** @brief Constructor por defecto. No reserva memoria en GPU. */
	Buffer() = default;

	/** @brief Destructor. No libera automáticamente; llamar a destroy(). */
	~Buffer() = default;

	/**
	 * @brief Inicializa el buffer como Vertex o Index Buffer usando un MeshComponent.
	 * * @param device Dispositivo para crear el recurso.
	 * @param mesh Componente de malla con los datos en RAM (vértices/índices).
	 * @param bindFlag Bandera de enlace (D3D11_BIND_VERTEX_BUFFER o D3D11_BIND_INDEX_BUFFER).
	 * @return S_OK si la creación fue exitosa.
	 */
	HRESULT
		init(Device& device, const MeshComponent& mesh, unsigned int bindFlag);

	/**
	 * @brief Inicializa un buffer de tamaño fijo, típicamente un Constant Buffer.
	 * * @param device Dispositivo para la creación.
	 * @param ByteWidth Tamaño en bytes. Para Constant Buffers, DEBE ser múltiplo de 16.
	 * @return S_OK si el hardware asignó la memoria correctamente.
	 */
	HRESULT
		init(Device& device, unsigned int ByteWidth);

	/**
	 * @brief Actualiza los datos dentro del buffer (UpdateSubresource).
	 * * Envía nuevos datos desde la CPU (RAM) a la GPU (VRAM). Esencial para animaciones y cámaras.
	 * @param deviceContext Contexto para ejecutar la transferencia.
	 * @param pDstResource Recurso destino (usualmente m_buffer).
	 * @param DstSubresource Índice de subrecurso (0 para buffers).
	 * @param pDstBox Región a actualizar (nullptr para todo el buffer).
	 * @param pSrcData Puntero a los datos de origen en RAM.
	 * @param SrcRowPitch Se ignora en buffers 1D.
	 * @param SrcDepthPitch Se ignora en buffers 1D.
	 */
	void
		update(DeviceContext& deviceContext,
			ID3D11Resource* pDstResource,
			unsigned int    DstSubresource,
			const D3D11_BOX* pDstBox,
			const void* pSrcData,
			unsigned int    SrcRowPitch,
			unsigned int    SrcDepthPitch);

	/**
	 * @brief Vincula el buffer a la etapa correspondiente del pipeline.
	 * * El comportamiento varía según m_bindFlag (IA para Vértices/Índices, VS/PS para Constantes).
	 * @param deviceContext Contexto donde se enlaza el buffer.
	 * @param StartSlot Registro o Slot inicial de montaje.
	 * @param NumBuffers Cantidad de buffers a enlazar (típicamente 1).
	 * @param setPixelShader Si es true, enlaza también al Pixel Shader (sólo Constant Buffers).
	 * @param format Formato del índice (Obligatorio para Index Buffers).
	 */
	void
		render(DeviceContext& deviceContext,
			unsigned int    StartSlot,
			unsigned int    NumBuffers,
			bool            setPixelShader = false,
			DXGI_FORMAT     format = DXGI_FORMAT_UNKNOWN);

	/**
	 * @brief Libera el recurso ID3D11Buffer y reinicia los metadatos.
	 */
	void
		destroy();

	/**
	 * @brief Helper interno para invocar D3D11CreateBuffer.
	 * * @param device Envoltorio del Device.
	 * @param desc Configuración del buffer.
	 * @param initData Datos iniciales opcionales.
	 */
	HRESULT
		createBuffer(Device& device,
			D3D11_BUFFER_DESC& desc,
			D3D11_SUBRESOURCE_DATA* initData);

public:
	/** @brief Puntero nativo al buffer de DirectX 11 administrado. */
	ID3D11Buffer* m_buffer = nullptr;

private:
	/** @brief Tamaño de un elemento (sizeof(Vertex)). Requerido para Vertex Buffers. */
	unsigned int m_stride = 0;

	/** @brief Offset inicial en bytes para lectura de vértices. */
	unsigned int m_offset = 0;

	/** @brief Define el rol: Vértice, Índice o Constante. */
	unsigned int m_bindFlag = 0;
};