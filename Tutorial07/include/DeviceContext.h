#pragma once
/**
 * @file DeviceContext.h
 * @brief Wrapper mínimo (move-only) del contexto inmediato de D3D11.
 *
 * @details
 * Esta clase encapsula un puntero a `ID3D11DeviceContext` (el contexto inmediato)
 * y expone métodos del pipeline que usas a menudo (OM/RS/IA/VS/PS), con la opción
 * de adquirirlo desde un `ID3D11Device` o adjuntar uno existente.
 *
 * - Semántica *move-only*: no copiable para evitar dobles `Release()`.
 * - No crea el contexto; lo adquiere vía `GetImmediateContext` o `attach()`.
 * - No posee SRVs/RTVs/buffers/shaders que se le pasan; sólo los enlaza.
 *
 * @note Requiere que `Prerequisites.h` traiga las dependencias de Win32/D3D11.
 */

#include "Prerequisites.h"

 /**
  * @class DeviceContext
  * @brief Envoltura del `ID3D11DeviceContext` inmediato con utilidades del pipeline.
  *
  * @details
  * Uso típico:
  * @code
  * DeviceContext ctx;
  * ctx.initFromDevice(device);  // obtiene el inmediato
  * ctx.OMSetRenderTargets(1, rtvAddr, dsv);
  * ctx.RSSetViewports(1, &vp);
  * // ... binds de IA/VS/PS ...
  * @endcode
  *
  * @warning La clase no administra la vida de los recursos vinculados (RTV/SRV/Buffer/Shader).
  * Sólo mantiene una referencia al propio contexto cuando se adjunta con `attach()`.
  */
class DeviceContext {
public:
  /** @brief Ctor por defecto (no adquiere contexto). */
  DeviceContext() = default;

  /** @brief Dtor por defecto (no libera automáticamente; usa `destroy()`). */
  ~DeviceContext() = default;

  // --------------------------
  // Semántica de movimiento
  // --------------------------

  /**
   * @brief Move constructor: transfiere el puntero al contexto.
   * @param other Instancia origen; su puntero queda en `nullptr`.
   */
  DeviceContext(DeviceContext&& other) noexcept;

  /**
   * @brief Asignación por movimiento: libera el actual y toma el de `other`.
   * @param other Instancia origen; su puntero queda en `nullptr`.
   * @return *this
   */
  DeviceContext& operator=(DeviceContext&& other) noexcept;

  /** @brief Copia deshabilitada para evitar doble `Release()`. */
  DeviceContext(const DeviceContext&) = delete;

  /** @brief Asignación por copia deshabilitada para evitar doble `Release()`. */
  DeviceContext& operator=(const DeviceContext&) = delete;

  // --------------------------
  // Adquisición / gestión
  // --------------------------

  /**
   * @brief Obtiene el contexto inmediato desde un `ID3D11Device`.
   * @param device Puntero válido al dispositivo D3D11.
   * @return `S_OK` si tuvo éxito; `E_POINTER` si `device == nullptr`; `E_FAIL` si no se pudo obtener el contexto.
   * @pre `device != nullptr`
   * @post `get() != nullptr` si tuvo éxito.
   */
  HRESULT initFromDevice(ID3D11Device* device);

  /**
   * @brief Adjunta un contexto existente e incrementa su referencia (`AddRef`).
   * @param ctx Contexto a adjuntar.
   * @return `S_OK` si tuvo éxito; `E_POINTER` si `ctx == nullptr`.
   * @post `get()` devuelve el contexto adjunto; esta instancia llama a `Release()` en `destroy()`.
   */
  HRESULT attach(ID3D11DeviceContext* ctx);

  /**
   * @brief Limpia el estado del pipeline (`ID3D11DeviceContext::ClearState`).
   * @details Útil antes de cambiar completamente de pipeline o al hacer shutdown ordenado.
   * @note No libera recursos, sólo desempaqueta binds/estados del contexto.
   */
  void clearState();

  /**
   * @brief Libera el contexto (si está adjunto) y lo pone a `nullptr`.
   * @details Internamente hace `SAFE_RELEASE(m_deviceContext)`.
   */
  void destroy();

  // --------------------------
  // OUTPUT-MERGER (OM)
  // --------------------------

  /**
   * @brief Enlaza vistas de render (RTV) y depth-stencil (DSV) al OM.
   * @param NumViews Número de RTVs.
   * @param ppRenderTargetViews Array de punteros a RTVs.
   * @param pDepthStencilView Vista de depth-stencil (puede ser `nullptr`).
   * @pre `get() != nullptr`
   */
  void OMSetRenderTargets(UINT NumViews,
    ID3D11RenderTargetView* const* ppRenderTargetViews,
    ID3D11DepthStencilView* pDepthStencilView);

  // --------------------------
  // RASTERIZER (RS)
  // --------------------------

  /**
   * @brief Define uno o varios viewports.
   * @param NumViewports Número de viewports.
   * @param pViewports   Array de `D3D11_VIEWPORT`.
   * @pre `get() != nullptr`
   */
  void RSSetViewports(UINT NumViewports, const D3D11_VIEWPORT* pViewports);

