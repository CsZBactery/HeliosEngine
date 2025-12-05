#include "../include/SamplerState.h"
#include "../include/Device.h"
#include "../include/DeviceContext.h"

HRESULT
SamplerState::init(Device& device) {
    // Sin dispositivo no podemos reservar memoria en la GPU
    if (!device.m_device) {
        ERROR("SamplerState", "init", "Device is nullptr");
        return E_POINTER;
    }

    // Configuramos la descripcion del muestreo.
    // Esto define las "reglas de juego" para leer pixeles de las texturas.
    D3D11_SAMPLER_DESC sampDesc;
    ZeroMemory(&sampDesc, sizeof(sampDesc));

    // FILTER: Usamos LINEAR para suavizar la imagen (Antialiasing basico de textura).
    // Si quisieras un look "Retro" o "Minecraft", aqui usarias D3D11_FILTER_MIN_MAG_MIP_POINT.
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;

    // ADDRESS: WRAP significa que si las coordenadas UV son mayores a 1.0, 
    // la textura se repite en mosaico (Tileable).
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

    // Creamos el objeto de estado
    HRESULT hr = device.CreateSamplerState(&sampDesc, &m_sampler);
    if (FAILED(hr)) {
        ERROR("SamplerState", "init", "Failed to create SamplerState");
        return hr;
    }

    return S_OK;
}

void
SamplerState::update() {
    // Los Samplers suelen ser estaticos, no cambian cada frame.
}

void
SamplerState::render(DeviceContext& deviceContext,
    unsigned int StartSlot,
    unsigned int NumSampler) {

    if (!m_sampler) {
        ERROR("SamplerState", "render", "SamplerState is nullptr");
        return;
    }

    // Le decimos al Pixel Shader: "Usa estas reglas para leer la textura en el slot X"
    deviceContext.PSSetSamplers(StartSlot, NumSampler, &m_sampler);
}

void
SamplerState::destroy() {
    SAFE_RELEASE(m_sampler);
}