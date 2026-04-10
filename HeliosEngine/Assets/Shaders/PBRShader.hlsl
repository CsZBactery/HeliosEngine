// ======================================================================================
// PBRShader.hlsl
// Shader de Renderizado Basado en Física (Physically Based Rendering)
// Soporta Albedo, Normal, Metallic, Roughness y Ambient Occlusion.
// ======================================================================================

static const float PI = 3.14159265359;

// ==========================================
// CONSTANT BUFFERS
// ==========================================

cbuffer CBMain : register(b0)
{
    matrix View;
    matrix Projection;
    float3 CameraPos;
    float padding1;
    float3 LightDir;
    float padding2;
    float3 LightColor;
    float padding3;
};

cbuffer CBObject : register(b1)
{
    matrix World;
};

// ==========================================
// TEXTURAS Y SAMPLER 
// ==========================================
Texture2D albedoMap : register(t0);
Texture2D normalMap : register(t1);
Texture2D metallicMap : register(t2);
Texture2D roughnessMap : register(t3);
Texture2D aoMap : register(t4);

SamplerState objSamplerState : register(s0);

// ==========================================
// ESTRUCTURAS DE DATOS
// ==========================================
struct VS_INPUT
{
    float3 Pos : POSITION;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float3 Bitangent : BITANGENT;
    float2 TexCoord : TEXCOORD;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float3 WorldPos : POSITION;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float3 Bitangent : BITANGENT;
    float2 TexCoord : TEXCOORD;
};

// ==========================================
// VERTEX SHADER (Corregido a "VS")
// ==========================================
PS_INPUT VS(VS_INPUT input)
{
    PS_INPUT output;
    
    float4 worldPos = mul(float4(input.Pos, 1.0f), World);
    output.WorldPos = worldPos.xyz;
    
    output.Pos = mul(worldPos, View);
    output.Pos = mul(output.Pos, Projection);

    output.Normal = normalize(mul(input.Normal, (float3x3) World));
    output.Tangent = normalize(mul(input.Tangent, (float3x3) World));
    output.Bitangent = normalize(mul(input.Bitangent, (float3x3) World));
    
    output.TexCoord = input.TexCoord;

    return output;
}

// ==========================================
// FUNCIONES MATEMÁTICAS PBR 
// ==========================================
float DistributionGGX(float3 N, float3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / max(denom, 0.0000001);
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

float3 fresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// ==========================================
// PIXEL SHADER (Corregido a "PS")
// ==========================================
float4 PS(PS_INPUT input) : SV_TARGET
{
    float3 albedo = pow(albedoMap.Sample(objSamplerState, input.TexCoord).rgb, 2.2);
    float metallic = metallicMap.Sample(objSamplerState, input.TexCoord).r;
    float roughness = roughnessMap.Sample(objSamplerState, input.TexCoord).r;
    float ao = aoMap.Sample(objSamplerState, input.TexCoord).r;

    float3 tangentNormal = normalMap.Sample(objSamplerState, input.TexCoord).xyz * 2.0 - 1.0;
    float3x3 TBN = float3x3(normalize(input.Tangent), normalize(input.Bitangent), normalize(input.Normal));
    float3 N = normalize(mul(tangentNormal, TBN));

    float3 V = normalize(CameraPos - input.WorldPos);
    float3 L = normalize(-LightDir);
    float3 H = normalize(V + L);

    float3 F0 = float3(0.04, 0.04, 0.04);
    F0 = lerp(F0, albedo, metallic);

    float3 Lo = float3(0.0, 0.0, 0.0);
    float3 radiance = LightColor;

    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    float3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);
    
    float3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    float3 specular = numerator / denominator;
    
    float3 kS = F;
    float3 kD = float3(1.0, 1.0, 1.0) - kS;
    kD *= 1.0 - metallic;

    float NdotL = max(dot(N, L), 0.0);
    Lo += (kD * albedo / PI + specular) * radiance * NdotL;

    float3 ambient = float3(0.03, 0.03, 0.03) * albedo * ao;
    float3 color = ambient + Lo;

    color = color / (color + float3(1.0, 1.0, 1.0));
    color = pow(color, float3(1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2));

    return float4(color, 1.0);
}