# ☀️ HeliosEngine
*Compendio de Gráficas Computacionales 3D | Generación 2026-1*

<div align="center">
<img src="HeliosEngine/img/Ss2.jpg" alt="HeliosEngine Demo" width="800"/>
<br>
<sub><i>Renderizado en tiempo real de modelo de alta fidelidad con iluminación y texturizado básico.</i></sub>
</div>

---

## 📖 Acerca del Proyecto

HeliosEngine es un motor de renderizado 3D en tiempo real construido desde cero (*from scratch*) utilizando C++ y la API nativa de DirectX 11.

Este proyecto no utiliza motores comerciales (como Unity o Unreal); su propósito es desmitificar la "caja negra" del renderizado gráfico. Sirve como una implementación práctica y académica de los conceptos fundamentales del pipeline gráfico, abarcando:

* **Inicialización de bajo nivel:** Win32 API y Contextos de Dispositivo.
* **Matemáticas 3D y Shading:** Transformaciones espaciales, matrices de Mundo/Vista/Proyección y programación en HLSL.
* **Gestión de Memoria en GPU:** Buffers de Vértices, Índices y Constant Buffers en VRAM.
* **Arquitectura de Editor:** Bucle de juego estructurado, Grafo de Escena y Render-to-Texture.

---

## ✨ Características Principales

### 🛠️ Core & Pipeline
* **Pipeline DirectX 11 Completo:** Implementación robusta de Device, DeviceContext, SwapChain, RenderTargetView y DepthStencilView.
* **Separation of Concerns:** Arquitectura estricta de Game Loop que separa la lógica matemática (Update) de las llamadas a la API gráfica (Render).
* **Input Layout Dinámico:** Sistema LayoutBuilder para ensamblar vértices de forma segura, previniendo errores de Data Misalignment entre C++ y HLSL.
* **Anti-Aliasing (MSAA):** Configuración automática de calidad de muestreo (4x MSAA) adaptativa a la GPU para bordes suaves.

### 🎨 Gráficos & Renderizado
* **Soporte de Modelos Complejos (FBX/OBJ):** Carga de geometría avanzada incluyendo Posición, Normales, Tangentes, Bitangentes y Coordenadas UV.
* **Mapeo UV y Culling Controlado:** Corrección matemática en el Shader para adaptar sistemas de coordenadas, y control dinámico del Backface Culling para renderizado Two-Sided.
* **Skybox & Entornos Infinitos:** Renderizado de cielos mediante texturas Cubemap utilizando manipulaciones en el Z-Buffer para garantizar profundidad infinita.
* **Iluminación Base:** Implementación de luz direccional y cálculos de reflexión difusa (Lambert) en el Pixel Shader.
* **Soporte de Texturas:** Carga nativa de texturas (.png, .jpg, .dds) aplicadas al modelo mediante Shader Resource Views.

### 🕹️ Interactividad & Interfaz de Editor (UI)
* **Editor Viewport:** La escena 3D se renderiza en una textura independiente (Render-to-Texture) incrustada como un panel interactivo dentro de ImGui.
* **Outliner & Scene Graph:** Panel que muestra la jerarquía de todos los Actores presentes en el mundo.
* **Inspector de Propiedades:** Control en tiempo real de las Transformaciones (Posición, Rotación, Escala) de cada entidad.
* **Cámara Libre:** Sistema de cámara dinámica para navegar fluidamente por la escena.

---

## 📂 Arquitectura del Proyecto

Una visión general de las clases más importantes del motor:

| Clase | Responsabilidad |
| :--- | :--- |
| **BaseApp** | *El Director.* Orquesta la aplicación, maneja la ventana Win32, inicializa el hardware y ejecuta el bucle principal. |
| **Device / Context** | *La GPU Virtual.* Encapsulan la creación de recursos y el envío de comandos (Draw calls) a la tarjeta de video. |
| **SwapChain** | *El Presentador.* Gestiona el Double Buffering para mostrar imágenes sin parpadeos (tearing). |
| **SceneGraph** | *El Mundo.* Gestiona la actualización lógica y las llamadas de dibujado de todas las entidades activas. |
| **Actor** | *La Entidad.* Representa un objeto en el mundo, uniendo su Malla, su Transformación, sus Shaders y sus Texturas. |
| **ShaderProgram** | *El Programa.* Compila código HLSL (.fx) a Bytecode y gestiona el ciclo de vida del Vertex y Pixel Shader. |
| **LayoutBuilder** | *El Ensamblador.* Construye descriptores exactos de memoria para evitar desfases de datos entre la RAM y la VRAM. |
| **Skybox** | *El Entorno.* Gestiona el Cubemap y altera los estados de profundidad para dibujar fondos inalcanzables. |
| **EditorViewportPass** | *El Lienzo.* Crea un RenderTarget secundario para dibujar el juego dentro de una ventana de la interfaz gráfica. |

---

## ⚙️ Cómo Compilar y Ejecutar

**Prerrequisitos**
* Visual Studio 2022
* Carga de trabajo: "Desarrollo de escritorio con C++"
* Windows 10/11 SDK

**Pasos de Instalación**
1. **Clonar el repositorio:** `git clone https://github.com/CsZBactery/HeliosEngine.git`
2. **Abrir el proyecto:** Ejecuta `HeliosEngine.sln` con Visual Studio 2022.
3. **Configuración de Assets (Crucial):** El ejecutable busca la carpeta `Assets` en el directorio de trabajo. Asegúrate de que la carpeta esté en la raíz del proyecto (junto a los archivos `.cpp`), o configura en VS: *Propiedades > Depuración > Directorio de trabajo = $(ProjectDir)*.
4. **Compilar y Ejecutar:** Selecciona la Configuración `Debug`, Plataforma `x64` y presiona F5.

---

## 🎮 Controles

| Input / Acción | Descripción |
| :--- | :--- |
| **Paneles ImGui** | Usa el ratón para seleccionar objetos en el Outliner y modificar sus valores en el Inspector. |
| **Arrastrar Sliders** | Clic y arrastre sobre los valores numéricos de la interfaz para modificarlos dinámicamente. |
| **Botón [R]** | Restablece el valor asociado a su estado original (Posición a 0, Escala a 1, etc.). |
| **Slider Zoom** | Acerca o aleja la cámara del objeto seleccionado. |

<br>

<div align="center">
<sub>Desarrollado por <b>César Sasia (CsZBactery)</b> - Gráficas Computacionales 3D</sub>
</div>