  /**
   * @brief Establece el estado de rasterizado.
   * @param pRasterizerState Estado de rasterizer (puede ser `nullptr` para por defecto).
   * @pre `get() != nullptr`
   */
  void RSSetState(ID3D11RasterizerState* pRasterizerState);

  // --------------------------
  // INPUT ASSEMBLER (IA)
  // --------------------------

  /**
   * @brief Fija el input layout activo.
   * @param pInputLayout Layout de entrada (puede ser `nullptr`).
   * @pre `get() != nullptr`
   */
  void IASetInputLayout(ID3D11InputLayout* pInputLayout);

  /**
   * @brief Enlaza uno o varios vertex buffers.
   * @param StartSlot   Primer slot destino.
   * @param NumBuffers  Número de buffers.
   * @param ppVertexBuffers Array de buffers.
   * @param pStrides    Array de strides por vértice.
   * @param pOffsets    Array de offsets por buffer.
   * @pre `get() != nullptr`
   */
  void IASetVertexBuffers(UINT StartSlot, UINT NumBuffers,
    ID3D11Buffer* const* ppVertexBuffers,
    const UINT* pStrides, const UINT* pOffsets);

  /**
   * @brief Enlaza el index buffer.
   * @param pIndexBuffer Buffer de índices.
   * @param Format       Formato (p.ej. `DXGI_FORMAT_R16_UINT`).
   * @param Offset       Offset inicial (bytes).
   * @pre `get() != nullptr`
   */
  void IASetIndexBuffer(ID3D11Buffer* pIndexBuffer,
    DXGI_FORMAT Format, UINT Offset);

  /**
   * @brief Fija la topología primitiva del IA.
   * @param Topology Valor de `D3D11_PRIMITIVE_TOPOLOGY`.
   * @pre `get() != nullptr`
   */
  void IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY Topology);

  // --------------------------
  // VERTEX SHADER (VS)
  // --------------------------

  /**
   * @brief Enlaza el Vertex Shader (y sus class-instances si aplica).
   * @param pVS               Shader.
   * @param ppClassInstances  Array de class-instances (o `nullptr`).
   * @param NumClassInstances Número de class-instances.
   * @pre `get() != nullptr`
   */
  void VSSetShader(ID3D11VertexShader* pVS,
    ID3D11ClassInstance* const* ppClassInstances,
    UINT NumClassInstances);

  /**
   * @brief Enlaza constant-buffers al VS.
   * @param StartSlot Slot inicial.
   * @param NumBuffers Número de CBs.
   * @param ppConstantBuffers Array de CBs.
   * @pre `get() != nullptr`
   */
  void VSSetConstantBuffers(UINT StartSlot, UINT NumBuffers,
    ID3D11Buffer* const* ppConstantBuffers);

  // --------------------------
  // PIXEL SHADER (PS)
  // --------------------------

  /**
   * @brief Enlaza el Pixel Shader (y sus class-instances si aplica).
   * @param pPS               Shader.
   * @param ppClassInstances  Array de class-instances (o `nullptr`).
   * @param NumClassInstances Número de class-instances.
   * @pre `get() != nullptr`
   */
  void PSSetShader(ID3D11PixelShader* pPS,
    ID3D11ClassInstance* const* ppClassInstances,
    UINT NumClassInstances);

  /**
   * @brief Enlaza constant-buffers al PS.
   * @param StartSlot Slot inicial.
   * @param NumBuffers Número de CBs.
   * @param ppConstantBuffers Array de CBs.
   * @pre `get() != nullptr`
   */
  void PSSetConstantBuffers(UINT StartSlot, UINT NumBuffers,
    ID3D11Buffer* const* ppConstantBuffers);

  /**
   * @brief Enlaza Shader Resource Views (texturas, etc.) al PS.
   * @param StartSlot Slot inicial.
   * @param NumViews  Número de SRVs.
   * @param ppShaderResourceViews Array de SRVs.
   * @pre `get() != nullptr`
   */
  void PSSetShaderResources(UINT StartSlot, UINT NumViews,
    ID3D11ShaderResourceView* const* ppShaderResourceViews);

  /**
   * @brief Enlaza Sampler States al PS.
   * @param StartSlot Slot inicial.
   * @param NumSamplers Número de samplers.
   * @param ppSamplers  Array de samplers.
   * @pre `get() != nullptr`
   */
  void PSSetSamplers(UINT StartSlot, UINT NumSamplers,
    ID3D11SamplerState* const* ppSamplers);

  // --------------------------
  // Acceso crudo
  // --------------------------

  /**
   * @brief Devuelve el puntero crudo al `ID3D11DeviceContext`.
   * @return Puntero al contexto (puede ser `nullptr` si no está inicializado).
   */
  ID3D11DeviceContext* get() const { return m_deviceContext; }

private:
  /**
   * @brief Puntero al contexto inmediato (propiedad compartida).
   * @details La clase mantiene una referencia cuando se adjunta (`attach`); se libera en `destroy()`.
   */
  ID3D11DeviceContext* m_deviceContext = nullptr; // no-owning
};
