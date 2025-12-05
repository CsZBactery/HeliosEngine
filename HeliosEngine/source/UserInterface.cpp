#include "../include/UserInterface.h"

UserInterface::UserInterface() {
    // Constructor vacio
}

UserInterface::~UserInterface() {
    // Destructor vacio
}

void
UserInterface::init(void* window, ID3D11Device* device, ID3D11DeviceContext* deviceContext) {
    // Verificamos version y creamos el contexto global de ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    // Configuracion de IO (Input/Output)
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Permitir navegar con teclado

    // Habilitar Docking: Esto permite arrastrar ventanas y pegarlas unas con otras (estilo editor)
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    // Estilo visual por defecto (Dark Mode, el mejor)
    ImGui::StyleColorsDark();

    // Ajustes de estilo para cuando se sacan ventanas fuera de la principal (Viewports)
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Inicializamos los "backends": Plataforma (Windows) y Render (DX11)
    ImGui_ImplWin32_Init(window);
    ImGui_ImplDX11_Init(device, deviceContext);
}

void
UserInterface::update() {
    // Le decimos a ImGui que empieza un nuevo frame.
    // A partir de aqui, en cualquier parte del codigo (BaseApp::update),
    // podemos llamar a ImGui::Begin() y dibujar ventanas.
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

void
UserInterface::render() {
    // Finaliza el frame y genera la geometria para dibujar
    ImGui::Render();

    // Dibuja los datos de ImGui usando DirectX
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    // Logica extra para Viewports (ventanas flotantes fuera de la app)
    // Si arrastras una ventana fuera, ImGui crea una ventana de Windows nativa para ella.
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }
}

void
UserInterface::destroy() {
    // Limpieza en orden inverso a la inicializacion
    if (ImGui::GetCurrentContext()) {
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
    }
}

void
UserInterface::vec3Control(std::string label, float* values, float resetValues, float columnWidth) {
    // (Pendiente de implementacion: Aqui iria la logica para dibujar 3 sliders X,Y,Z)
}