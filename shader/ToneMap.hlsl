Texture2D renderTarget : register(t0);
SamplerState sp : register(s0);


cbuffer Const : register(b0){
    float exposure;
};

static const float2 Square[] = {
    float2(0.0f, 1.0f),
    float2(1.0f, 1.0f),
    float2(1.0f, 0.0f),
    float2(0.0f, 0.0f),
    float2(0.0f, 1.0f),
    float2(1.0f, 0.0f)
};

#include "ToneMapUtil.hlsli"

struct VertexOut{
    float4 ScreenPos : SV_POSITION;
    float2 Uv        : TEXCOORD;
};

VertexOut VS(uint vid : SV_VERTEXID){
    VertexOut vout;
    vout.Uv  = Square[vid];
    vout.ScreenPos = float4(vout.Uv.x * 2. - 1.,1. - vout.Uv.y * 2.,0.,1.);
    return vout;
}

float4 PS(VertexOut vout):SV_TARGET{
    //tone mapping and gamma space
    return float4(TM_STANDERD(renderTarget.Sample(sp,vout.Uv).rgb,exposure),1.f);
}