// ======================================================================================
// Archivo: GUI.cpp
// Implementación de la Interfaz de Usuario del Editor usando ImGui e ImGuizmo.
// Tema Personalizado: Industrial Dark (Estilo UE5)
// ======================================================================================

#include "EngineUtilities/GUI/GUI.h"
#include "Viewport.h"
#include "Window.h"
#include "Device.h"
#include "DeviceContext.h"
#include "MeshComponent.h"
#include "ECS/Actor.h"
#include "EngineUtilities/Utilities/Camera.h"

// Variable global estática para el control del Gizmo
static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::TRANSLATE);

// ======================================================================================
// CONFIGURACIÓN INICIAL
// ======================================================================================
void
GUI::awake() {
    // Configuración previa si fuera necesaria
}

void
GUI::init(Window& window, Device& device, DeviceContext& deviceContext) {
    // 1. Crear contexto de ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // Habilitar Teclado y Docking (Anclaje de ventanas)
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;   // Habilitar ventanas fuera de la app principal

    // 2. Configurar Estilo Base
    ImGui::StyleColorsDark();

    // Tweaks para Viewports
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // 3. Aplicar tema visual personalizado (Industrial UE5 Style)
    appleLiquidStyle(0.98f, ImVec4(0.8f, 0.4f, 0.0f, 1.0f)); // Naranja de acento

    // 4. Inicializar Backends
    ImGui_ImplWin32_Init(window.m_hWnd);
    ImGui_ImplDX11_Init(device.m_device, deviceContext.m_deviceContext);

    toolTipData();
    selectedActorIndex = 0;
    m_isInitialized = true;
}

// ======================================================================================
// PREPARACIÓN DE FRAME
// ======================================================================================
void
GUI::update(Viewport& viewport, Window& window) {
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGuizmo::BeginFrame();
    ImGuizmo::SetOrthographic(false);

    drawStudioTopRibbon(); // Menú superior y herramientas
    drawEditorDockspace(); // Sistema de anclaje 

    closeApp();
}

// ======================================================================================
// ENVÍO A LA GPU
// ======================================================================================
void
GUI::render() {
    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }
}

// ======================================================================================
// LIMPIEZA DE MEMORIA
// ======================================================================================
void
GUI::destroy() {
    if (!m_isInitialized) return;

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();

    if (ImGui::GetCurrentContext()) {
        ImGui::DestroyContext();
    }
    m_isInitialized = false;
}

// ======================================================================================
// PANEL: VIEWPORT
// ======================================================================================
void
GUI::drawViewportPanel(ID3D11ShaderResourceView* viewportSRV) {
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    if (ImGui::Begin("Viewport", nullptr, flags)) {
        m_viewportDrawList = ImGui::GetWindowDrawList();

        ImVec2 panelMin = ImGui::GetCursorScreenPos();
        ImVec2 panelSize = ImGui::GetContentRegionAvail();

        if (panelSize.x < 1.0f) panelSize.x = 1.0f;
        if (panelSize.y < 1.0f) panelSize.y = 1.0f;

        m_viewportPos = panelMin;
        m_viewportSize = panelSize;

        if (viewportSRV) {
            ImGui::Image((ImTextureID)viewportSRV, panelSize);
        }
        else {
            ImDrawList* drawList = ImGui::GetWindowDrawList();
            ImVec2 panelMax(panelMin.x + panelSize.x, panelMin.y + panelSize.y);
            drawList->AddRectFilled(panelMin, panelMax, IM_COL32(15, 15, 15, 255));
            drawList->AddText(ImVec2(panelMin.x + 12.0f, panelMin.y + 12.0f), IM_COL32(220, 220, 220, 255), "Viewport sin textura");
        }

        m_viewportHovered = ImGui::IsItemHovered();
        m_viewportActive = ImGui::IsItemActive();
        m_viewportFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
    }
    ImGui::End();
    ImGui::PopStyleVar();
}

