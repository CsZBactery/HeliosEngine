/**
 * @file RenderPipeline.cpp
 * @brief Implementación del selector y gestor de pipelines de renderizado.
 * @details Permite la transición fluida entre renderizado Forward y Deferred (Diferido).
 */

#include "Rendering/RenderPipeline.h"

 // ======================================================================================
 // INICIALIZACIÓN Y CONFIGURACIÓN
 // ======================================================================================

HRESULT
RenderPipeline::init(Device& device, RendererType initialRenderer) {
    m_activeRenderer = nullptr;
    m_activeRendererType = initialRenderer;
    m_forwardInitialized = false;
    m_deferredInitialized = false;

    // Configurar el renderer inicial solicitado
    return setRendererType(initialRenderer, device);
}

HRESULT
RenderPipeline::setRendererType(RendererType rendererType, Device& device) {
    // Asegurar que el renderer solicitado esté cargado en memoria de video
    HRESULT hr = ensureRendererInitialized(rendererType, device);
    if (FAILED(hr)) {
        return hr;
    }

    m_activeRendererType = rendererType;
    m_activeRenderer = resolveRenderer(rendererType);

    if (m_activeRenderer) {
        MESSAGE("RenderPipeline", "setRendererType", m_activeRenderer->getDebugName());
    }

    return m_activeRenderer ? S_OK : E_FAIL;
}

// ======================================================================================
// CICLO DE VIDA DE RENDERIZADO
// ======================================================================================

void
RenderPipeline::render(DeviceContext& deviceContext,
    const Camera& camera,
    RenderScene& scene,
    EditorViewportPass& viewportPass) {
    // Delegar la ejecución al renderer activo actualmente
    if (m_activeRenderer) {
        m_activeRenderer->render(deviceContext, camera, scene, viewportPass);
    }
}

void
RenderPipeline::resize(Device& device, unsigned int width, unsigned int height) {
    // Notificar al renderer activo que el tamaño del viewport ha cambiado
    if (m_activeRenderer) {
        m_activeRenderer->resize(device, width, height);
    }
}

void
RenderPipeline::destroy() {
    // Limpieza segura de ambos subsistemas si fueron inicializados
    if (m_deferredInitialized) {
        m_deferredRenderer.destroy();
        m_deferredInitialized = false;
    }

    if (m_forwardInitialized) {
        m_forwardRenderer.destroy();
        m_forwardInitialized = false;
    }

    m_activeRenderer = nullptr;
}

// ======================================================================================
// GETTERS PARA DEPURACIÓN (Integración con GUI)
// ======================================================================================

ID3D11ShaderResourceView*
RenderPipeline::getShadowMapSRV() const {
    const ISceneRenderer* renderer = resolveRenderer(m_activeRendererType);
    return renderer ? renderer->getShadowMapSRV() : nullptr;
}

ID3D11ShaderResourceView*
RenderPipeline::getPreShadowSRV() const {
    const ISceneRenderer* renderer = resolveRenderer(m_activeRendererType);
    return renderer ? renderer->getPreShadowSRV() : nullptr;
}

ID3D11ShaderResourceView*
RenderPipeline::getGBufferAlbedoMetallicSRV() const {
    const ISceneRenderer* renderer = resolveRenderer(m_activeRendererType);
    return renderer ? renderer->getGBufferAlbedoMetallicSRV() : nullptr;
}

ID3D11ShaderResourceView*
RenderPipeline::getGBufferNormalRoughnessSRV() const {
    const ISceneRenderer* renderer = resolveRenderer(m_activeRendererType);
    return renderer ? renderer->getGBufferNormalRoughnessSRV() : nullptr;
}

ID3D11ShaderResourceView*
RenderPipeline::getGBufferWorldAoSRV() const {
    const ISceneRenderer* renderer = resolveRenderer(m_activeRendererType);
    return renderer ? renderer->getGBufferWorldAoSRV() : nullptr;
}

ID3D11ShaderResourceView*
RenderPipeline::getGBufferEmissiveAlphaSRV() const {
    const ISceneRenderer* renderer = resolveRenderer(m_activeRendererType);
    return renderer ? renderer->getGBufferEmissiveAlphaSRV() : nullptr;
}

void
RenderPipeline::setShadowFactorDebugEnabled(bool enabled) {
    // El factor de sombra diferido es específico del DeferredRenderer
    ISceneRenderer* renderer = resolveRenderer(RendererType::Deferred);
    if (renderer) {
        renderer->setShadowFactorDebugEnabled(enabled);
    }
}

// ======================================================================================
// HELPERS INTERNOS (Resolución de tipos)
// ======================================================================================

HRESULT
RenderPipeline::ensureRendererInitialized(RendererType rendererType, Device& device) {
    switch (rendererType) {
    case RendererType::Forward:
        if (!m_forwardInitialized) {
            HRESULT hr = m_forwardRenderer.init(device);
            if (FAILED(hr)) return hr;
            m_forwardInitialized = true;
        }
        return S_OK;

    case RendererType::Deferred:
        if (!m_deferredInitialized) {
            HRESULT hr = m_deferredRenderer.init(device);
            if (FAILED(hr)) return hr;
            m_deferredInitialized = true;
        }
        return S_OK;

    default:
        return E_INVALIDARG;
    }
}

ISceneRenderer*
RenderPipeline::resolveRenderer(RendererType rendererType) {
    switch (rendererType) {
    case RendererType::Forward:  return &m_forwardRenderer;
    case RendererType::Deferred: return &m_deferredRenderer;
    default: return nullptr;
    }
}

const ISceneRenderer*
RenderPipeline::resolveRenderer(RendererType rendererType) const {
    switch (rendererType) {
    case RendererType::Forward:  return &m_forwardRenderer;
    case RendererType::Deferred: return &m_deferredRenderer;
    default: return nullptr;
    }
}