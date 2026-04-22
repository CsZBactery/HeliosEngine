/**
 * @file ForwardRenderer.h
 * @brief Definición del pipeline de renderizado Forward con soporte de sombras.
 * @ingroup rendering
 */

#pragma once
#include "Prerequisites.h"
#include "Rendering/ISceneRenderer.h"
#include "Buffer.h"
#include "DepthStencilState.h"
#include "RasterizerState.h"
#include "ShaderProgram.h"
#include "Texture.h"
#include "DepthStencilView.h"
#include "EngineUtilities/Utilities/EditorViewportPass.h"
#include "Rendering/RenderScene.h"
#include <vector>

class Camera;
class Device;
class DeviceContext;
class Material;

class ForwardRenderer : public ISceneRenderer {
public:
    ForwardRenderer() = default;
    virtual ~ForwardRenderer() = default;

    HRESULT init(Device& device) override;
    void resize(Device& device, unsigned int width, unsigned int height) override;
    void render(DeviceContext& deviceContext, const Camera& camera, RenderScene& scene, EditorViewportPass& viewportPass) override;
    void destroy() override;

    // Implementación de ISceneRenderer para el Pipeline Manager
    ID3D11ShaderResourceView* getShadowMapSRV() const override { return m_shadowDepthSRV.m_textureFromImg; }
    ID3D11ShaderResourceView* getPreShadowSRV() const override { return m_preShadowDebugPass.getSRV(); }
    ID3D11ShaderResourceView* getGBufferAlbedoMetallicSRV() const override { return nullptr; }
    ID3D11ShaderResourceView* getGBufferNormalRoughnessSRV() const override { return nullptr; }
    ID3D11ShaderResourceView* getGBufferWorldAoSRV() const override { return nullptr; }
    ID3D11ShaderResourceView* getGBufferEmissiveAlphaSRV() const override { return nullptr; }
    void setShadowFactorDebugEnabled(bool enabled) override {}
    const char* getDebugName() const override { return "ForwardRenderer"; }

private:
    void buildQueues(RenderScene& scene, const Camera& camera);
    void updatePerFrame(const Camera& camera, const RenderScene& scene, DeviceContext& deviceContext);
    void updateLightMatrices(const Camera& camera, const RenderScene& scene);

    void renderPreShadowDebugPass(DeviceContext& deviceContext, RenderScene& scene);
    void renderShadowPass(DeviceContext& deviceContext);
    void renderSkyboxPass(DeviceContext& deviceContext, RenderScene& scene);
    void renderOpaquePass(DeviceContext& deviceContext);
    void renderTransparentPass(DeviceContext& deviceContext);

    void renderObject(DeviceContext& deviceContext, const RenderObject& object, RenderPassType passType);
    void renderShadowObject(DeviceContext& deviceContext, const RenderObject& object);

    HRESULT createShadowResources(Device& device);
    HRESULT createBlendStates(Device& device);
    ID3D11BlendState* resolveBlendState(const Material* material) const;

private:
    // Colas de renderizado dinámicas
    std::vector<const RenderObject*> m_opaqueQueue;
    std::vector<const RenderObject*> m_transparentQueue;

    // Comunicación con HLSL
    Buffer m_perFrameBuffer;
    Buffer m_perObjectBuffer;
    Buffer m_perMaterialBuffer;

    CBPerFrame m_cbPerFrame;
    CBPerObject m_cbPerObject;
    CBPerMaterial m_cbPerMaterial;

    // Estados del Pipeline
    DepthStencilState m_transparentDepthStencil;
    DepthStencilState m_shadowDepthStencil;
    RasterizerState m_shadowRasterizer;

    ID3D11BlendState* m_opaqueBlendState = nullptr;
    ID3D11BlendState* m_alphaBlendState = nullptr;
    ID3D11BlendState* m_additiveBlendState = nullptr;
    ID3D11BlendState* m_premultipliedBlendState = nullptr;
    float m_blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

    // Subsistema de Shadow Mapping
    EditorViewportPass m_preShadowDebugPass;
    ShaderProgram m_shadowShader;
    DepthStencilView m_shadowDSV;
    Texture m_shadowDepthSRV;
    Texture m_shadowDepthTexture;

    unsigned int m_shadowMapSize = 2048;
    bool m_applyShadows = true;
};