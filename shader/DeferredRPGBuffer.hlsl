#define OBJECT_PASS_REGISTER b0
#define CAMERA_PASS_REGISTER b1

#include "ObjectPass.hlsli"
#include "CameraPass.hlsli"

#ifdef USE_SKINNED_VERTEX

#define BONE_ARRAY_REGISTER t5
#include "SkinnedVertex.hlsli"

#else

struct VertexIn{
    float3 Position : POSITION;
    float3 Normal   : NORMAL;
    float2 Uv       : TEXCOORD0;
    float3 Tangent  : TANGENT;
};

#endif

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
Texture2D emissionMap  : register(t4);

SamplerState sp : register(s0);

#ifdef USE_SKINNED_VERTEX
    VertexOut VS(SkinnedVertexIn vin){
#else
    VertexOut VS(VertexIn vin){
#endif

    VertexOut vout;

#ifdef USE_SKINNED_VERTEX
    LocalVert vert = BlendLocalVertex(vin);
    vout.WorldPos = mul(world,float4(vert.localPos,0.));
    vout.Normal = mul((float3x3)transInvWorld,vert.localNormal);

    vout.Tangent = mul((float3x3)transInvWorld,vert.localTanget);
#else
    vout.WorldPos  = mul(world,float4(vin.Position,1.)).xyz;
    vout.Normal    = mul(transInvWorld,float4(vin.Normal,0.)).xyz;

    vout.Tangent   = mul((float3x3)transInvWorld,vin.Tangent);
#endif

    vout.ScreenPos = mul(perspect,mul(view,float4(vout.WorldPos,1.)));
    vout.Bitangent = cross(vout.Tangent,vout.Normal);
    vout.Uv        = mul((float3x3)mat.matTransform,float3(vin.Uv,1.)).xy;


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
    float3 emission    = emissionMap.Sample(sp,vin.Uv).rgb * mat.emission;

    GBufferData data;
    data.worldPos = float4(vin.WorldPos,1.);
    data.worldNormal = worldNormal;
    data.diffuse = diffuse;
    data.metallic = metallic;
    data.roughness = roughness; 
    data.emission = emission;

    return Pack2GBuffer(data);
}