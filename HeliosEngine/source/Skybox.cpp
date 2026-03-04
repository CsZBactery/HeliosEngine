// ======================================================================================
// Archivo: Skybox.cpp
// Implementación del entorno 3D (Cielo). 
// Utiliza un cubo gigante proyectado alrededor de la cámara.
// ======================================================================================

#include "EngineUtilities/Utilities/Skybox.h" // Ajusta esta ruta a "include/..." si tu VS lo requiere
#include "Device.h"
#include "DeviceContext.h"

// Inicializa la geometría, shaders y buffers necesarios para dibujar el cielo
HRESULT
Skybox::init(Device& device, DeviceContext* deviceContext, Texture& cubemap) {
	destroy();

	// Guardamos la textura del mapa de cubos (las 6 imágenes del cielo)
	m_skyboxTexture = cubemap;

	// 1) GEOMETRÍA DEL CUBO
	// Definimos los 8 vértices de un cubo unitario centrado en el origen (0,0,0).
	const SkyboxVertex vertices[] = {
		{-1,-1,-1}, {-1,+1,-1}, {+1,+1,-1}, {+1,-1,-1}, // Cara trasera (-Z)
		{-1,-1,+1}, {-1,+1,+1}, {+1,+1,+1}, {+1,-1,+1}, // Cara delantera (+Z)
	};

	// Definimos el orden para conectar los puntos y formar los 12 triángulos (36 índices)
	const unsigned int indices[] = {
		0,1,2, 0,2,3, // Atrás (-Z)
		4,6,5, 4,7,6, // Frente (+Z)
		4,5,1, 4,1,0, // Izquierda (-X)
		3,2,6, 3,6,7, // Derecha (+X)
		1,5,6, 1,6,2, // Arriba (+Y)
		4,0,3, 4,3,7  // Abajo (-Y)
	};

	// 2) CREACIÓN DEL ACTOR DEL ENTORNO
	m_skybox = EU::MakeShared<Actor>(device);

	if (!m_skybox.isNull()) {
		std::vector<MeshComponent> skybox;

		// Inyectamos la geometría estática directamente desde la RAM
		m_cubeModel = new Model3D("Skybox", vertices, indices);
		skybox = m_cubeModel->GetMeshes();

		// Asignamos la malla al actor.
		m_skybox->setMesh(device, skybox);
		m_skybox->setName("skybox");
	}
	else {
		ERROR("Skybox", "Init", "Failed to create Skybox Actor.");
		return E_FAIL;
	}

	// 3) CONFIGURACIÓN DE SHADERS (Input Layout)
	// Para el Skybox, el Shader solo necesita saber la Posición 3D (x, y, z).
	std::vector<D3D11_INPUT_ELEMENT_DESC> Layout;
	D3D11_INPUT_ELEMENT_DESC position;
	position.SemanticName = "POSITION";
	position.SemanticIndex = 0;
	position.Format = DXGI_FORMAT_R32G32B32_FLOAT;
	position.InputSlot = 0;
	position.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT; // Automático
	position.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	position.InstanceDataStepRate = 0;
	Layout.push_back(position);

	HRESULT hr = S_OK;

	//  Búsqueda robusta del shader del Skybox 
	hr = m_shaderProgram.init(device, "Assets/Shaders/Skybox.hlsl", Layout);
	if (FAILED(hr)) hr = m_shaderProgram.init(device, "Skybox.hlsl", Layout);
	if (FAILED(hr)) hr = m_shaderProgram.init(device, "Assets/Shaders/Skybox.fx", Layout);
	if (FAILED(hr)) hr = m_shaderProgram.init(device, "Skybox.fx", Layout);

	if (FAILED(hr)) {
		ERROR("Skybox", "init", ("Failed to initialize ShaderProgram. HRESULT: " + std::to_string(hr)).c_str());
		return hr;
	}

	// Buffer Constante para enviarle la matriz de la cámara al Shader
	hr = m_constantBuffer.init(device, sizeof(CBSkybox));
	if (FAILED(hr)) {
		ERROR("Skybox", "init", ("Failed to initialize NeverChanges Buffer. HRESULT: " + std::to_string(hr)).c_str());
		return hr;
	}

	// Sampler: Define cómo se filtra la textura del cielo
	hr = m_samplerState.init(device);
	if (FAILED(hr)) {
		ERROR("Skybox", "init", "Failed to create new SamplerState");
	}

	// 4) CONFIGURACIÓN DE ESTADOS ESPECÍFICOS PARA EL CIELO

	// Rasterizer: CULL_FRONT -> Dibujamos las caras INTERNAS del cubo porque estamos adentro de él.
	hr = m_rasterizerState.init(device, D3D11_FILL_SOLID, D3D11_CULL_FRONT, false, true);
	if (FAILED(hr)) {
		ERROR("Skybox", "init", "Failed to create new RasterizerState");
	}

	// DepthStencil: WRITE_MASK_ZERO -> No escribe profundidad (no tapa a la moto).
	// COMPARISON_LESS_EQUAL -> Asegura que se dibuje en el límite más lejano (Z = 1.0).
	hr = m_depthStencilState.init(device, true, D3D11_DEPTH_WRITE_MASK_ZERO, D3D11_COMPARISON_LESS_EQUAL);
	if (FAILED(hr)) {
		ERROR("Skybox", "init", "Failed to create new DepthStencilState");
	}

	return S_OK;
}

