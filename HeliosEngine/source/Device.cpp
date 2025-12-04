#include "../include/Device.h"

// -----------------------------------------------------------------------------
// [CRÍTICO] Esta es la función 'init' que faltaba y causaba el error LNK2019.
// Se encarga de crear el dispositivo D3D11 real.
// -----------------------------------------------------------------------------
void Device::init() {
	UINT createDeviceFlags = 0;
#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_DRIVER_TYPE driverTypes[] = {
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};
	UINT numDriverTypes = ARRAYSIZE(driverTypes);

	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
	HRESULT hr = E_FAIL;

	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++) {
		D3D_DRIVER_TYPE driverType = driverTypes[driverTypeIndex];

		// Puntero temporal para el contexto
		ID3D11DeviceContext* pContext = nullptr;

		hr = D3D11CreateDevice(
			nullptr,                    // Adapter
			driverType,                 // Driver Type
			nullptr,                    // Software
			createDeviceFlags,          // Flags
			featureLevels,              // Feature Levels
			numFeatureLevels,           // Num Feature Levels
			D3D11_SDK_VERSION,          // SDK Version
			&m_device,                  // Device (Output)
			&featureLevel,              // Feature Level (Output)
			&pContext                   // Context (Output temporal)
		);

		if (SUCCEEDED(hr)) {
			// Liberamos el contexto temporal (BaseApp lo recuperará después usando GetImmediateContext)
			if (pContext) pContext->Release();
			break;
		}
	}

	if (FAILED(hr)) {
		ERROR("Device", "init", "Failed to create D3D11 Device.");
	}
}

// Implementaciones vacías necesarias para evitar errores de Linker si se llaman
void Device::update() {}
void Device::render() {}

// -----------------------------------------------------------------------------
// RESTO DE FUNCIONES (Tal cual las tenías)
// -----------------------------------------------------------------------------

void Device::destroy() {
	if (m_device) {
		m_device->Release();
		m_device = nullptr;
	}
}

HRESULT Device::CreateRenderTargetView(ID3D11Resource* pResource,
	const D3D11_RENDER_TARGET_VIEW_DESC* pDesc,
	ID3D11RenderTargetView** ppRTView) {

	if (!pResource) {
		ERROR("Device", "CreateRenderTargetView", "pResource is nullptr");
		return E_INVALIDARG;
	}
	if (!ppRTView) {
		ERROR("Device", "CreateRenderTargetView", "ppRTView is nullptr");
		return E_POINTER;
	}
	// Safety check
	if (!m_device) return E_FAIL;

	HRESULT hr = m_device->CreateRenderTargetView(pResource, pDesc, ppRTView);

	if (SUCCEEDED(hr)) {
		MESSAGE("Device", "CreateRenderTargetView", "Render Target View created successfully!");
	}
	else {
		ERROR("Device", "CreateRenderTargetView", ("Failed to create RTV. HRESULT: " + std::to_string(hr)).c_str());
	}
	return hr;
}

HRESULT Device::CreateTexture2D(const D3D11_TEXTURE2D_DESC* pDesc,
	const D3D11_SUBRESOURCE_DATA* pInitialData,
	ID3D11Texture2D** ppTexture2D) {

	if (!pDesc) {
		ERROR("Device", "CreateTexture2D", "pDesc is nullptr");
		return E_INVALIDARG;
	}
	if (!ppTexture2D) {
		ERROR("Device", "CreateTexture2D", "ppTexture2D is nullptr");
		return E_POINTER;
	}
	if (!m_device) return E_FAIL;

	HRESULT hr = m_device->CreateTexture2D(pDesc, pInitialData, ppTexture2D);

	if (SUCCEEDED(hr)) {
		MESSAGE("Device", "CreateTexture2D", "Texture2D created successfully!");
	}
	else {
		ERROR("Device", "CreateTexture2D", ("Failed to create Texture2D. HRESULT: " + std::to_string(hr)).c_str());
	}
	return hr;
}

HRESULT Device::CreateDepthStencilView(ID3D11Resource* pResource,
	const D3D11_DEPTH_STENCIL_VIEW_DESC* pDesc,
	ID3D11DepthStencilView** ppDepthStencilView) {

	if (!pResource) {
		ERROR("Device", "CreateDepthStencilView", "pResource is nullptr");
		return E_INVALIDARG;
	}
	if (!ppDepthStencilView) {
		ERROR("Device", "CreateDepthStencilView", "ppDepthStencilView is nullptr");
		return E_POINTER;
	}
	if (!m_device) return E_FAIL;

	HRESULT hr = m_device->CreateDepthStencilView(pResource, pDesc, ppDepthStencilView);

	if (SUCCEEDED(hr)) {
		MESSAGE("Device", "CreateDepthStencilView", "Depth Stencil View created successfully!");
	}
	else {
		ERROR("Device", "CreateDepthStencilView", ("Failed to create DSV. HRESULT: " + std::to_string(hr)).c_str());
	}
	return hr;
}

