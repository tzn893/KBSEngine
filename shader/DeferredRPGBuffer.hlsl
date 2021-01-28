#define OBJECT_PASS_REGISTER b0
#define CAMERA_PASS_REGISTER b1

#include "ObjectPass.hlsli"
#include "CameraPass.hlsli"

struct VertexIn{
    float3 Position : POSITION;
    float3 Normal   : NORMAL;
    float2 Uv       : TEXCOORD0;
    float3 Tangent  : TANGENT;
};

struct VertexOut{
    float4 ScreenPos : SV_POSITION;
    float3 WorldPos  : POSITION;
    float3 Normal    : NORMAL;
    float2 Uv        : TEXCOORD0;
    float3 Tangent   : TANGENT;
    float3 Bitangent : TEXCOORD1;
};

#define  GBUFFER_OUTPUT
#include "GBufferUtil.hlsli"

Texture2D normalMap : register(t0);
Texture2D diffuseMap : register(t1);
Texture2D roughnessMap : register(t2);
Texture2D metallicMap  : register(t3);

SamplerState sp : register(s0);

VertexOut VS(VertexIn vin){
    VertexOut vout;
    vout.WorldPos  = mul(world,float4(vin.Position,1.)).xyz;
    vout.ScreenPos = mul(perspect,mul(view,float4(vout.WorldPos,1.)));
    vout.Normal    = mul(transInvWorld,float4(vin.Normal,0.)).xyz;

    vout.Uv        = mul((float3x3)mat.matTransform,float3(vin.Uv,1.)).xy;

    vout.Tangent   = mul((float3x3)transInvWorld,vin.Tangent);
    vout.Bitangent = cross(vout.Tangent,vout.Normal);

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

GBufferOut PS(VertexOut vin){
    float3 tNormal    = normalize(vin.Normal  );
    float3 tTangent   = normalize(vin.Tangent );
    float3 tBitangent = normalize(vin.Bitangent);

    float3 worldNormal = SampleNormalMap(normalMap,sp,vin.Uv,tNormal,tTangent,tBitangent);
    float3 diffuse     = diffuseMap.Sample(sp,vin.Uv).rgb * mat.diffuse.xyz;
    //float  specular    = specularMap.Sample(sp,vin.Uv).r;
    float  metallic    = metallicMap.Sample(sp,vin.Uv).r * mat.Metallic;
    float  roughness   = roughnessMap.Sample(sp,vin.Uv).r * mat.Roughness;

    GBufferData data;
    data.worldPos = float4(vin.WorldPos,1.);
    data.worldNormal = worldNormal;
    data.diffuse = diffuse;
    data.metallic = metallic;
    data.roughness = roughness; 

    return Pack2GBuffer(data);
}