// Proceso de dibujo del cielo en cada frame
void
Skybox::render(DeviceContext& deviceContext, Camera& camera) {
	// Guardia de seguridad: Evita crashes si la textura o modelo no cargaron
	if (!m_cubeModel || !m_skyboxTexture.m_textureFromImg) return;

	// 1) Aplicamos las reglas especiales de dibujo (estar adentro del cubo, pintar al fondo)
	m_rasterizerState.render(deviceContext);
	m_depthStencilState.render(deviceContext, 0, false);

	// 2) CÁLCULO DE LA MATRIZ DE VISTA (El truco del cielo infinito)
	// Le borramos la posición a la cámara. Solo nos importa a dónde mira.
	XMMATRIX viewNoT = camera.GetViewNoTranslation();
	XMMATRIX vp = viewNoT * camera.getProj();

	CBSkybox cb{};
	cb.mviewProj = XMMatrixTranspose(vp); // Multiplicación final
	m_constantBuffer.update(deviceContext, nullptr, 0, nullptr, &cb, 0, 0);
	m_constantBuffer.render(deviceContext, 0, 1);

	// 3) Activamos Shaders
	m_shaderProgram.render(deviceContext);

	// 4) IMPORTANTÍSIMO: Usamos el Slot 10 para no interferir con las texturas de los modelos 3D
	m_samplerState.render(deviceContext, 10, 1);
	m_skyboxTexture.render(deviceContext, 10, 1);

	// 5) Renderizamos usando la función especializada que agregamos a Actor
	m_skybox->renderForSkybox(deviceContext);

	// 6) FASE DE LIMPIEZA
	// Limpiamos el slot 10 y el 0 para evitar texturas fantasma en el siguiente modelo
	ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
	deviceContext.m_deviceContext->PSSetShaderResources(10, 1, nullSRV);
	deviceContext.m_deviceContext->PSSetShaderResources(0, 1, nullSRV);
}

// ======================================================================================
// FASE DE LIMPIEZA
// ======================================================================================
// Libera la memoria de la tarjeta gráfica y la RAM ocupada por el entorno
void
Skybox::destroy() {
	// Liberar el modelo 3D dinámico
	if (m_cubeModel) {
		delete m_cubeModel;
		m_cubeModel = nullptr;
	}

	// Liberar buffers, shaders, estados y texturas
	m_constantBuffer.destroy();
	m_shaderProgram.destroy();
	m_samplerState.destroy();
	m_skyboxTexture.destroy();

	// Limpiamos los estados de DirectX
	m_rasterizerState.destroy();
	m_depthStencilState.destroy();
}