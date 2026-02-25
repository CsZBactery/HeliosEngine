// ======================================================================================
// Archivo: Skybox.cpp
// Implementación del entorno 3D (Cielo). 
// Utiliza un cubo gigante proyectado alrededor de la cámara.
// ======================================================================================

#include "EngineUtilities/Utilities/Skybox.h"
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
	// El tamaño real no importa, porque más adelante le quitaremos la traslación a la cámara,
	// haciendo que este cubo siempre envuelva al jugador sin importar a dónde camine.
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

	// 2) CREACIÓN DEL ACTOR
	m_skybox = EU::MakeShared<Actor>(device);

	if (!m_skybox.isNull()) {
		std::vector<MeshComponent> skybox;

		// Usamos el constructor paramétrico de Model3D que creamos antes para inyectar 
		// la geometría estática directamente desde la RAM sin leer un archivo .obj
		m_cubeModel = new Model3D("Skybox", vertices, indices);
		skybox = m_cubeModel->GetMeshes();

		// Asignamos la malla al actor. (No se le pasa textura aquí, se renderiza aparte).
		m_skybox->setMesh(device, skybox);

		// NOTA: Tu profe dejó "CyberGun" por accidente (copiar y pegar). Lo ideal es:
		m_skybox->setName("SkyboxActor");
	}
	else {
		ERROR("Skybox", "Init", "Failed to create Skybox Actor.");
		return E_FAIL;
	}

	// 3) CONFIGURACIÓN DE SHADERS (Input Layout)
	// Para el Skybox, el Shader solo necesita saber la Posición 3D (x, y, z).
	// Las coordenadas UV se calculan matemáticamente en el shader usando esa misma posición.
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

	// Carga y compila el shader especial para el cielo
	hr = m_shaderProgram.init(device, "Skybox.fx", Layout);

	// Buffer Constante para enviarle la matriz de la cámara al Shader
	hr = m_constantBuffer.init(device, sizeof(CBSkybox));
	if (FAILED(hr)) {
		ERROR("Skybox", "init",
			("Failed to initialize NeverChanges Buffer. HRESULT: " + std::to_string(hr)).c_str());
		return hr;
	}

	// Sampler: Define cómo se filtra la textura del cielo (ej. Linear o Anisotropic)
	hr = m_samplerState.init(device);
	if (FAILED(hr)) {
		ERROR("Skybox", "init", "Failed to create new SamplerState");
	}

	// NOTA DEL PROFE: Rasterizer y DepthStencil están comentados porque probablemente
	// los está controlando globalmente desde el BaseApp.cpp (usando RasterizerStateNoCull).
	//hr = m_rasterizerState.init(device, true, false);
	//hr = m_depthStencilState.init(device, true, false);

	return E_NOTIMPL; // Retorno temporal del profe
}

// Proceso de dibujo del cielo en cada frame
void
Skybox::render(DeviceContext& deviceContext, Camera& camera) {

	// 1) CÁLCULO DE LA MATRIZ DE VISTA (El truco del cielo infinito)
	XMMATRIX view = camera.getView();

	// Obtenemos la matriz de vista PERO le borramos la posición (traslación).
	// Esto hace que la cámara pueda rotar para ver a todos lados, pero si el jugador avanza,
	// el cielo "avanza" con él. Nunca te acercarás a la pared del cubo.
	view = XMMatrixTranspose(camera.GetViewNoTranslation());

	// Multiplicamos: Vista (solo rotación) * Proyección
	XMMATRIX vp = view * camera.getProj();

	// 2) ACTUALIZACIÓN DE DATOS EN LA GPU
	CBSkybox cb{};
	cb.mviewProj = XMMatrixTranspose(vp); // Preparamos la matriz para enviarla a DirectX
	m_constantBuffer.update(deviceContext, nullptr, 0, nullptr, &cb, 0, 0);
	m_constantBuffer.render(deviceContext, 0, 1);

	// 3) DIBUJO
	m_shaderProgram.render(deviceContext);
	m_samplerState.render(deviceContext, 0, 1);

	// NOTA: En el código del profe, el DrawIndexed está ANTES de poner la textura. 
	// Aunque en algunas arquitecturas previas esto funciona por estados heredados, 
	// la convención limpia es poner la textura ANTES de dar la orden de dibujar. 
	// Lo dejo como el profe lo estructuró, pero la Textura debería activarse primero.

	deviceContext.DrawIndexed(m_cubeModel->m_meshes[0].m_index.size(), 0, 0);

	m_skyboxTexture.render(deviceContext, 0, 1);
}

// ======================================================================================
// FASE DE LIMPIEZA
// ======================================================================================
// Libera la memoria de la tarjeta gráfica y la RAM ocupada por el entorno
void
Skybox::destroy() {
	// Liberar el modelo 3D dinámico (el cubo que creamos con 'new')
	if (m_cubeModel) {
		delete m_cubeModel;
		m_cubeModel = nullptr;
	}

	// Liberar buffers, shaders y texturas
	m_constantBuffer.destroy();
	m_shaderProgram.destroy();
	m_samplerState.destroy();
	m_skyboxTexture.destroy();

	// NOTA: Si en algún momento descomentas el Rasterizer y DepthStencil en tu init(),
	// también deberás descomentar estas dos líneas para evitar fugas de memoria:
	// m_rasterizerState.destroy();
	// m_depthStencilState.destroy();
}