// ======================================================================================
// GIZMOS: Manipulación 3D 
// ======================================================================================
void
GUI::editTransform(Camera& cam, Window& window, EU::TSharedPointer<Actor> actor) {
    if (!actor.get()) return;

    auto transform = actor->getComponent<Transform>();
    if (!transform.get()) return;

    float rectX = m_viewportPos.x;
    float rectY = m_viewportPos.y;
    float rectW = m_viewportSize.x;
    float rectH = m_viewportSize.y;

    if (rectW < 64.0f || rectH < 64.0f) {
        m_isUsingGizmo = false;
        return;
    }

    float* pos = const_cast<float*>(transform->getPosition().data());
    float* rot = const_cast<float*>(transform->getRotation().data());
    float* sca = const_cast<float*>(transform->getScale().data());

    float mArr[16];
    ImGuizmo::RecomposeMatrixFromComponents(pos, rot, sca, mArr);

    float vArr[16], pArr[16];
    ToFloatArray(cam.getView(), vArr);
    ToFloatArray(cam.getProj(), pArr);

    ImGuizmo::SetOrthographic(false);

    if (m_viewportDrawList)
        ImGuizmo::SetDrawlist(m_viewportDrawList);
    else
        ImGuizmo::SetDrawlist(ImGui::GetForegroundDrawList());

    ImGuizmo::SetID(0);
    ImGuizmo::SetGizmoSizeClipSpace(0.15f);
    ImGuizmo::AllowAxisFlip(false);
    ImGuizmo::SetRect(rectX, rectY, rectW, rectH);

    float snapValue = 25.0f;
    if (mCurrentGizmoOperation == ImGuizmo::ROTATE)    snapValue = 5.0f;
    if (mCurrentGizmoOperation == ImGuizmo::TRANSLATE) snapValue = 0.5f;

    float snap[3] = { snapValue, snapValue, snapValue };
    bool useSnap = ImGui::GetIO().KeyCtrl;

    if (m_showGizmo) {
        ImGuizmo::Manipulate(vArr, pArr, mCurrentGizmoOperation, mCurrentGizmoMode, mArr, nullptr, useSnap ? snap : nullptr);
    }

    m_isUsingGizmo = ImGuizmo::IsUsing();

    if (m_isUsingGizmo) {
        float newPos[3], newRot[3], newSca[3];
        ImGuizmo::DecomposeMatrixToComponents(mArr, newPos, newRot, newSca);
        transform->setPosition(EU::Vector3(newPos[0], newPos[1], newPos[2]));
        transform->setRotation(EU::Vector3(newRot[0], newRot[1], newRot[2]));
        transform->setScale(EU::Vector3(newSca[0], newSca[1], newSca[2]));
    }
}

// ======================================================================================
// PANEL: OUTLINER (Jerarquía)
// ======================================================================================
void
GUI::outliner(const std::vector<EU::TSharedPointer<Actor>>& actors) {
    if (!m_showExplorer) return;

    ImGui::Begin("Hierarchy");

    static ImGuiTextFilter filter;
    filter.Draw("Search...", 180.0f);
    ImGui::Separator();

    for (int i = 0; i < actors.size(); ++i) {
        const auto& actor = actors[i];
        std::string actorName = actor ? actor->getName() : "Actor";

        if (!filter.PassFilter(actorName.c_str())) continue;

        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
        if (selectedActorIndex == i) flags |= ImGuiTreeNodeFlags_Selected;

        bool nodeOpen = ImGui::TreeNodeEx((void*)(intptr_t)i, flags, "%s", actorName.c_str());

        if (ImGui::IsItemClicked()) selectedActorIndex = i;

        if (nodeOpen) {
            ImGui::Text("Type: Static Mesh");
            ImGui::TreePop();
        }
    }
    ImGui::End();
}

