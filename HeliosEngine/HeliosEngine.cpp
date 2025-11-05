#include "../include/Prerequisites.h"
#include "../include/BaseApp.h"

// Punto de entrada de la app en Windows (subsystem: Windows, sin consola).
// hInstance: handle de la instancia actual.
// hPrevInstance: siempre null en Win32 modernas (no se usa).
// lpCmdLine: argumentos de línea de comandos (Unicode).
// nCmdShow: cómo mostrar la ventana al iniciar.
int WINAPI
wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) {
    // Crea la aplicación base (inicializa ventana, DX, etc. en el constructor/Init).
    BaseApp app(hInstance, nCmdShow);

    // Ejecuta el loop principal (procesa mensajes y renderiza frames).O
    return app.run(hInstance, nCmdShow);
}
