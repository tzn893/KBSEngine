#include "../LightPass.hlsli"
#include "../CameraPass.hlsli"

cbuffer FFTWaveObjectPass : register(b0){
    float4x4 world;
    float4x4 transInvWorld;
};

struct VertexIn{
    float3 Position : POSITION;
    float3 Normal   : NORMAL;
    float2 Uv       : TEXCOORD0;
};

struct VertexOut{
    float4 ScreenPos : SV_POSITION;
    float3 WorldPos  : POSITION;
    float3 WorldNor  : NORMAL;
};

Texture2D<float2> heightMap : register(t0);
SamplerState defaultSampler : register(s0);

VertexOut VS(VertexIn vin){
	VertexOut vout;

    float2 heightSample = heightMap.SampleLevel(defaultSampler,vin.Uv,0).xy;

    vout.WorldPos = mul(world,float4(vin.Position,1.) + float4(0.,heightSample.x,0.,0.)).xyz;
    vout.ScreenPos = mul(perspect,mul(view,float4(vout.WorldPos,1.)));
    vout.WorldNor = mul(transInvWorld,float4(vin.Normal,0.)).xyz;

    return vout;
}

float4 PS(VertexOut vin) : SV_TARGET{
    return float4(0.,0.,1.,1.) * dot(normalize(vin.WorldNor),float3(0.,1.,0.));
}