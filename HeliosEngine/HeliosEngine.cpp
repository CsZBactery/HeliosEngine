#include "../include/Prerequisites.h"
#include "../include/BaseApp.h"

// Entry Point oficial de Windows.
int WINAPI
wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) {

    // Instanciamos la aplicación usando el constructor por defecto
    BaseApp app;

    // Lanzamos el bucle principal pasando los handles de Windows
    return app.run(hInstance, nCmdShow);
}