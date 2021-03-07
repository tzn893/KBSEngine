#include "ToneMapUtil.hlsli"

struct VertexIn{
    float2 Position : POSITION;
    float2 TexCoord : TEXCOORD0;
};

struct SpriteConstant{
    float4 color;
    float4x4 worldTrans;
};

StructuredBuffer<SpriteConstant> spriteBuffer : register(t1);

cbuffer ViewConstant : register(b1){
    float4x4 viewTrans;
    float exposure;
};

struct VertexOut{
    float4 ScreenPos : SV_POSITION;
    float2 TexCoord  : TEXCOORD0;
    float4 Color     : TEXCOORD1;
};

Texture2D sprite : register(t0);
SamplerState defaultSampler : register(s0);

VertexOut VS(VertexIn vin,uint iid : SV_INSTANCEID){
    float4x4 worldTrans = spriteBuffer[iid].worldTrans;

    VertexOut vout;
    vout.ScreenPos = mul(viewTrans,mul(worldTrans,float4(vin.Position,1.,1.)));
    vout.TexCoord  = vin.TexCoord;
    vout.Color     = spriteBuffer[iid].color;

    return vout;
}

float4 PS(VertexOut vin) : SV_TARGET{
    float4 result = sprite.Sample(defaultSampler,vin.TexCoord) * vin.Color;
	if (result.a == 0.f) {
		discard;
	}
    return float4(ITM_STANDERD(result.rgb,exposure),result.a);
}