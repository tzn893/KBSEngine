
cbuffer ObjectPass : register(b0){
    float4x4 world;
    float4x4 transInvWorld;
};

cbuffer CameraPass : register(b1){
    float4x4 view;
    float4x4 perspect;
    float3   cameraPos;
}

#ifndef MAX_POINT_LIGHT_NUM
#define MAX_POINT_LIGHT_NUM 1
#endif


#ifndef DISABLE_LIGHT_PASS

#ifndef MAX_DIRECTIONAL_LIGHT_NUM
#define MAX_DIRECTIONAL_LIGHT_NUM 1
#endif //MAX_DIRECTIONAL_LIGHT_NUM 

struct Light{
    float3 Vec;//direction for directional light\position for point light
    float3 intensity;

    float fallOutStart;
    float fallOutEnd;
};

cbuffer LightPass : register(b2){
    int dirLightNum;
    Light dirLights[MAX_DIRECTIONAL_LIGHT_NUM];
    int pointLightNum;
    Light pointLights[MAX_POINT_LIGHT_NUM];
}

#endif //DISABLE_LIGHT_PASS

struct VertexIn{
    float3 Position : POSITION;
    float3 Normal   : NORMAL;
    float2 Uv       : TEXCOORD0;
};

struct VertexOut{
    float4 ScreenPos : SV_POSITION;
    float3 WorldPos  : POSITION;
    float3 WorldNor  : NORMAL;
    float2 Uv        : TEXCOORD0;
};

VertexOut VS(VertexIn vin){
    VertexOut vout;

    vout.WorldPos = mul(world,float4(vin.Position,1.)).xyz;
    vout.ScreenPos = mul(perspect,mul(view,float4(vout.WorldPos,1.)));
    vout.WorldNor = mul(transInvWorld,float4(vin.Normal,0.)).xyz;
    vout.Uv = vin.Uv;

    return vout;
}

float4 PS(VertexOut vin) : SV_TARGET {

    return float4(1.,1.,1.,1.);
}