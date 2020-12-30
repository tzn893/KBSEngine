#include "../LightPass.hlsli"
#include "../CameraPass.hlsli"

#include "../PhongModel.hlsli"

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
Texture2D<float2> normalMap : register(t1);
SamplerState defaultSampler : register(s0);

VertexOut VS(VertexIn vin){
	VertexOut vout;

    float heightSample = heightMap.SampleLevel(defaultSampler,vin.Uv,0).x;
    float2 normalxz = normalMap.SampleLevel(defaultSampler,vin.Uv,0).xy;
    float3 normal = float3(normalxz.x,sqrt(1. - dot(normalxz,normalxz)),normalxz.y);
    vout.WorldPos = mul(world,float4(vin.Position,1.) + float4(0.,heightSample,0.,0.)).xyz;
    vout.ScreenPos = mul(perspect,mul(view,float4(vout.WorldPos,1.)));
    //the Normal from input will always be (0.,1.,0.) we don't need to care
    vout.WorldNor = mul(transInvWorld,float4(normal,0.)).xyz;

    return vout;
}

float4 PS(VertexOut vin) : SV_TARGET{

    return float4(0.,0.,1.,1.) * dot(normalize(vin.WorldNor),normalize(float3(0.,1,0.)));
}