HRESULT Device::CreateVertexShader(const void* pShaderBytecode,
	unsigned int BytecodeLength,
	ID3D11ClassLinkage* pClassLinkage,
	ID3D11VertexShader** ppVertexShader) {

	if (!pShaderBytecode) {
		ERROR("Device", "CreateVertexShader", "pShaderBytecode is nullptr");
		return E_INVALIDARG;
	}
	if (!ppVertexShader) {
		ERROR("Device", "CreateVertexShader", "ppVertexShader is nullptr");
		return E_POINTER;
	}
	if (!m_device) return E_FAIL;

	HRESULT hr = m_device->CreateVertexShader(pShaderBytecode, BytecodeLength, pClassLinkage, ppVertexShader);

	if (SUCCEEDED(hr)) {
		MESSAGE("Device", "CreateVertexShader", "Vertex Shader created successfully!");
	}
	else {
		ERROR("Device", "CreateVertexShader", ("Failed to create Vertex Shader. HRESULT: " + std::to_string(hr)).c_str());
	}
	return hr;
}

HRESULT Device::CreateInputLayout(const D3D11_INPUT_ELEMENT_DESC* pInputElementDescs,
	unsigned int NumElements,
	const void* pShaderBytecodeWithInputSignature,
	unsigned int BytecodeLength,
	ID3D11InputLayout** ppInputLayout) {

	if (!pInputElementDescs) {
		ERROR("Device", "CreateInputLayout", "pInputElementDescs is nullptr");
		return E_INVALIDARG;
	}
	if (!ppInputLayout) {
		ERROR("Device", "CreateInputLayout", "ppInputLayout is nullptr");
		return E_POINTER;
	}
	if (!m_device) return E_FAIL;

	HRESULT hr = m_device->CreateInputLayout(pInputElementDescs, NumElements, pShaderBytecodeWithInputSignature, BytecodeLength, ppInputLayout);

	if (SUCCEEDED(hr)) {
		MESSAGE("Device", "CreateInputLayout", "Input Layout created successfully!");
	}
	else {
		ERROR("Device", "CreateInputLayout", ("Failed to create Input Layout. HRESULT: " + std::to_string(hr)).c_str());
	}
	return hr;
}

HRESULT Device::CreatePixelShader(const void* pShaderBytecode,
	unsigned int BytecodeLength,
	ID3D11ClassLinkage* pClassLinkage,
	ID3D11PixelShader** ppPixelShader) {

	if (!pShaderBytecode) {
		ERROR("Device", "CreatePixelShader", "pShaderBytecode is nullptr");
		return E_INVALIDARG;
	}
	if (!ppPixelShader) {
		ERROR("Device", "CreatePixelShader", "ppPixelShader is nullptr");
		return E_POINTER;
	}
	if (!m_device) return E_FAIL;

	HRESULT hr = m_device->CreatePixelShader(pShaderBytecode, BytecodeLength, pClassLinkage, ppPixelShader);

	if (SUCCEEDED(hr)) {
		MESSAGE("Device", "CreatePixelShader", "Pixel Shader created successfully!");
	}
	else {
		ERROR("Device", "CreatePixelShader", ("Failed to create Pixel Shader. HRESULT: " + std::to_string(hr)).c_str());
	}
	return hr;
}

HRESULT Device::CreateSamplerState(const D3D11_SAMPLER_DESC* pSamplerDesc,
	ID3D11SamplerState** ppSamplerState) {

	if (!pSamplerDesc) {
		ERROR("Device", "CreateSamplerState", "pSamplerDesc is nullptr");
		return E_INVALIDARG;
	}
	if (!ppSamplerState) {
		ERROR("Device", "CreateSamplerState", "ppSamplerState is nullptr");
		return E_POINTER;
	}
	if (!m_device) return E_FAIL;

	HRESULT hr = m_device->CreateSamplerState(pSamplerDesc, ppSamplerState);

	if (SUCCEEDED(hr)) {
		MESSAGE("Device", "CreateSamplerState", "Sampler State created successfully!");
	}
	else {
		ERROR("Device", "CreateSamplerState", ("Failed to create Sampler State. HRESULT: " + std::to_string(hr)).c_str());
	}
	return hr;
}

HRESULT Device::CreateBuffer(const D3D11_BUFFER_DESC* pDesc,
	const D3D11_SUBRESOURCE_DATA* pInitialData,
	ID3D11Buffer** ppBuffer) {

	if (!pDesc) {
		ERROR("Device", "CreateBuffer", "pDesc is nullptr");
		return E_INVALIDARG;
	}
	if (!ppBuffer) {
		ERROR("Device", "CreateBuffer", "ppBuffer is nullptr");
		return E_POINTER;
	}
	if (!m_device) return E_FAIL;

	HRESULT hr = m_device->CreateBuffer(pDesc, pInitialData, ppBuffer);

	if (SUCCEEDED(hr)) {
		MESSAGE("Device", "CreateBuffer", "Buffer created successfully!");
	}
	else {
		ERROR("Device", "CreateBuffer", ("Failed to create Buffer. HRESULT: " + std::to_string(hr)).c_str());
	}
	return hr;
}