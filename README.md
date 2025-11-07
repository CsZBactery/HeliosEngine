# HeliosEngine

> Este es un compendio de las clases vistas en la materia de Graficas Computacionales 3D. Generación 2026-1.

¡HeliosEngine en acción, cargando un modelo `.obj` con texturas usando un parser manual!

![HeliosEngine cargando un modelo .obj](https://github.com/CsZBactery/HeliosEngine/blob/main/HeliosEngine/img/imgXbox.jpg?raw=true)

---

## Acerca del Proyecto

**HeliosEngine** es un motor de renderizado 3D en tiempo real construido desde cero utilizando **C++** y la API de **DirectX 11**.

El proyecto sirve como un compendio práctico de los conceptos fundamentales del pipeline gráfico, abarcando desde la inicialización de la ventana (Win32) y el dispositivo Direct3D, hasta la carga y renderizado de geometría compleja con shaders personalizados en HLSL.

## Características Principales

* **Pipeline de DirectX 11:** Configuración completa del pipeline de D3D11 (Device, DeviceContext, SwapChain, RenderTargetView, DepthStencilView).
* **Parser Manual de `.obj`:** Un cargador de modelos 3D personalizado y robusto. Es capaz de leer, parsear y triangular geometría (posiciones `v`, coordenadas de textura `vt`, y normales `vn`) desde archivos de formato `.obj`.
* **Renderizado de Malla:** Creación y enlace de Vertex Buffers e Index Buffers en la GPU a partir de la geometría cargada.
* **Shaders HLSL:** Shaders de Vértice (VS) y Píxel (PS) escritos en HLSL para manejar las transformaciones (matrices World, View, Projection) y el muestreo de texturas.
* **Carga de Texturas:** Soporte para cargar texturas de múltiples formatos (incluyendo `.dds`, `.png`, `.jpg`) para ser usadas en el Pixel Shader.
* **Cámara Interactiva:** Controles de cámara básicos (zoom) para inspeccionar el modelo cargado.

## ⚙️ Cómo Compilar y Ejecutar

### Prerrequisitos

* **Visual Studio 2022** (con la carga de trabajo "Desarrollo de escritorio con C++")
* **Windows 10/11 SDK** (generalmente incluido con la carga de trabajo de Visual Studio)

### Pasos

1.  Clona el repositorio:
    ```sh
    git clone [https://github.com/CsZBactery/HeliosEngine.git]
    ```
2.  Abre la solución `HeliosEngine.sln` con Visual Studio 2022.
3.  Asegúrate de que la configuración de la solución esté en `Debug` (o `Release`) y `x64`.
4.  Compila la solución (**Build** > **Build Solution** o `F7`).
5.  ¡Ejecuta el proyecto (`F5`)!

> **¡Importante!** El programa espera que los assets (modelos `.obj` y texturas) se encuentren en una carpeta `Assets` ubicada junto al archivo `.exe` generado (ej: `x64/Debug/Assets/Moto/repsol3.obj`).

## Controles

* **Rueda del Mouse (Scroll):** Acercar / Alejar la cámara.
* **Teclas `+` / `-`:** Acercar / Alejar la cámara.

## Estructura Clave del Proyecto

* `BaseApp`: Clase principal que orquesta la aplicación, maneja el bucle de mensajes de Win32 e inicializa todos los recursos de D3D11.
* `ModelLoader`: Contiene el **parser manual de `.obj`**, responsable de leer la geometría del archivo y poblar la estructura `MeshComponent`.
* `ShaderProgram`: Encapsula la compilación y administración de los shaders HLSL (VS/PS) y el Input Layout.
* `Buffer`: Clase wrapper para los buffers de la GPU (Vertex, Index y Constant Buffers).
* `Window`, `Device`, `SwapChain`: Clases que encapsulan los objetos COM de DirectX y la lógica de la ventana.