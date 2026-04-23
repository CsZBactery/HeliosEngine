// ==========================================================
// DeferredGBuffer.hlsl
// Escribe Posición, Normales, Albedo, y PBR en el G-Buffer
// ==========================================================

// --- AHORA SÍ LEEMOS LA CÁMARA ---
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

cbuffer CBPerMaterial : register(b2)
{
    float4 BaseColor;
    float Metallic;
    float Roughness;
    float AO;
    float NormalScale;
    float EmissiveStrength;
    float AlphaCutoff;
};

Texture2D txAlbedo : register(t0);
Texture2D txNormal : register(t1);
Texture2D txMetallic : register(t2);
Texture2D txRoughness : register(t3);
Texture2D txAO : register(t4);
SamplerState samLinear : register(s0);

struct VS_INPUT
{
    float4 Pos : POSITION;
    float3 Norm : NORMAL;
    float3 Tan : TANGENT;
    float3 Bitan : BITANGENT;
    float2 Tex : TEXCOORD;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float4 WorldPos : POSITION;
    float3 Norm : NORMAL;
    float2 Tex : TEXCOORD;
};

PS_INPUT VS(VS_INPUT input)
{
    PS_INPUT output;
    
    // 1. Calculamos dónde está el objeto en el mundo 3D
    output.WorldPos = mul(input.Pos, World);
    
    // 2. APLICAMOS LA CÁMARA (¡Esto es lo que faltaba!)
    // Proyectamos el 3D hacia la pantalla 2D usando la vista de la cámara
    matrix viewProj = mul(View, Projection);
    output.Pos = mul(output.WorldPos, viewProj);
    
    // 3. Rotamos las normales y pasamos texturas
    output.Norm = normalize(mul(input.Norm, (float3x3) World));
    output.Tex = input.Tex;
    
    return output;
}

struct PS_OUTPUT
{
    float4 AlbedoMetallic : SV_Target0;
    float4 NormalRoughness : SV_Target1;
    float4 WorldAo : SV_Target2;
    float4 EmissiveAlpha : SV_Target3;
};

PS_OUTPUT PS(PS_INPUT input)
{
    PS_OUTPUT output;
    
    float4 texColor = txAlbedo.Sample(samLinear, input.Tex) * BaseColor;
    float metallic = txMetallic.Sample(samLinear, input.Tex).r * Metallic;
    float roughness = txRoughness.Sample(samLinear, input.Tex).r * Roughness;
    float ao = txAO.Sample(samLinear, input.Tex).r * AO;
    
    if (texColor.a < AlphaCutoff)
        discard;

    // Llenar las pantallas del G-Buffer
    output.AlbedoMetallic = float4(texColor.rgb, metallic);
    output.NormalRoughness = float4(normalize(input.Norm), roughness);
    output.WorldAo = float4(input.WorldPos.xyz, ao);
    output.EmissiveAlpha = float4(0.0f, 0.0f, 0.0f, texColor.a);
    
    return output;
}