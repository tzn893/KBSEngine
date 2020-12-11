struct VertexIn{
    float2 Position : POSITION;
    float2 TexCoord : TEXCOORD0;
};

cbuffer SpriteConstant : register(b0) {
    float4 color;
    float4x4 worldTrans;
};

cbuffer ViewConstant : register(b1){
    float4x4 viewTrans;  
};

struct VertexOut{
    float4 ScreenPos : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
};

Texture2D sprite : register(t0);
SamplerState defaultSampler : register(s0);

VertexOut VS(VertexIn vin){
    VertexOut vout;
    vout.ScreenPos = mul(viewTrans,mul(worldTrans,float4(vin.Position,1.,1.)));//float4(mul(viewTrans,mul(worldTrans,float3(vin.Position,1.f))),1.f);
    vout.TexCoord = vin.TexCoord;
    return vout;
}

float4 PS(VertexOut vin) : SV_TARGET{
    float4 result = sprite.Sample(defaultSampler,vin.TexCoord) * color;
	if (result.a < 1e-3) {
		discard;
	}
    return result;
}