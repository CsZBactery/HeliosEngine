// ==========================================================
// DeferredLighting.hlsl
// Lee el G-Buffer y calcula la luz direccional y sombras
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

// G-Buffer Inputs
Texture2D txAlbedoMetallic : register(t0);
Texture2D txNormalRoughness : register(t1);
Texture2D txWorldAo : register(t2);
Texture2D txEmissiveAlpha : register(t3);
Texture2D txShadowMap : register(t6);

SamplerState samPoint : register(s0);

struct VS_INPUT
{
    float4 Pos : POSITION;
    float2 Tex : TEXCOORD0;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD0;
};

// Dibuja un rectángulo que cubre toda la pantalla
PS_INPUT VS(VS_INPUT input)
{
    PS_INPUT output;
    output.Pos = input.Pos;
    output.Tex = input.Tex;
    return output;
}

float4 PS(PS_INPUT input) : SV_Target
{
    // 1. Leer los datos del G-Buffer
    float4 albedoMetal = txAlbedoMetallic.Sample(samPoint, input.Tex);
    float4 normRough = txNormalRoughness.Sample(samPoint, input.Tex);
    float4 worldAo = txWorldAo.Sample(samPoint, input.Tex);
    
    float3 albedo = albedoMetal.rgb;
    float3 normal = normRough.rgb;
    float3 worldPos = worldAo.xyz;
    
    // Si no hay geometría aquí (fondo), pinta oscuro
    if (length(normal) < 0.1f)
        return float4(0.1f, 0.1f, 0.1f, 1.0f);
    
    // 2. Luz Básica (Lambert)
    float3 lightDir = normalize(-LightDir);
    float NdotL = max(dot(normal, lightDir), 0.0f);
    float3 diffuse = albedo * LightColor * NdotL;
    
    // Ambient básico usando el AO
    float3 ambient = albedo * 0.1f * worldAo.a;

    // 3. Resultado Final
    float3 finalColor = diffuse + ambient;
    
    return float4(finalColor, 1.0f);
}