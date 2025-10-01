#pragma once
/**
 * @file Device.h
 * @brief Wrapper mínimo sobre `ID3D11Device` para utilidades puntuales.
 *
 * @details
 * - Centraliza la propiedad del puntero `ID3D11Device*` y operaciones básicas.
 * - Expone un *thin wrapper* a `CreateRenderTargetView` con validaciones.
 * - El puntero `m_device` se deja **público** por compatibilidad con tu código actual.
 *
 * @note Las dependencias de Win32/D3D11/DirectXMath se incluyen vía `Prerequisites.h`.
 */

#include "Prerequisites.h"

 /**
  * @class Device
  * @brief Encapsula un `ID3D11Device*` y ofrece utilidades mínimas.
  *
  * @details
  * Esta clase NO crea el dispositivo por sí misma; espera que un tercero (p.ej.
  * `D3D11CreateDeviceAndSwapChain`) inicialice `m_device`. Provee:
  * - `destroy()` para liberar el dispositivo de forma segura.
  * - `CreateRenderTargetView(...)` como envoltura validada.
  *
  * @warning No es copiable para evitar doble liberación del `ID3D11Device*`.
  * Si necesitas transferencia de propiedad, considera añadir semántica de movimiento.
  */
class Device {
public:
  /** @brief Ctor por defecto: no adquiere recursos. */
  Device() = default;

  /** @brief Dtor por defecto: no libera automáticamente (usa `destroy()`). */
  ~Device() = default;

  // --------------------------
  // Semántica de copia/movimiento
  // --------------------------

  /**
   * @brief Copia deshabilitada para evitar doble `Release()`.
   */
  Device(const Device&) = delete;

  /**
   * @brief Asignación por copia deshabilitada para evitar doble `Release()`.
   */
  Device& operator=(const Device&) = delete;

  // --------------------------
  // Gestión del recurso
  // --------------------------

  /**
   * @brief Libera el dispositivo si está presente y lo pone a `nullptr`.
   *
   * @details
   * Internamente invoca `SAFE_RELEASE(m_device)`.
   * Debes llamar a este método durante el shutdown de tu app/sistema gráfico,
   * después de limpiar todos los objetos creados por el device.
   *
   * @code
   * // ejemplo de uso
   * g_device.destroy();
   * @endcode
   */
  void destroy();

  // --------------------------
  // Envolturas (thin wrappers)
  // --------------------------

  /**
   * @brief Envoltura con validaciones a `ID3D11Device::CreateRenderTargetView`.
   *
   * @param pResource   Recurso de origen (típicamente el backbuffer o una textura).
   * @param pDesc       Descriptor (puede ser `nullptr` para vista por defecto).
   * @param ppRTView    Salida: puntero a la vista creada (no nulo).
   * @return `S_OK` si tuvo éxito; un `HRESULT` de error en caso contrario.
   *
   * @pre `m_device != nullptr`
   * @pre `pResource != nullptr`
   * @pre `ppRTView != nullptr`
   *
   * @post En éxito, `*ppRTView` contendrá una referencia válida que deberás liberar
   *       con `Release()` cuando ya no la uses.
   */
  HRESULT CreateRenderTargetView(ID3D11Resource* pResource,
    const D3D11_RENDER_TARGET_VIEW_DESC* pDesc,
    ID3D11RenderTargetView** ppRTView);

public:
  /**
   * @brief Puntero al dispositivo D3D11 subyacente.
   *
   * @details
   * Se expone **público** por compatibilidad con código existente (p.ej. creación
   * de buffers/shaders directamente). Si quieres aislar mejor responsabilidades,
   * puedes hacerlo privado y añadir métodos proxy más adelante.
   */
  ID3D11Device* m_device = nullptr;
};