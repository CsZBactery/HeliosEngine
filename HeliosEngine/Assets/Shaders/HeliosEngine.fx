// ======================================================================================
// Archivo: HeliosEngine.fx
// Shader principal para renderizar modelos 3D con textura e iluminación básica.
// ======================================================================================

// 1. BUFFERS CONSTANTES
cbuffer CBNeverChanges : register(b0)
{
    float4x4 gView;
    // 💥 CORRECCIÓN: Ahora el shader sí recibe la luz que le manda C++
    float4 gLightDir;
    float4 gLightColor;
};

cbuffer CBChangeOnResize : register(b1)
{
    float4x4 gProj;
};

cbuffer CBChangesEveryFrame : register(b2)
{
    float4x4 gWorld;
    float4 gMeshColor;
};

// 2. RECURSOS
Texture2D gDiffuse : register(t0);
SamplerState gSamp : register(s0);

// 3. ESTRUCTURAS DE DATOS
struct VSIN
{
    float3 Pos : POSITION;
    float2 Tex : TEXCOORD0;
    // 💥 CORRECCIÓN: Agregamos las Normales para que coincida con el Layout de C++
    float3 Norm : NORMAL;
};

struct VSOUT
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD0;
    float3 Norm : NORMAL;
};

// ======================================================================================
// VERTEX SHADER
// ======================================================================================
VSOUT VS(VSIN i)
{
    VSOUT o;
    
    // Transformación de la posición del vértice a la pantalla
    float4 p = float4(i.Pos, 1.0f);
    o.Pos = mul(mul(mul(p, gWorld), gView), gProj);
    
    // Pasamos las coordenadas de textura intactas
    o.Tex = i.Tex;
    
    // Transformamos las normales usando solo la rotación del mundo para la iluminación
    o.Norm = mul(i.Norm, (float3x3) gWorld);
    
    return o;
}

// ======================================================================================
// PIXEL SHADER
// ======================================================================================
float4 PS(VSOUT i) : SV_Target
{
    // 1. Obtenemos el color base de la textura (el logo de Repsol)
    float4 texColor = gDiffuse.Sample(gSamp, i.Tex);
    
    // 2. Cálculos matemáticos de luz (Producto Punto)
    float3 normal = normalize(i.Norm);
    float3 lightDir = normalize(-gLightDir.xyz);
    
    // Intensidad de luz (max 0.2f asegura que haya una luz ambiental mínima y no se vea negro total)
    float lightIntensity = max(dot(normal, lightDir), 0.2f);
    
    // 3. Combinamos Textura * Color de Malla * Color de Luz * Intensidad
    float3 finalColor = texColor.rgb * gMeshColor.rgb * gLightColor.rgb * lightIntensity;
    
    return float4(finalColor, texColor.a);
}