// ======================================================================================
// PANEL: INSPECTOR (Propiedades)
// ======================================================================================
void
GUI::inspectorGeneral(EU::TSharedPointer<Actor> actor) {
    if (!m_showProperties) return;
    if (!actor.get()) return;

    ImGui::Begin("Inspector");

    bool isStatic = false;
    ImGui::Checkbox("##Static", &isStatic);
    ImGui::SameLine();

    char objectName[128];
    strncpy_s(objectName, actor->getName().c_str(), sizeof(objectName));

    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.6f);

    if (ImGui::InputText("##ObjectName", objectName, IM_ARRAYSIZE(objectName))) {
        actor->setName(std::string(objectName));
    }
    ImGui::Separator();

    if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
        inspectorContainer(actor);
    }
    ImGui::End();
}

void
GUI::inspectorContainer(EU::TSharedPointer<Actor> actor) {
    auto transform = actor->getComponent<Transform>();
    if (!transform.get()) return;

    EU::Vector3 pos = transform->getPosition();
    EU::Vector3 rot = transform->getRotation();
    EU::Vector3 scl = transform->getScale();

    float fPos[3] = { pos.x, pos.y, pos.z };
    float fRot[3] = { rot.x, rot.y, rot.z };
    float fScl[3] = { scl.x, scl.y, scl.z };

    vec3Control("Position", fPos);
    vec3Control("Rotation", fRot);
    vec3Control("Scale", fScl, 1.0f);

    transform->setPosition(EU::Vector3(fPos[0], fPos[1], fPos[2]));
    transform->setRotation(EU::Vector3(fRot[0], fRot[1], fRot[2]));
    transform->setScale(EU::Vector3(fScl[0], fScl[1], fScl[2]));
}

// ======================================================================================
// HELPER: Controles XYZ 
// ======================================================================================
void
GUI::vec3Control(const std::string& label, float* values, float resetValue, float columnWidth) {
    ImGuiIO& io = ImGui::GetIO();
    auto boldFont = io.Fonts->Fonts[0];

    ImGui::PushID(label.c_str());
    ImGui::Columns(2);
    ImGui::SetColumnWidth(0, columnWidth);
    ImGui::Text(label.c_str());
    ImGui::NextColumn();
    ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

    float lineHeight = ImGui::GetFontSize() + GImGui->Style.FramePadding.y * 2.0f;
    ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

    // X
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.7f, 0.1f, 0.15f, 1.0f });
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.8f, 0.2f, 0.2f, 1.0f });
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.7f, 0.1f, 0.15f, 1.0f });
    ImGui::PushFont(boldFont);
    if (ImGui::Button("X", buttonSize)) values[0] = resetValue;
    ImGui::PopFont();
    ImGui::PopStyleColor(3);
    ImGui::SameLine();
    ImGui::DragFloat("##X", &values[0], 0.1f, 0.0f, 0.0f, "%.2f");
    ImGui::PopItemWidth();
    ImGui::SameLine();

    // Y
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.6f, 0.2f, 1.0f });
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.7f, 0.3f, 1.0f });
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.6f, 0.2f, 1.0f });
    ImGui::PushFont(boldFont);
    if (ImGui::Button("Y", buttonSize)) values[1] = resetValue;
    ImGui::PopFont();
    ImGui::PopStyleColor(3);
    ImGui::SameLine();
    ImGui::DragFloat("##Y", &values[1], 0.1f, 0.0f, 0.0f, "%.2f");
    ImGui::PopItemWidth();
    ImGui::SameLine();

    // Z
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.7f, 1.0f });
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.8f, 1.0f });
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.7f, 1.0f });
    ImGui::PushFont(boldFont);
    if (ImGui::Button("Z", buttonSize)) values[2] = resetValue;
    ImGui::PopFont();
    ImGui::PopStyleColor(3);
    ImGui::SameLine();
    ImGui::DragFloat("##Z", &values[2], 0.1f, 0.0f, 0.0f, "%.2f");
    ImGui::PopItemWidth();

    ImGui::PopStyleVar();
    ImGui::Columns(1);
    ImGui::PopID();
}

