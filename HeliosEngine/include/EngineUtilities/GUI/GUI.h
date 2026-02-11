#pragma once
#include "Prerequisites.h"

// ImGui & ImGuizmo Includes
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include "ImGuizmo.h"

// Forward Declarations
class Viewport;
class Window;
class Device;
class DeviceContext;
class Actor;

/**
 * @class GUI
 * @brief Sistema de interfaz de usuario para el editor (ImGui + ImGuizmo).
 */
class GUI {
public:
    GUI() = default;
    ~GUI() = default;

    void awake();

    /**
     * @brief Inicializa ImGui y sus backends (Win32 / DX11).
     */
    void init(Window& window, Device& device, DeviceContext& deviceContext);

    /**
     * @brief Inicia el frame de ImGui y configura el DockSpace.
     */
    void update(Viewport& viewport, Window& window);

    /**
     * @brief Renderiza los datos de dibujo de ImGui.
     */
    void render();

    /**
     * @brief Limpia los recursos de ImGui.
     */
    void destroy();

    // -----------------------------------------------------------
    // WIDGETS Y PANELES
    // -----------------------------------------------------------

    void ToolBar();
    void closeApp();
    void toolTipData();

    /**
     * @brief Aplica un tema visual estilo macOS/Apple.
     */
    void appleLiquidStyle(float opacity = 1.0f, ImVec4 accent = ImVec4(0.04f, 0.52f, 1.0f, 1.0f));

    /**
     * @brief Control personalizado para editar vectores (X, Y, Z).
     */
    void vec3Control(const std::string& label, float* values, float resetValues = 0.0f, float columnWidth = 100.0f);

    // -----------------------------------------------------------
    // INSPECTOR Y OUTLINER
    // -----------------------------------------------------------

    /**
     * @brief Muestra las propiedades del actor seleccionado.
     */
    void inspectorGeneral(EU::TSharedPointer<Actor> actor);

    void inspectorContainer(EU::TSharedPointer<Actor> actor);

    /**
     * @brief Muestra la lista jerárquica de actores en la escena.
     */
    void outliner(const std::vector<EU::TSharedPointer<Actor>>& actors);

    // -----------------------------------------------------------
    // GIZMOS (Transformación en viewport)
    // -----------------------------------------------------------

    /**
     * @brief Dibuja el manipulador 3D (Gizmo) sobre el actor seleccionado.
     */
    void editTransform(const XMMATRIX& view, const XMMATRIX& projection, EU::TSharedPointer<Actor> actor);

    void drawGizmoToolbar();

    // Helper para conversión de matrices a float array (row-major vs column-major)
    void ToFloatArray(const XMMATRIX& mat, float* dest) {
        XMFLOAT4X4 temp;
        XMStoreFloat4x4(&temp, mat);
        memcpy(dest, &temp, sizeof(float) * 16);
    }

private:
    bool checkboxValue = true;
    bool checkboxValue2 = false;
    std::vector<const char*> m_objectsNames;
    std::vector<const char*> m_tooltips;

    bool show_exit_popup = false;

public:
    int selectedActorIndex = -1; // Índice del actor seleccionado en el vector m_actors

    // Operación actual del Gizmo (Translate, Rotate, Scale)
    ImGuizmo::OPERATION mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
    ImGuizmo::MODE mCurrentGizmoMode = ImGuizmo::WORLD;
};