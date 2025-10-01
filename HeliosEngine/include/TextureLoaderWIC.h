/**
 * @file TextureLoaderWIC.h
 * @brief Carga texturas con WIC (PNG/JPG/BMP/etc.) y crea un Shader Resource View (SRV) de D3D11.
 *
 * Este helper convierte la imagen a 32 bpp BGRA y genera una textura 2D con opción de mipmaps.
 * Pensado para pipelines simples sin dependencias de D3DX/DirectXTK.
 *
 * @note Requiere Windows Imaging Component (WIC) y enlazar con @c windowscodecs.lib.
 * @see https://learn.microsoft.com/windows/win32/wic/-wic-start
 */

#pragma once
#include <d3d11.h>
#include <wincodec.h> // IWIC*
#include <cstdint>

 /**
  * @def SAFE_RELEASE
  * @brief Macro utilitario para liberar interfaces COM de forma segura.
  *
  * @param p Puntero a una interfaz COM (por ejemplo, @c IUnknown* o derivados).
  *
  * @warning Solo debe usarse con interfaces COM (que proveen @c Release()).
  */
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p) do { if(p){ (p)->Release(); (p)=nullptr; } } while(0)
#endif

  /**
   * @brief Carga una imagen (PNG/JPG/BMP/etc.) vía WIC y crea su @c ID3D11ShaderResourceView.
   *
   * @details
   * - La imagen se convierte a 32bpp BGRA y se sube a una @c ID3D11Texture2D con formato
   *   @c DXGI_FORMAT_B8G8R8A8_UNORM (compatibilidad directa con WIC).
   * - Si @p generateMips es @c true: la textura se crea con @c MipLevels=0,
   *   @c BIND_RENDER_TARGET y @c MISC_GENERATE_MIPS; se sube el nivel 0 y se llama
   *   a @c ID3D11DeviceContext::GenerateMips().
   * - Si @p generateMips es @c false: la textura se crea con un único mip y datos iniciales.
   *
   * @param device        Dispositivo de D3D11 válido (no nulo).
   * @param context       Contexto inmediato de D3D11 (obligatorio si @p generateMips es @c true).
   * @param filePath      Ruta Unicode del archivo de imagen (png/jpg/bmp/..).
   * @param outSRV        [out] Recibe el SRV creado. Debe liberarse con @c Release() por el llamador.
   * @param outWidth      [out, opcional] Ancho de la imagen cargada (en píxeles).
   * @param outHeight     [out, opcional] Alto de la imagen cargada (en píxeles).
   * @param generateMips  Si es @c true, autogenera la cadena de mipmaps (requiere @p context).
   *
   * @return
   * - @c S_OK en éxito.
   * - Un @c HRESULT de error si falla la lectura/decodificación WIC o la creación de recursos D3D11.
   *
   * @pre
   * - Si usas este helper en un hilo que no haya inicializado COM, asegúrate de llamar a
   *   @c CoInitializeEx(nullptr, COINIT_MULTITHREADED) antes (y @c CoUninitialize() al finalizar).
   *   Alternativamente, implementa un guard en el .cpp que lo llame internamente.
   *
   * @post
   * - @p outSRV contendrá un SRV válido listo para hacer @c PSSetShaderResources(0,1,&srv).
   *
   * @note
   * - El directorio de trabajo (Working Directory) debe hacer que @p filePath sea resoluble
   *   o usa rutas absolutas.
   * - Si necesitas corrección de gamma, considera usar @c DXGI_FORMAT_B8G8R8A8_UNORM_SRGB y
   *   un swap chain/RTV en sRGB.
   *
   * @code{.cpp}
   * // Ejemplo de uso:
   * ID3D11ShaderResourceView* srv = nullptr;
   * HRESULT hr = LoadTextureWIC(device, context,
   *                             L"Assets\\Textures\\checker_256.png",
   *                             &srv, nullptr, nullptr, true);
   * if (SUCCEEDED(hr)) {
   *   context->PSSetShaderResources(0, 1, &srv);
   *   // ...
   *   srv->Release();
   * }
   * @endcode
   */
HRESULT LoadTextureWIC(
  ID3D11Device* device,
  ID3D11DeviceContext* context,
  const wchar_t* filePath,
  ID3D11ShaderResourceView** outSRV,
  UINT* outWidth = nullptr,
  UINT* outHeight = nullptr,
  bool generateMips = true
);