// ======================================================================================
// HELPER: Dockspace
// ======================================================================================
void
GUI::drawEditorDockspace() {
    ImGuiViewport* mainViewport = ImGui::GetMainViewport();
    const float topOffset = 96.0f; // 24 menu + 72 ribbon

    ImVec2 dockPos = ImVec2(mainViewport->Pos.x, mainViewport->Pos.y + topOffset);
    ImVec2 dockSize = ImVec2(mainViewport->Size.x, mainViewport->Size.y - topOffset);

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_MenuBar;

    ImGui::SetNextWindowPos(dockPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(dockSize, ImGuiCond_Always);
    ImGui::SetNextWindowViewport(mainViewport->ID);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGui::Begin("##MainEditorDockspace", nullptr, window_flags);
    ImGuiID dockspace_id = ImGui::GetID("##EditorDockspace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
    ImGui::End();

    ImGui::PopStyleVar(3);
}

void
GUI::closeApp() {
    if (m_showExitPopup) {
        ImGui::OpenPopup("Exit?");
        m_showExitPopup = false;
    }
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("Exit?", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Estas a punto de salir de la aplicacion.\nEstas seguro?\n\n");
        ImGui::Separator();

        if (ImGui::Button("OK", ImVec2(120, 0))) { exit(0); }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
        ImGui::EndPopup();
    }
}

// ======================================================================================
// HELPER: Tema Visual Oscuro Industrial (Estilo Unreal Engine 5)
// ======================================================================================
void
GUI::appleLiquidStyle(float opacity, ImVec4 accent) {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // Cambiamos el estilo redondo y suave por uno más angular y técnico
    style.WindowRounding = 2.0f;
    style.FrameRounding = 2.0f;
    style.ScrollbarRounding = 2.0f;
    style.GrabRounding = 2.0f;
    style.PopupRounding = 2.0f;
    style.ChildRounding = 2.0f;
    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 1.0f;

    // Gris casi negro absoluto para los fondos (Look Industrial)
    colors[ImGuiCol_WindowBg] = ImVec4(0.05f, 0.05f, 0.05f, opacity);
    colors[ImGuiCol_ChildBg] = ImVec4(0.08f, 0.08f, 0.08f, opacity);
    colors[ImGuiCol_PopupBg] = ImVec4(0.05f, 0.05f, 0.05f, 0.98f);

    // Tonos medios para cabeceras
    colors[ImGuiCol_Header] = ImVec4(0.15f, 0.15f, 0.15f, 1.0f);
    colors[ImGuiCol_HeaderHovered] = accent;
    colors[ImGuiCol_HeaderActive] = accent;

    // Botones oscuros que brillan con el acento (Naranja) al pasar el mouse
    colors[ImGuiCol_Button] = ImVec4(0.15f, 0.15f, 0.15f, 1.0f);
    colors[ImGuiCol_ButtonHovered] = accent;
    colors[ImGuiCol_ButtonActive] = ImVec4(accent.x * 0.8f, accent.y * 0.8f, accent.z * 0.8f, 1.0f);

    // Fondos de cajas de texto
    colors[ImGuiCol_FrameBg] = ImVec4(0.02f, 0.02f, 0.02f, 1.0f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.10f, 0.10f, 0.10f, 1.0f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.15f, 0.15f, 0.15f, 1.0f);

    colors[ImGuiCol_TitleBg] = ImVec4(0.02f, 0.02f, 0.02f, 1.0f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.02f, 0.02f, 0.02f, 1.0f);

    // Pestañas (Tabs)
    colors[ImGuiCol_Tab] = ImVec4(0.08f, 0.08f, 0.08f, 1.0f);
    colors[ImGuiCol_TabHovered] = accent;
    colors[ImGuiCol_TabActive] = accent;
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.05f, 0.05f, 0.05f, 1.0f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.08f, 0.08f, 0.08f, 1.0f);

    colors[ImGuiCol_DockingPreview] = accent;
    colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.05f, 0.05f, 0.05f, 1.0f);
    colors[ImGuiCol_Border] = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
}

