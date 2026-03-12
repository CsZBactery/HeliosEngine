// ======================================================================================
// Archivo: HeliosEngine.fx
// ======================================================================================

cbuffer CBNeverChanges : register(b0)
{
    float4x4 gView;
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

Texture2D gDiffuse : register(t0);
SamplerState gSamp : register(s0);

struct VSIN
{
    float3 Pos : POSITION;
    float2 Tex : TEXCOORD0; // El layout ahora nos mandara la textura real aqui
    float3 Norm : NORMAL;
    // (DirectX ignorara la Tangente y Bitangente automaticamente)
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
    
    float4 p = float4(i.Pos, 1.0f);
    o.Pos = mul(mul(mul(p, gWorld), gView), gProj);
    
    // ¡CORRECCIÓN! Devolvemos la textura a su estado original
    o.Tex = i.Tex;
    
    o.Norm = mul(i.Norm, (float3x3) gWorld);
    
    return o;
}

float4 PS(VSOUT i) : SV_Target
{
    float4 texColor = gDiffuse.Sample(gSamp, i.Tex);
    float3 normal = normalize(i.Norm);
    
    // Luz artificial
    float3 lightDir = normalize(float3(0.5f, -1.0f, 0.8f));
    float3 lightColor = float3(1.0f, 1.0f, 1.0f);
    float lightIntensity = max(dot(normal, lightDir), 0.4f);
    
    // Forzamos blanco puro para no oscurecer la consola
    float3 finalColor = texColor.rgb * float3(1.0f, 1.0f, 1.0f) * lightColor * lightIntensity;
    
    return float4(finalColor, texColor.a);
}