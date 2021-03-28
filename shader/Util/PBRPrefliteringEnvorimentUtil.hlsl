#include "PBRLightingUtil.hlsli"

#define SAMPLE_NUM 65536
/*
cbuffer Proj : register(b0){
    float4x4 Mright;
    float4x4 Mleft ;
    float4x4 Mup   ;
    float4x4 Mdown ;
    float4x4 Mfront;
    float4x4 Mback ;
};

cbuffer State : register(b1){
    float roughness;
};

TextureCube srcTex : register(t0);
SamplerState sp  : register(s0);

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
*/

cbuffer State : register(b1){
    float roughness;
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

cbuffer Proj : register(b0){
    float4x4 proj;
};

TextureCube srcTex : register(t0);
SamplerState sp  : register(s0);

struct Vout{
    float3 Pos : TEXCOORD0;
    float4 NDC : SV_POSITION;
};

Vout VS(uint vid : SV_VERTEXID){ 
	Vout v;
    v.NDC = quads[vid];
    v.Pos = mul(proj,quads[vid]).xyz;
    return v;
}


float4 SampleImportanceGGX(float3 normal){
    float3 result = 0.;
    float  weight = 0.;
    
    //we assume that view direction equals normal direction
    float3  view = normal;

    for(uint i = 0;i != SAMPLE_NUM;i++){
        float3 dir = ImportanceGGX(i,SAMPLE_NUM,normal,roughness);
        dir = reflect(-dir,view);
        float w = max(0.,dot(dir,normal));

        result += srcTex.Sample(sp,dir) * w;
        weight += w;
    }
    result /= max(weight,1e-4);

    return float4(result,1.);
}

float4 PS(Vout v) : SV_TARGET{
    return SampleImportanceGGX(normalize(v.Pos));
}

/*Pout PS(Vout v) : SV_TARGET{
    Pout p;
    p.pright = SampleImportanceGGX(normalize(v.Pright));
    p.pleft  = SampleImportanceGGX(normalize(v.Pleft ));
    p.pdown  = SampleImportanceGGX(normalize(v.Pdown));
    p.pup    = SampleImportanceGGX(normalize(v.Pup));
    p.pfront = SampleImportanceGGX(normalize(v.Pfront));
    p.pback  = SampleImportanceGGX(normalize(v.Pback));
    return p;
}*/
