// ======================================================================================
// Archivo: Skybox.hlsl
// Shader especializado para renderizar un mapa de cubos (Entorno 3D).
// ======================================================================================

// 1. BUFFER CONSTANTE (Debe coincidir con tu struct CBSkybox en C++)
// Recibe la matriz de Vista-Proyeccion (sin traslacion) desde la CPU.
cbuffer CBSkybox : register(b0)
{
    matrix viewProj;
};

// 2. RECURSOS (Textura y Sampler)
// Registrados en el "Slot 10" tal como lo configuramos en Skybox::render()
TextureCube skyboxTexture : register(t10);
SamplerState samLinear : register(s10);

// 3. ESTRUCTURAS DE DATOS
// Lo que entra al Vertex Shader (Coincide con tu Input Layout: Solo Posicion)
struct VS_INPUT
{
    float3 Pos : POSITION;
};

// Lo que sale del Vertex Shader y entra al Pixel Shader
struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float3 TexCd : TEXCOORD; // Para un Cubemap necesitamos coordenadas 3D (X,Y,Z)
};

// ======================================================================================
// VERTEX SHADER
// ======================================================================================
PS_INPUT VS(VS_INPUT input)
{
    PS_INPUT output = (PS_INPUT) 0;

    // TRUCO #1: Las coordenadas de textura del cubo son exactamente la posicion local 
    // del vertice original, ya que el cubo esta centrado en (0,0,0).
    output.TexCd = input.Pos;

    // Multiplicamos la posicion por la matriz (recuerda que tu matriz ya transpuesta viene de C++)
    output.Pos = mul(float4(input.Pos, 1.0f), viewProj);

    // TRUCO MAGICO DEL SKYBOX #2:
    // Forzamos que el valor Z sea igual a W. 
    // Cuando la GPU haga su division de perspectiva (Z / W), el resultado siempre sera 1.0.
    // En DirectX, 1.0 es la profundidad mas lejana posible, haciendo que el cielo 
    // siempre se dibuje al fondo sin importar el tamano del cubo.
    output.Pos.z = output.Pos.w;

    return output;
}

// ======================================================================================
// PIXEL SHADER (Skybox)
// ======================================================================================
float4 PS(PS_INPUT input) : SV_Target
{
    // Muestreamos la textura de las 6 caras
    float4 color = skyboxTexture.Sample(samLinear, input.TexCd);
    
    // Forzamos el color a ser 100% brillante (Emisivo) para que no le afecten las sombras de la escena.
    return color;
}