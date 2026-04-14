/**
 * @file GUI.h
 * @brief Interfaz Gráfica de Usuario (Editor) basada en ImGui e ImGuizmo para HeliosEngine.
 */

#pragma once
#include "Prerequisites.h"

 // ImGui & ImGuizmo Includes
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include <imgui_internal.h>
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include "ImGuizmo.h"

class Viewport;
class Window;
class Device;
class DeviceContext;
class Actor;
class Camera;

/**
 * @class GUI
 * @brief Orquesta todos los paneles del editor, la gestión de ventanas y herramientas de transformación.
 * @details Se encarga de dibujar el Outliner (jerarquía), el Inspector (propiedades),
 * los Gizmos (manipuladores 3D) y el Dockspace que organiza el layout del motor.
 */
class GUI {
public:
    GUI() = default;
    ~GUI() = default;

    /** @brief Configuración inicial previa a la creación de la ventana. */
    void awake();

    /** @brief Inicializa ImGui y sus backends para Win32 y DirectX 11. */
    void init(Window& window, Device& device, DeviceContext& deviceContext);

    /** @brief Inicia el frame de UI, gestiona el input del Viewport y el Docking. */
    void update(Viewport& viewport, Window& window);

    /** @brief Envía los datos de dibujo de ImGui al pipeline de renderizado. */
    void render();

    /** @brief Libera los recursos de ImGui y cierra los backends. */
    void destroy();

    // -----------------------------------------------------------
    // PANELES Y WIDGETS
    // -----------------------------------------------------------

    /** @brief Barra de herramientas superior (File, Edit, etc.). */
    void ToolBar();

    /** @brief Diálogo de confirmación para cerrar la aplicación. */
    void closeApp();

    /** @brief Carga de datos de ayuda y tooltips para los botones. */
    void toolTipData();

    /** @brief Estilizado visual estilo macOS / Apple Liquid / UE5. */
    void appleLiquidStyle(float opacity = 1.0f, ImVec4 accent = ImVec4(0.04f, 0.52f, 1.0f, 1.0f));

    /** @brief Widget personalizado para edición de vectores XYZ (Transform). */
    void vec3Control(const std::string& label, float* values, float resetValue = 0.0f, float columnWidth = 100.0f, bool displayAsDegrees = false);

    /** @brief Panel de propiedades del Actor seleccionado. */
    void inspectorGeneral(EU::TSharedPointer<Actor> actor);
    void inspectorContainer(EU::TSharedPointer<Actor> actor);

    /** @brief Lista de todos los actores presentes en la escena actual. */
    void outliner(const std::vector<EU::TSharedPointer<Actor>>& actors);

    /** @brief Dibuja la cinta superior (Studio Top Ribbon) de opciones rápidas. */
    void drawStudioTopRibbon();

    /** @brief Renderiza la textura del juego dentro de un panel de ImGui. */
    void drawViewportPanel(ID3D11ShaderResourceView* viewportSRV);

    /** @brief Gestiona el sistema de anclaje de ventanas (Dockspace). */
    void drawEditorDockspace();

    // -----------------------------------------------------------
    // GIZMOS Y MATRICES
    // -----------------------------------------------------------

    /** @brief Dibuja y gestiona los manipuladores de transformación 3D en el viewport. */
    void editTransform(Camera& cam, Window& window, EU::TSharedPointer<Actor> actor);

    /** @brief Botonera flotante para cambiar entre Traslación, Rotación y Escala. */
    void drawGizmoToolbar();

    /** @brief Convierte matrices XMMATRIX al formato de array plano que requiere ImGuizmo. */
    void ToFloatArray(const XMMATRIX& mat, float* dest) {
        XMFLOAT4X4 temp;
        XMStoreFloat4x4(&temp, mat);
        memcpy(dest, &temp, sizeof(float) * 16);
    }

    /** @brief Consume la petición de guardado lanzada por la UI (NUEVO DEL PROFE). */
    bool consumeSaveSceneRequest() {
        bool req = m_requestSaveScene;
        m_requestSaveScene = false;
        return req;
    }

private:
    bool m_checkboxValue = true;
    bool m_checkboxValue2 = false;
    std::vector<const char*> m_objectsNames;
    std::vector<const char*> m_tooltips;

    bool m_showExitPopup = false;
    ImDrawList* m_viewportDrawList = nullptr;
    bool m_viewportActive = false;

    bool m_requestSaveScene = false; // <-- Almacena si el usuario presionó Guardar Escena

public:
    // Estados de interacción y lógica
    bool m_isUsingGizmo = false;
    bool m_showGizmo = false;
    bool m_requestSpawnCube = false; // <-- Bandera para decirle al motor que instancie algo
    int  selectedActorIndex = -1;
    bool m_isInitialized = false;

    // Estados de Paneles
    bool m_showExplorer = true;      // <-- Controla la visibilidad del Outliner (Hierarchy)
    bool m_showProperties = true;    // <-- Controla la visibilidad del Inspector

    // Datos del Viewport del Editor
    ImVec2 m_viewportPos = ImVec2(0.0f, 0.0f);
    ImVec2 m_viewportSize = ImVec2(0.0f, 0.0f);
    bool   m_viewportHovered = false;
    bool   m_viewportFocused = false;

    // Configuración de ImGuizmo
    ImGuizmo::OPERATION mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
    ImGuizmo::MODE      mCurrentGizmoMode = ImGuizmo::WORLD;
};