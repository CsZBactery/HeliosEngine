// ==========================================================
// ShadowMap.hlsl
// Genera el mapa de profundidad desde la perspectiva de la luz
// ==========================================================

cbuffer CBPerFrame : register(b0)
{
    matrix View;
    matrix Projection;
    matrix LightViewProjection;
    float3 CameraPos;
    float pad0;
    float3 LightDir;
    float pad1;
    float3 LightColor;
    float pad2;
};

cbuffer CBPerObject : register(b1)
{
    matrix World;
};

struct VS_INPUT
{
    float4 Pos : POSITION;
    float3 Norm : NORMAL;
    float3 Tan : TANGENT;
    float3 Bitan : BITANGENT;
    float2 Tex : TEXCOORD0;
};

struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
};

// ==========================================================
// VERTEX SHADER
// ==========================================================
VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output;
    
    // 1. Posición del vértice en el mundo
    float4 worldPos = mul(input.Pos, World);
    
    // 2. Posición vista desde la LUZ (No desde la cámara)
    output.Pos = mul(worldPos, LightViewProjection);
    
    return output;
}

// ==========================================================
// PIXEL SHADER
// ==========================================================
// Para mapas de sombras direccionales solo nos importa la profundidad (Z-Buffer).
// No necesitamos calcular colores, la GPU escribe la profundidad automáticamente.
float4 PS(VS_OUTPUT input) : SV_Target
{
    return float4(0.0f, 0.0f, 0.0f, 0.0f);
}