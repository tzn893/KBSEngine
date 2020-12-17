
#include "LightPass.hlsli"
#include "ObjectPass.hlsli"
#include "CameraPass.hlsli"
#include "PhongModel.hlsli"


#ifndef SHADOW_MAP_TEX_WIDTH
#define SHADOW_MAP_TEX_WIDTH 1e-3
#endif

#ifndef SHADOW_MAP_TEX_HEIGHT
#define SHADOW_MAP_TEX_HEIGHT 1e-3
#endif

struct VertexIn{
    float3 Position : POSITION;
    float3 Normal   : NORMAL;
    float2 Uv       : TEXCOORD0;
    float3 Tangent  : TANGENT;
};

struct VertexOut{
    float4 ScreenPos : SV_POSITION;
    float3 WorldNorm : NORMAL;
    float2 Uv        : TEXCOORD0;
    float3 WorldPos  : POSITION;
    float4 lightPos  : TEXCOORD2;
#ifdef ENABLE_DIFFUSE_MAP
    float3 Tangent   : TANGENT;
    float3 BiTangent : TEXCOORD1;
#endif
};

cbuffer ShadowPass : register(b3){
    float4x4 shadowMat;
};
Texture2D shadowMap  : register(t2);
SamplerState defaultSampler : register(s0);


#ifdef ENABLE_DIFFUSE_MAP
    Texture2D normalMap  : register(t1);
    Texture2D diffuseMap : register(t0);
#endif //ENABLE_DIFFUSE_MAP


VertexOut VS(VertexIn vin){
    VertexOut vout;

    vout.WorldPos  = mul(world,float4(vin.Position,1.)).xyz;
	vout.ScreenPos = mul(perspect,mul(view,float4(vout.WorldPos,1.)));
    vout.WorldNorm = mul(transInvWorld,float4(vin.Normal,0.)).xyz;
    vout.Uv = vin.Uv;

    vout.lightPos = mul(shadowMat,float4(vout.WorldPos,1.));

#ifdef ENABLE_DIFFUSE_MAP
    vout.Tangent = mul(transInvWorld,float4(vin.Tangent,0.)).xyz;
    vout.BiTangent = cross(vout.Tangent,vout.WorldNorm);
#endif
    return vout; 
}

float3 SampleNormalMap(Texture2D normalMap,SamplerState samp,float2 uv,float3 normal,float3 tangent,float3 bitan){
    float3 localNorm = normalMap.Sample(samp,uv).rgb;
    //unpack data
    localNorm = normalize(localNorm * 2. - 1.);
    return normalize(
        localNorm.x * tangent + localNorm.y * bitan + localNorm.z * normal
    );
}

float SampleShadowMap(Texture2D shadowMap,SamplerState samp,float4 lightPos){
    const float shadowBias = 3e-3;

    float3 NDCLightPos = lightPos.xyz / lightPos.w;
    float2 shadowUv = NDCLightPos.xy;

    shadowUv.x = shadowUv.x * .5 + .5;
    //It seems that the sampling direction of y axis should be opposite compared to depth generating
    shadowUv.y = shadowUv.y * (-.5) + .5;
    float currentDepth = NDCLightPos.z;
    
    float shadowFactor = 0.;
    //Do PCF
    for(int x = -1;x <= 1;x++){
        for(int y = -1;y <= 1;y++){
            float2 pcfUv = float2(x,y) * float2(SHADOW_MAP_TEX_WIDTH,SHADOW_MAP_TEX_HEIGHT) + shadowUv;
            float shadowDepth = shadowMap.Sample(samp,pcfUv).r + shadowBias;
            shadowFactor += shadowDepth < currentDepth ? 0. : 1.;
        }
    }

    return shadowFactor / 9.;
}

float4 PS(VertexOut vin) :SV_TARGET{
    float3 normal = normalize(vin.WorldNorm);
#ifdef ENABLE_DIFFUSE_MAP
    float3 tangent = normalize(vin.Tangent);
    float3 bitangent = normalize(vin.BiTangent);
    normal = SampleNormalMap(normalMap,defaultSampler,vin.Uv,normal,tangent,bitangent);
#endif // ENABLE_DIFFUSE_MAP

    float3 toEye  = normalize(cameraPos - vin.WorldPos);
    float3 worldPos = vin.WorldPos;
    
    Material pmat = mat;
#ifdef ENABLE_DIFFUSE_MAP
    pmat.diffuse = diffuseMap.Sample(defaultSampler,vin.Uv) * mat.diffuse;
	float3 result = ambient.xyz * pmat.diffuse.xyz;
#else 
    float3 result = ambient.xyz;
#endif//ENABLE_DIFFUSE_MAP
    
    for(int i = 0;i != MAX_LIGHT_STRUCT_NUM;i++){
        float3 shadowFactor = 1.;
        if(i == MAIN_LIGHT_INDEX){
            shadowFactor = SampleShadowMap(shadowMap,defaultSampler,vin.lightPos);
        }
        float3 lightShading;
        if(lights[i].lightType == LIGHT_TYPE_POINT){
            lightShading = ComputePointLight(lights[i],pmat,toEye,worldPos,normal);
        }else if(lights[i].lightType == LIGHT_TYPE_DIRECTIONAL){
            lightShading = ComputeDirectionalLight(lights[i],pmat,toEye,worldPos,normal);
        }
        result += lightShading * shadowFactor;
    }

    return float4(result,1.);
}