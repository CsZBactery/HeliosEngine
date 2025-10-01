cbuffer CBNeverChanges : register(b0)
{
    float4x4 gView;
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
    float2 Tex : TEXCOORD0;
};
struct VSOUT
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD0;
};

VSOUT VS(VSIN i)
{
    VSOUT o;
    float4 p = float4(i.Pos, 1.0);
    o.Pos = mul(mul(mul(p, gWorld), gView), gProj);
    o.Tex = i.Tex;
    return o;
}

float4 PS(VSOUT i) : SV_Target
{
    float4 tex = gDiffuse.Sample(gSamp, i.Tex);
    return tex * float4(gMeshColor.rgb, 1.0);
}
