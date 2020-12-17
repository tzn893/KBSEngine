Texture2D tex : register(t0);
SamplerState defaultSampler : register(s0);

struct VIN{
    float2 screenPos : POSITION;
    float2 Uv        : TEXCOORD0;
};

struct VOUT{
    float4 screenPos : SV_POSITION;
    float2 Uv        : TEXCOORD0;
};

cbuffer ViewPort : register(b0){
    float2 downLeft;
    float2 upRight;
};

VOUT VS(VIN vin){
    VOUT vout;

    float2 center = downLeft + upRight - 1.;
    float2 diff   = upRight - downLeft;

    vout.screenPos = float4(vin.screenPos * diff + center,1.,1.);
    vout.Uv        = vin.Uv;
    return vout;
}

float4 PS(VOUT vin) : SV_TARGET{
    float3 result = tex.Sample(defaultSampler,vin.Uv).xyz;
    return float4(result,1.);
}