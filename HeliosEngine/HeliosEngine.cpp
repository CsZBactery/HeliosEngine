#include "../include/Prerequisites.h"
#include "../include/BaseApp.h"

// Entry Point oficial de Windows.
// Usamos wWinMain en lugar de main() standard para que NO se abra una consola de comandos negra de fondo.
int WINAPI
wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) {

    // Creamos la instancia de nuestra aplicacion.
    // Aqui se pasan los handles de Windows, pero la inicializacion pesada (DirectX) ocurre dentro del .init()
    BaseApp app(hInstance, nCmdShow);

    // Lanzamos el bucle principal (Game Loop).
    // El programa se quedara "atrapado" aqui dentro procesando frames infinitamente
    // hasta que el usuario cierre la ventana o mandemos un PostQuitMessage.
    return app.run(hInstance, nCmdShow);
}