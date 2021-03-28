Texture2D tex : register(t0);
SamplerState sp : register(s0);

struct Vout{
    float3 Pright : TEXCOORD0;
    float3 Pleft  : TEXCOORD1;
    float3 Pup    : TEXCOORD2;
    float3 Pdown  : TEXCOORD3;
    float3 Pfront : TEXCOORD4;
    float3 Pback  : TEXCOORD5;
    float4 NDC    : SV_POSITION;
};

struct Pout{
    float4 pright : SV_TARGET0;
    float4 pleft  : SV_TARGET1;
    float4 pup    : SV_TARGET2;
    float4 pdown  : SV_TARGET3;
    float4 pfront : SV_TARGET4;
    float4 pback  : SV_TARGET5;
};

cbuffer Proj : register(b0){
    float4x4 Mright;
    float4x4 Mleft ;
    float4x4 Mup   ;
    float4x4 Mdown ;
    float4x4 Mfront;
    float4x4 Mback ;
};

static const float4 quads[6] =
{
    float4(-1.0f, -1.0f, 0.f,  1.f),
    float4( 1.0f,  1.0f, 0.f,  1.f),
    float4( 1.0f, -1.0f, 0.f,  1.f),
    float4(-1.0f,  1.0f, 0.f,  1.f),
    float4(-1.0f, -1.0f, 0.f,  1.f),
    float4( 1.0f,  1.0f, 0.f,  1.f)
};

Vout VS(uint vid : SV_VERTEXID){
    Vout v;
    v.NDC = quads[vid];
    v.Pright = mul(Mright,quads[vid]).xyz;
    v.Pleft  = mul(Mleft ,quads[vid]).xyz;
    v.Pdown  = mul(Mdown ,quads[vid]).xyz;
    v.Pup    = mul(Mup   ,quads[vid]).xyz;
    v.Pfront = mul(Mfront,quads[vid]).xyz;
    v.Pback  = mul(Mback ,quads[vid]).xyz;
    return v;
}

#ifndef PI
#define PI 3.1415926
#endif

static const float2 invPi = float2(1. / (PI * 2) , 1. / PI);

float4 Sample(float3 dir){
    float2 uv = float2(atan2(dir.z,dir.x),asin(dir.y)) * invPi;
    uv += .5;
    uv.y = 1. - uv.y;
    return float4(tex.SampleLevel(sp,uv,0).rgb,1.);
}


Pout PS(Vout vin){
    Pout pout;
    pout.pright = Sample(normalize(vin.Pright));
    pout.pleft  = Sample(normalize(vin.Pleft));
    pout.pup    = Sample(normalize(vin.Pup));
    pout.pdown  = Sample(normalize(vin.Pdown));
    pout.pfront = Sample(normalize(vin.Pfront));
    pout.pback  = Sample(normalize(vin.Pback));
    return pout;
}