// ======================================================================================
// MENÚ SUPERIOR Y BARRA DE HERRAMIENTAS
// ======================================================================================
void
GUI::drawStudioTopRibbon() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Exit", "Alt+F4")) m_showExitPopup = true;
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit")) ImGui::EndMenu();
        if (ImGui::BeginMenu("View")) ImGui::EndMenu();
        if (ImGui::BeginMenu("Plugins")) ImGui::EndMenu();
        if (ImGui::BeginMenu("Help")) ImGui::EndMenu();
        ImGui::EndMainMenuBar();
    }

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + ImGui::GetFrameHeight()));
    ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, 72.0f));

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoDocking;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 10.0f));
    ImGui::Begin("Toolbar", nullptr, flags);

    ImVec2 btnSize(65.0f, 50.0f);

    // ======================================================
    // GRUPO 1: TRANSFORMACIÓN (Gizmos)
    // ======================================================
    bool isSelect = !m_showGizmo;
    if (isSelect) ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
    if (ImGui::Button("Select\n(Q)", btnSize)) m_showGizmo = false;
    if (isSelect) ImGui::PopStyleColor();
    ImGui::SameLine();

    bool isMove = (mCurrentGizmoOperation == ImGuizmo::TRANSLATE && m_showGizmo);
    if (isMove) ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
    if (ImGui::Button("Move\n(W)", btnSize)) { mCurrentGizmoOperation = ImGuizmo::TRANSLATE; m_showGizmo = true; }
    if (isMove) ImGui::PopStyleColor();
    ImGui::SameLine();

    bool isScale = (mCurrentGizmoOperation == ImGuizmo::SCALE && m_showGizmo);
    if (isScale) ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
    if (ImGui::Button("Scale\n(R)", btnSize)) { mCurrentGizmoOperation = ImGuizmo::SCALE; m_showGizmo = true; }
    if (isScale) ImGui::PopStyleColor();
    ImGui::SameLine();

    bool isRotate = (mCurrentGizmoOperation == ImGuizmo::ROTATE && m_showGizmo);
    if (isRotate) ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
    if (ImGui::Button("Rotate\n(E)", btnSize)) { mCurrentGizmoOperation = ImGuizmo::ROTATE; m_showGizmo = true; }
    if (isRotate) ImGui::PopStyleColor();

    // ======================================================
    // GRUPO 2: CREACIÓN Y EDICIÓN
    // ======================================================
    ImGui::SameLine(0, 15.0f); // Espacio extra antes del separador
    ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
    ImGui::SameLine(0, 15.0f); // Espacio extra después del separador

    if (ImGui::Button("Part\nMesh", btnSize)) {
        m_requestSpawnCube = true; // Avisamos al motor que queremos instanciar
    }
    ImGui::SameLine();
    if (ImGui::Button("Terrain\nEdit", btnSize)) {
        // Lógica futura de terreno
    }
    ImGui::SameLine();
    if (ImGui::Button("Material\nEditor", btnSize)) {
        // Lógica futura de materiales
    }
    ImGui::SameLine();
    if (ImGui::Button("Color\nPicker", btnSize)) {
        // Lógica futura de color
    }

    // ======================================================
    // GRUPO 3: GESTIÓN DE PANELES
    // ======================================================
    ImGui::SameLine(0, 15.0f);
    ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
    ImGui::SameLine(0, 15.0f);

    // Botón Explorer (Hierarchy)
    if (m_showExplorer) ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
    if (ImGui::Button("Explorer\nPanel", btnSize)) { m_showExplorer = !m_showExplorer; }
    if (m_showExplorer) ImGui::PopStyleColor();

    ImGui::SameLine();

    // Botón Properties (Inspector)
    if (m_showProperties) ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
    if (ImGui::Button("Properties\nPanel", btnSize)) { m_showProperties = !m_showProperties; }
    if (m_showProperties) ImGui::PopStyleColor();

    ImGui::SameLine();

    // Botón Toolbox
    if (ImGui::Button("Toolbox\nAssets", btnSize)) {
        // Lógica futura del Toolbox
    }

    ImGui::End();
    ImGui::PopStyleVar();
}

// Funciones vacías necesarias para la declaración en el .h
void GUI::ToolBar() {}
void GUI::toolTipData() {}
void GUI::drawGizmoToolbar() {}