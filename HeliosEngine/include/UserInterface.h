#pragma once
#include "Prerequisites.h"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <imgui_internal.h>

/**
 * @class UserInterface
 * @brief Wrapper para la biblioteca "Dear ImGui".
 *
 * Esta clase encapsula la inicialización, actualización y renderizado de la interfaz
 * gráfica de depuración (GUI). ImGui funciona en "Modo Inmediato", lo que significa
 * que la interfaz se reconstruye y dibuja completamente en cada fotograma.
 */
class UserInterface {
public:
    /**
     * @brief Constructor por defecto.
     */
    UserInterface();

    /**
     * @brief Destructor por defecto.
     */
    ~UserInterface();

    /**
     * @brief Inicializa el contexto de ImGui y los backends de plataforma/renderer.
     *
     * Configura los estilos visuales, conecta con la ventana de Windows (Win32)
     * y prepara el dispositivo gráfico (DX11) para dibujar la UI.
     *
     * @param window Puntero al manejador de la ventana (HWND cast a void*).
     * @param device Dispositivo DirectX para crear fuentes y texturas de la UI.
     * @param deviceContext Contexto para los comandos de dibujo.
     */
    void
        init(void* window, ID3D11Device* device, ID3D11DeviceContext* deviceContext);

    /**
     * @brief Inicia un nuevo frame de la interfaz (NewFrame).
     *
     * Esta función debe llamarse al principio del ciclo de renderizado (antes de updatear
     * actores). Prepara a ImGui para recibir nuevos comandos de ventanas, botones, textos, etc.
     */
    void
        update();

    /**
     * @brief Finaliza el frame y envía los comandos de dibujo a la GPU.
     *
     * Debe llamarse al final del ciclo de renderizado, justo antes del SwapChain::present().
     * Convierte los datos de ImGui en vértices y los dibuja sobre la escena 3D.
     */
    void
        render();

    /**
     * @brief Cierra el contexto de ImGui y libera la memoria.
     */
    void
        destroy();

    /**
     * @brief Dibuja un control personalizado para vectores de 3 componentes (X, Y, Z).
     *
     * Utilidad para el inspector de objetos. Dibuja 3 cajas de texto numérico con etiquetas
     * y permite resetear los valores a un defecto.
     *
     * @param label Etiqueta principal del control (ej: "Position").
     * @param values Puntero al array de floats o struct (x, y, z) a modificar.
     * @param resetValues Valor al que volverán los campos si se resetean (default 0.0f).
     * @param columnWidth Ancho de la etiqueta de texto para alinear columnas.
     */
    void
        vec3Control(std::string label,
            float* values,
            float resetValues = 0.0f,
            float columnWidth = 100.0f);

private:
    // Aquí se podrían guardar punteros internos si fuera necesario en el futuro,
    // aunque ImGui gestiona su propio contexto globalmente.
};