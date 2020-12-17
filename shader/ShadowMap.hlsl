#include "ObjectPass.hlsli"

struct VertexIn{
    float3 Position : POSITION;
    float3 Normal   : NORMAL;
    float2 Uv       : TEXCOORD0;
    float3 ignore   : TANGENT;
};

cbuffer ShadowLightPass : register(b1) { 
    float4x4 lightMat;
};

struct VertexOut{
    float4 ViewPos  : SV_POSITION;
};

VertexOut VS(VertexIn vin){
    VertexOut vout;
    vout.ViewPos = mul(lightMat,mul(world,float4(vin.Position,1.f)));
    
    return vout;
}

float4 PS(VertexOut vout) :SV_TARGET{
    float3 result = vout.ViewPos.z;
    return float4(result,1.);
}