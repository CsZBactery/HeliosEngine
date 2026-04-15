#pragma once
#include "Prerequisites.h"
#include "Buffer.h"
#include "DepthStencilState.h"
#include "Rendering/RenderScene.h"
#include "Rendering/RenderTypes.h"

class Device;
class DeviceContext;
class Camera;
class EditorViewportPass;
class Material;

/**
 * @class ForwardRenderer
 * @brief Motor de renderizado en cascada (Forward Rendering) para procesar mallas y luces.
 * @details Se encarga de ordenar los objetos de la escena por opacidad y distancia,
 * aplicando los estados de mezcla (Blend States) correctos y dibujándolos en el Viewport.
 */
class ForwardRenderer {
public:
    /**
     * @brief Inicializa los Buffers Constantes y los Estados de Mezcla (Blend States).
     * @param device Referencia al dispositivo físico (GPU).
     * @return HRESULT S_OK si la inicialización fue exitosa.
     */
    HRESULT init(Device& device);

    /**
     * @brief Actualiza los recursos internos si cambia el tamaño del Viewport.
     */
    void resize(Device& device, unsigned int width, unsigned int height);

    /**
     * @brief Prepara los datos del frame actual (Matrices de cámara, luces, tiempo).
     * @param camera Cámara activa de la escena.
     * @param scene Escena recolectada lista para renderizar.
     * @param deviceContext Contexto de ejecución.
     */
    void updatePerFrame(const Camera& camera, const RenderScene& scene, DeviceContext& deviceContext);

    /**
     * @brief Ejecuta el pipeline de dibujo completo en la textura del editor.
     * @details Dibuja en orden: Objetos Opacos -> Skybox -> Objetos Transparentes.
     */
    void render(DeviceContext& deviceContext,
        const Camera& camera,
        RenderScene& scene,
        EditorViewportPass& viewportPass);

    /** @brief Libera los recursos de DirectX instanciados por este renderer. */
    void destroy();

private:
    /** @brief Separa y ordena los objetos opacos de los transparentes. */
    void buildQueues(RenderScene& scene, const Camera& camera);

    /** @brief Dibuja todos los objetos de la cola de Opacos. */
    void renderOpaquePass(DeviceContext& deviceContext);

    /** @brief Dibuja todos los objetos de la cola de Transparentes (ordenados de atrás hacia adelante). */
    void renderTransparentPass(DeviceContext& deviceContext);

    /** @brief Dibuja el fondo del nivel. */
    void renderSkyboxPass(DeviceContext& deviceContext, RenderScene& scene);

    /** @brief Configura el pipeline para un objeto específico y emite el comando Draw. */
    void renderObject(DeviceContext& deviceContext, const RenderObject& object, RenderPassType passType);

    /** @brief Crea los perfiles de transparencia en la GPU (Alpha, Additive, etc). */
    HRESULT createBlendStates(Device& device);

    /** @brief Obtiene el Blend State adecuado según el Material del objeto. */
    ID3D11BlendState* resolveBlendState(const Material* material) const;

private:
    // ==========================================
    // RECURSOS DIRECTX
    // ==========================================
    Buffer m_perFrameBuffer;          /**< Buffer para datos de la cámara y entorno. */
    Buffer m_perObjectBuffer;         /**< Buffer para la posición del modelo. */
    Buffer m_perMaterialBuffer;       /**< Buffer para las propiedades visuales del modelo. */

    DepthStencilState m_transparentDepthStencil; /**< Z-Buffer especial (lectura, sin escritura). */

    ID3D11BlendState* m_alphaBlendState = nullptr;
    ID3D11BlendState* m_opaqueBlendState = nullptr;
    ID3D11BlendState* m_additiveBlendState = nullptr;
    ID3D11BlendState* m_premultipliedBlendState = nullptr;
    float m_blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

    // ==========================================
    // ESTRUCTURAS DE DATOS CPU
    // ==========================================
    CBPerFrame    m_cbPerFrame{};
    CBPerObject   m_cbPerObject{};
    CBPerMaterial m_cbPerMaterial{};

    // ==========================================
    // COLAS DE DIBUJO
    // ==========================================
    std::vector<const RenderObject*> m_opaqueQueue;      /**< Objetos sin transparencia. */
    std::vector<const RenderObject*> m_transparentQueue; /**< Objetos con transparencia. */
};