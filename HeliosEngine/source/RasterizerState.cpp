// ======================================================================================
// Archivo: RasterizerState.cpp
// Implementación de las reglas para la conversión de geometría 3D a píxeles 2D.
// ======================================================================================

#include "RasterizerState.h"
#include "Device.h"
#include "DeviceContext.h"

// ======================================================================================
// Inicializa el rasterizador con la configuración por defecto (Sólido y Culling trasero)
// ======================================================================================
HRESULT
RasterizerState::init(Device device) {
    // Estructura que define cómo se "rellenan" los triángulos en la pantalla
    D3D11_RASTERIZER_DESC rasterizerDesc = {};

    // Dibuja el interior del polígono (si fuera D3D11_FILL_WIREFRAME, solo veríamos líneas)
    rasterizerDesc.FillMode = D3D11_FILL_SOLID;

    // Optimización principal: No dibuja las caras traseras de los objetos.
    // (Ej. No dibuja el interior de una caja cerrada porque el jugador nunca lo verá).
    rasterizerDesc.CullMode = D3D11_CULL_BACK;

    // Define qué cara es el "Frente". En DirectX, el frente suele ser sentido horario (Clockwise).
    rasterizerDesc.FrontCounterClockwise = FALSE;

    // Estos parámetros (Bias) se usan principalmente para evitar artefactos visuales
    // cuando se dibujan sombras (Shadow Acne). Por ahora los dejamos en cero.
    rasterizerDesc.DepthBias = 0;
    rasterizerDesc.SlopeScaledDepthBias = 0.0f;
    rasterizerDesc.DepthBiasClamp = 0.0f;

    // Permite recortar los píxeles que están más allá de la cámara (Far Plane)
    rasterizerDesc.DepthClipEnable = TRUE;

    // Scissor permite renderizar solo dentro de un rectángulo específico de la pantalla. (Apagado por defecto)
    rasterizerDesc.ScissorEnable = FALSE;

    // Apagamos el Multisampling y Anti-aliasing de líneas en esta configuración base
    rasterizerDesc.MultisampleEnable = FALSE;
    rasterizerDesc.AntialiasedLineEnable = FALSE;

    HRESULT hr = S_OK;

    // Enviamos nuestra configuración a la GPU para crear el estado lógico
    hr = device.m_device->CreateRasterizerState(&rasterizerDesc, &m_rasterizerState);

    if (FAILED(hr)) {
        ERROR("Rasterizer", "init", "CHECK FOR CreateRasterizerState()");
    }
    return hr;
}

// ======================================================================================
// Inicializa el rasterizador permitiendo configurar el descarte de caras y la profundidad
// ======================================================================================
HRESULT
RasterizerState::init(Device& device,
    D3D11_FILL_MODE fill,
    D3D11_CULL_MODE cull,
    bool frontCCW,
    bool depthClip) {

    // Inicializamos en ceros
    D3D11_RASTERIZER_DESC desc{};

    // Asignamos directamente las variables enviadas por parámetro
    desc.FillMode = fill;
    desc.CullMode = cull;

    // Convertimos el tipo bool nativo de C++ al tipo BOOL (entero) que usa DirectX
    desc.FrontCounterClockwise = frontCCW ? TRUE : FALSE;
    desc.DepthClipEnable = depthClip ? TRUE : FALSE;

    HRESULT hr = S_OK;
    hr = device.m_device->CreateRasterizerState(&desc, &m_rasterizerState);

    if (FAILED(hr)) {
        ERROR("Rasterizer", "init", "CHECK FOR CreateRasterizerState()");
    }
    return hr;
}

// ======================================================================================
// Actualización dinámica del estado (Placeholder)
// ======================================================================================
void
RasterizerState::update() {
    // Espacio reservado por si necesitamos modificar dinámicamente este estado 
    // (ej. cambiar a Wireframe en tiempo real durante el editor)
}

// ======================================================================================
// Vincula este estado de rasterización al pipeline
// ======================================================================================
void
RasterizerState::render(DeviceContext& deviceContext) {
    // Verificación de seguridad agregada por el profesor para evitar crashes en la GPU
    if (!m_rasterizerState) {
        ERROR("RasterizerState", "render", "RasterizerState is nullptr (init failed or not called)");
        return;
    }

    deviceContext.RSSetState(m_rasterizerState);
}

// ======================================================================================
// Libera la memoria de la tarjeta de video de forma segura
// ======================================================================================
void
RasterizerState::destroy() {
    SAFE_RELEASE(m_rasterizerState);
}