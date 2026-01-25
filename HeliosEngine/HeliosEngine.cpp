#include "../include/Prerequisites.h"
#include "../include/BaseApp.h"

// Entry Point oficial de Windows.
int WINAPI
wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) {

    // CORRECCIÓN AQUÍ:
    // Usamos el constructor por defecto (sin paréntesis o vacíos)
    BaseApp app;

    // Lanzamos el bucle principal pasando los handles aquí.
    return app.run(hInstance, nCmdShow);
}