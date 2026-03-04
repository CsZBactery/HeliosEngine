// ======================================================================================
// Archivo: GUI.cpp
// Implementación de la Interfaz de Usuario usando ImGui e ImGuizmo.
// ======================================================================================

#include "EngineUtilities/GUI/GUI.h"
#include "Window.h"
#include "Device.h"
#include "DeviceContext.h"
#include "Viewport.h"
#include "ECS/Actor.h"
#include "ECS/Transform.h"

// xnamath.h define XMMATRIX en el namespace global, así que no necesitamos usar 'using namespace'.

void GUI::awake() {
    // Configuración previa si fuera necesaria
}

void GUI::init(Window& window, Device& device, DeviceContext& deviceContext) {
    // 1. Crear contexto de ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Habilitar teclado
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;     // Habilitar Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;   // Habilitar Multi-Viewport

    // 2. Configurar Estilo
    appleLiquidStyle(0.95f);

    // 3. Inicializar Backends
    ImGui_ImplWin32_Init(window.m_hWnd);
    ImGui_ImplDX11_Init(device.m_device, deviceContext.m_deviceContext);
}

void GUI::update(Viewport& viewport, Window& window) {
    // Inicio del frame de ImGui
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    ImGuizmo::BeginFrame();

    // Configurar Dockspace Principal
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(main_viewport->WorkPos);
    ImGui::SetNextWindowSize(main_viewport->WorkSize);
    ImGui::SetNextWindowViewport(main_viewport->ID);
    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    // Quitar padding para que el dockspace ocupe el 100% de la ventana
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    // Iniciamos la ventana principal que sirve de ancla para las demás
    ImGui::Begin("DockSpace Demo", nullptr, window_flags);
    ImGui::PopStyleVar();

    ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode); // <-- IMPORTANTE: Permite ver a través del centro

    ImGui::End();

    // Dibujar Toolbar en la parte superior
    ToolBar();
}

void GUI::render() {
    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }
}

