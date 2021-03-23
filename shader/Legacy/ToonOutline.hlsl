//we should use back face to render this pass

struct VertexIn{
    float3 Position : POSITION;
    float3 Normal   : NORMAL;
    float2 Uv       : TEXCOORD0;
    float3 Tangent  : TANGENT;
};

struct VertexOut{
    float2 Uv     : TEXCOORD0;
    float4 ScreenPos : SV_POSITION;
};

#include "../Util/ObjectPassUtil.hlsli"
#include "../Util/CameraPassUtil.hlsli"

#include "../Util/NoisesUtil.hlsli"

cbuffer OutlineOption : register(b2){
    float3 outlineColor;
    float  heightWidthRatio;
    float  outlineWidth;
};

Texture2D diffuse : register(t0);
SamplerState sp   : register(s0);

VertexOut VS(VertexIn vin){
    VertexOut vout;
    //float3 outlinePos = vin.Position + vin.Normal * outlineWidth;
    float3 WorldPos = mul(world,float4(vin.Position,1.)).xyz;
    vout.ScreenPos = mul(perspect,mul(view,float4(WorldPos,1.)));

    float3 EyeNorm = mul((float3x3)transInvView,mul((float3x3)transInvWorld,vin.Normal));
    float2 extDir  = mul((float2x2)perspect,EyeNorm.xy).xy;
    extDir = normalize(extDir);
    extDir /= heightWidthRatio;

    //float noise = PerlinNoise21(vin.Uv);
    float oWidth = outlineWidth; //* noise * 2;
    
    vout.Uv = vin.Uv;
    vout.ScreenPos.xy += extDir * (vout.ScreenPos.w * oWidth);
    
    return vout;
}

float4 PS(VertexOut vin) : SV_TARGET{
    float3 albedo = diffuse.Sample(sp,vin.Uv).rgb;
    albedo = albedo * outlineColor;

    return float4(albedo,1.);
}