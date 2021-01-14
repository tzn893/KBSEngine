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

struct GBufferOut{
    float4 Buffer1 : SV_TARGET0;
    float4 Buffer2 : SV_TARGET1;
    float4 Buffer3 : SV_TARGET2;
};

GBufferOut Pack2GBuffer(float3 worldPos,float3 worldNormal,float3 diffuse,float specular){
    GBufferOut output;
    output.Buffer1 = float4(worldPos,1.f);
    output.Buffer2.xyz = worldNormal;
    output.Buffer3.xyz = diffuse;
    output.Buffer3.w   = specular;

    return output;
}

Texture2D normalMap : register(t0);
Texture2D diffuseMap : register(t1);
Texture2D specularMap : register(t2);

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
    float  specular    = specularMap.Sample(sp,vin.Uv).r;

    return Pack2GBuffer(vin.WorldPos,worldNormal,diffuse,specular);
}