void GUI::destroy() {
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

void GUI::ToolBar() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Exit")) closeApp();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("Translate", "W")) mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
            if (ImGui::MenuItem("Rotate", "E"))    mCurrentGizmoOperation = ImGuizmo::ROTATE;
            if (ImGui::MenuItem("Scale", "R"))     mCurrentGizmoOperation = ImGuizmo::SCALE;
            ImGui::Separator();
            if (ImGui::MenuItem("World Space", NULL, mCurrentGizmoMode == ImGuizmo::WORLD)) mCurrentGizmoMode = ImGuizmo::WORLD;
            if (ImGui::MenuItem("Local Space", NULL, mCurrentGizmoMode == ImGuizmo::LOCAL)) mCurrentGizmoMode = ImGuizmo::LOCAL;
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

void GUI::closeApp() {
    PostQuitMessage(0);
}

// -----------------------------------------------------------------------------
// OUTLINER (Lista de actores en la escena)
// -----------------------------------------------------------------------------
void GUI::outliner(const std::vector<EU::TSharedPointer<Actor>>& actors) {
    ImGui::Begin("Outliner");

    for (size_t i = 0; i < actors.size(); ++i) {
        // Verificamos validez del puntero inteligente
        if (!actors[i].get()) continue;

        std::string name = actors[i]->getName();
        if (name.empty()) name = "Actor " + std::to_string(i);

        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
        if (selectedActorIndex == i) {
            flags |= ImGuiTreeNodeFlags_Selected;
        }

        ImGui::TreeNodeEx((void*)(intptr_t)i, flags, name.c_str());

        // Si el usuario hace clic en el nombre, actualizamos el índice seleccionado
        if (ImGui::IsItemClicked()) {
            selectedActorIndex = (int)i;
        }
    }

    ImGui::End();
}

// -----------------------------------------------------------------------------
// INSPECTOR (Propiedades del actor seleccionado)
// -----------------------------------------------------------------------------
void GUI::inspectorGeneral(EU::TSharedPointer<Actor> actor) {
    ImGui::Begin("Inspector");

    // Verificamos con .get() que el puntero sea válido
    if (actor.get()) {
        char buffer[256];
        memset(buffer, 0, sizeof(buffer));
        std::string name = actor->getName();
        strncpy_s(buffer, name.c_str(), sizeof(buffer));

        if (ImGui::InputText("Name", buffer, sizeof(buffer))) {
            actor->setName(std::string(buffer));
        }

        ImGui::Separator();

        auto transform = actor->getComponent<Transform>();
        if (transform.get()) {
            if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {

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
        }
    }
    else {
        ImGui::Text("No actor selected.");
    }

    ImGui::End();
}

// -----------------------------------------------------------------------------
// HELPER: Controles visuales XYZ para el Inspector
// -----------------------------------------------------------------------------
void GUI::vec3Control(const std::string& label, float* values, float resetValues, float columnWidth) {
    ImGui::PushID(label.c_str());

    ImGui::Columns(2);
    ImGui::SetColumnWidth(0, columnWidth);
    ImGui::Text(label.c_str());
    ImGui::NextColumn();

    ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

    float lineHeight = ImGui::GetFontSize() + GImGui->Style.FramePadding.y * 2.0f;
    ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

    // Botón X (Rojo)
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
    if (ImGui::Button("X", buttonSize)) values[0] = resetValues;
    ImGui::PopStyleColor();
    ImGui::SameLine();
    ImGui::DragFloat("##X", &values[0], 0.1f, 0.0f, 0.0f, "%.2f");
    ImGui::PopItemWidth();
    ImGui::SameLine();

    // Botón Y (Verde)
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
    if (ImGui::Button("Y", buttonSize)) values[1] = resetValues;
    ImGui::PopStyleColor();
    ImGui::SameLine();
    ImGui::DragFloat("##Y", &values[1], 0.1f, 0.0f, 0.0f, "%.2f");
    ImGui::PopItemWidth();
    ImGui::SameLine();

    // Botón Z (Azul)
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
    if (ImGui::Button("Z", buttonSize)) values[2] = resetValues;
    ImGui::PopStyleColor();
    ImGui::SameLine();
    ImGui::DragFloat("##Z", &values[2], 0.1f, 0.0f, 0.0f, "%.2f");
    ImGui::PopItemWidth();

    ImGui::PopStyleVar();
    ImGui::Columns(1);
    ImGui::PopID();
}

// -----------------------------------------------------------------------------
// GIZMOS (Flechas de manipulación en pantalla 3D)
// -----------------------------------------------------------------------------
void GUI::editTransform(const XMMATRIX& view, const XMMATRIX& projection, EU::TSharedPointer<Actor> actor) {
    if (!actor.get()) return;

    auto transform = actor->getComponent<Transform>();
    if (!transform.get()) return;

    XMFLOAT4X4 view4x4, proj4x4, world4x4;
    XMStoreFloat4x4(&view4x4, view);
    XMStoreFloat4x4(&proj4x4, projection);
    XMStoreFloat4x4(&world4x4, transform->matrix);

    // 1. Configuramos ImGuizmo para que dibuje en una ventana INVISIBLE que ocupe todo el Viewport central
    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoInputs;

    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);

    // Abrimos la ventana transparente
    ImGui::Begin("GizmoLayer", nullptr, windowFlags);

    // Le indicamos a ImGuizmo que use esta ventana para dibujar
    ImGuizmo::SetDrawlist();
    ImGuizmo::SetRect(viewport->Pos.x, viewport->Pos.y, viewport->Size.x, viewport->Size.y);

    // 2. Llamada a ImGuizmo::Manipulate
    if (ImGuizmo::Manipulate(
        (float*)&view4x4,
        (float*)&proj4x4,
        mCurrentGizmoOperation,
        mCurrentGizmoMode,
        (float*)&world4x4))
    {
        if (ImGuizmo::IsUsing()) {
            float matrixTranslation[3], matrixRotation[3], matrixScale[3];
            ImGuizmo::DecomposeMatrixToComponents((float*)&world4x4, matrixTranslation, matrixRotation, matrixScale);

            transform->setPosition(EU::Vector3(matrixTranslation[0], matrixTranslation[1], matrixTranslation[2]));
            transform->setRotation(EU::Vector3(matrixRotation[0], matrixRotation[1], matrixRotation[2]));
            transform->setScale(EU::Vector3(matrixScale[0], matrixScale[1], matrixScale[2]));
        }
    }

    // Cerramos la ventana transparente
    ImGui::End();
}

// -----------------------------------------------------------------------------
// ESTILO VISUAL (Apple Liquid)
// -----------------------------------------------------------------------------
void GUI::appleLiquidStyle(float opacity, ImVec4 accent) {
    ImGuiStyle& style = ImGui::GetStyle();

    style.Alpha = 1.0f;
    style.DisabledAlpha = 0.60f;
    style.WindowPadding = ImVec2(8.0f, 8.0f);
    style.WindowRounding = 10.0f;
    style.FrameRounding = 6.0f;
    style.PopupRounding = 6.0f;
    style.ScrollbarRounding = 12.0f;
    style.GrabRounding = 6.0f;
    style.TabRounding = 6.0f;

    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.12f, 0.12f, 0.12f, opacity);
    colors[ImGuiCol_TitleBg] = ImVec4(0.12f, 0.12f, 0.12f, 1.0f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.12f, 0.12f, 0.12f, 1.0f);

    colors[ImGuiCol_Button] = ImVec4(accent.x * 0.8f, accent.y * 0.8f, accent.z * 0.8f, 0.6f);
    colors[ImGuiCol_ButtonHovered] = accent;
    colors[ImGuiCol_ButtonActive] = ImVec4(accent.x * 1.2f, accent.y * 1.2f, accent.z * 1.2f, 1.0f);

    colors[ImGuiCol_Header] = ImVec4(accent.x, accent.y, accent.z, 0.4f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(accent.x, accent.y, accent.z, 0.6f);
    colors[ImGuiCol_HeaderActive] = accent;

    colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.54f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.30f, 0.30f, 0.30f, 0.54f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.40f, 0.40f, 0.40f, 0.54f);
}

void GUI::inspectorContainer(EU::TSharedPointer<Actor> actor) {}
void GUI::toolTipData() {}
void GUI::drawGizmoToolbar() {}