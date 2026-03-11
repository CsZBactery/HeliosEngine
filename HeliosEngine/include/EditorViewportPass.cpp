// ======================================================================================
// Archivo: EditorViewportPass.cpp
// Implementación del sistema "Render to Texture" para el panel del Viewport del Editor.
// ======================================================================================

#include "EngineUtilities/Utilities/EditorViewportPass.h"
#include "Device.h"
#include "DeviceContext.h"

// ======================================================================================
// Inicialización Base
// ======================================================================================
HRESULT EditorViewportPass::init(Device& device, unsigned int width, unsigned int height) {
    return createResources(device, width, height);
}

// ======================================================================================
// Redimensionamiento dinámico
// Se llama cuando el usuario arrastra los bordes de la ventana del Viewport en ImGui
// ======================================================================================
HRESULT EditorViewportPass::resize(Device& device, unsigned int width, unsigned int height) {
    // Límite de seguridad: DirectX puede crashear si intentamos crear texturas de 0x0.
    // 64x64 es un tamaño mínimo seguro cuando la ventana colapsa.
    if (width < 64) width = 64;
    if (height < 64) height = 64;

    // Si el tamaño no ha cambiado y los recursos están intactos, no hacemos nada para ahorrar rendimiento
    if (width == m_width && height == m_height && isValid()) {
        return S_OK;
    }

    // Si cambió el tamaño, recreamos los buffers internos
    return createResources(device, width, height);
}

// ======================================================================================
// Creación de Recursos (El núcleo del Render to Texture)
// ======================================================================================
HRESULT EditorViewportPass::createResources(Device& device, unsigned int width, unsigned int height) {
    // 0) Limpiamos cualquier recurso viejo antes de recrearlos
    destroy();

    if (width == 0)  width = 1;
    if (height == 0) height = 1;

    m_width = width;
    m_height = height;

    HRESULT hr = S_OK;

    // 1) Textura de Color (Offscreen)
    // Nota la bandera: BIND_RENDER_TARGET (para que la GPU dibuje en ella) 
    // y BIND_SHADER_RESOURCE (para que ImGui la lea después como imagen).
    hr = m_colorTexture.init(
        device,
        width,
        height,
        DXGI_FORMAT_R8G8B8A8_UNORM,
        D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE,
        1,
        0
    );
    if (FAILED(hr)) return hr;

    // 2) Render Target View (RTV)
    // Esta es la "boca" por donde el pipeline de DirectX inyecta los colores a la textura
    hr = m_rtv.init(
        device,
        m_colorTexture,
        D3D11_RTV_DIMENSION_TEXTURE2D,
        DXGI_FORMAT_R8G8B8A8_UNORM
    );
    if (FAILED(hr)) return hr;

    // 3) Shader Resource View (SRV)
    // Esta es la "lente" que ImGui usará para mostrar la textura en su interfaz
    hr = m_colorSRV.init(device, m_colorTexture, DXGI_FORMAT_R8G8B8A8_UNORM);
    if (FAILED(hr)) return hr;

    // 4) Textura de Profundidad (Z-Buffer local)
    // El Viewport necesita su propio Z-Buffer del mismo tamaño exacto que la textura de color
    hr = m_depthTexture.init(
        device,
        width,
        height,
        DXGI_FORMAT_D24_UNORM_S8_UINT,
        D3D11_BIND_DEPTH_STENCIL,
        1,
        0
    );
    if (FAILED(hr)) return hr;

    // 5) Depth Stencil View (DSV)
    // La "boca" por donde se inyectan las distancias para saber qué polígono está enfrente de otro
    hr = m_dsv.init(
        device,
        m_depthTexture,
        DXGI_FORMAT_D24_UNORM_S8_UINT,
        D3D11_DSV_DIMENSION_TEXTURE2D
    );
    if (FAILED(hr)) return hr;

    return S_OK;
}

// ======================================================================================
// Preparación del frame
// ======================================================================================
void EditorViewportPass::begin(DeviceContext& deviceContext, const float clearColor[4]) {
    // Limpia la textura con el color de fondo y vincula este RTV al pipeline.
    // A partir de esta línea, todo lo que se dibuje irá a nuestra textura, no a la pantalla.
    m_rtv.render(deviceContext, m_dsv, 1, clearColor);
}

// ======================================================================================
// Intercambio (Útil para Ping-Pong en Post-Procesado)
// ======================================================================================
void EditorViewportPass::swap(EditorViewportPass& other) {
    std::swap(m_colorTexture, other.m_colorTexture);
    std::swap(m_colorSRV, other.m_colorSRV);
    std::swap(m_rtv, other.m_rtv);
    std::swap(m_depthTexture, other.m_depthTexture);
    std::swap(m_dsv, other.m_dsv);
    std::swap(m_width, other.m_width);
    std::swap(m_height, other.m_height);
}

// ======================================================================================
// Limpieza parcial
// ======================================================================================
void EditorViewportPass::clearDepth(DeviceContext& deviceContext) {
    m_dsv.render(deviceContext);
}

// ======================================================================================
// Configura el Viewport de la GPU para que coincida con la textura
// ======================================================================================
void EditorViewportPass::setViewport(DeviceContext& deviceContext) {
    D3D11_VIEWPORT vp{};
    vp.TopLeftX = 0.0f;
    vp.TopLeftY = 0.0f;
    vp.Width = static_cast<float>(m_width);
    vp.Height = static_cast<float>(m_height);
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;

    deviceContext.m_deviceContext->RSSetViewports(1, &vp);
}

// ======================================================================================
// Destrucción segura
// ======================================================================================
void EditorViewportPass::destroy() {
    m_dsv.destroy();
    m_depthTexture.destroy();
    m_colorSRV.destroy();
    m_rtv.destroy();
    m_colorTexture.destroy();

    m_width = 1;
    m_height = 1;
}