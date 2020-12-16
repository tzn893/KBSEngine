
#include "LightPass.hlsli"
#include "ObjectPass.hlsli"
#include "CameraPass.hlsli"
#include "PhongModel.hlsli"

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
#ifdef ENABLE_DIFFUSE_MAP
    float3 Tangent   : TANGENT;
    float3 BiTangent : TEXCOORD1;
#endif
};

#ifdef ENABLE_DIFFUSE_MAP
Texture2D normalMap  : register(t1);
Texture2D diffuseMap : register(t0);
SamplerState defaultSampler : register(s0);
#endif //ENABLE_DIFFUSE_MAP


VertexOut VS(VertexIn vin,uint id : SV_VERTEXID){
    VertexOut vout;

    vout.WorldPos  = mul(world,float4(vin.Position,1.)).xyz;
	vout.ScreenPos = mul(perspect,mul(view,float4(vout.WorldPos,1.)));
    vout.WorldNorm = mul(transInvWorld,float4(vin.Normal,0.)).xyz;
    vout.Uv = vin.Uv;

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
        if(lights[i].lightType == LIGHT_TYPE_POINT){
            result += ComputePointLight(lights[i],pmat,toEye,worldPos,normal);
        }else if(lights[i].lightType == LIGHT_TYPE_DIRECTIONAL){
            result += ComputeDirectionalLight(lights[i],pmat,toEye,worldPos,normal);
        }
    }

    return float4(result,1.);
}