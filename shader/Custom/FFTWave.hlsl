#include "../LightPass.hlsli"
#include "../CameraPass.hlsli"

#include "../PhongModel.hlsli"

cbuffer FFTWaveObjectPass : register(b0){
    float4x4 world;
    float4x4 transInvWorld;
    float4   oceanColorSwallow;
	float4   oceanColorDeep;
};

struct VertexIn{
    float3 Position : POSITION;
    float3 Normal   : NORMAL;
    float2 Uv       : TEXCOORD0;
};

struct VertexOut{
    float4 ScreenPos : SV_POSITION;
    float3 WorldPos  : POSITION;
    float3 WorldNor  : NORMAL;
	float2 Uv		 : TEXCOORD0;
};

Texture2D<float2> heightMap : register(t0);
Texture2D<float2> normalMap : register(t1);
SamplerState defaultSampler : register(s0);

VertexOut VS(VertexIn vin){
	VertexOut vout;

    float heightSample = heightMap.SampleLevel(defaultSampler,vin.Uv,0).x;
   
    vout.WorldPos = mul(world,float4(vin.Position,1.) + float4(0.,heightSample,0.,0.)).xyz;
    vout.ScreenPos = mul(perspect,mul(view,float4(vout.WorldPos,1.)));
    //the Normal from input will always be (0.,1.,0.) we don't need to care
    vout.WorldNor = mul(transInvWorld,float4(vin.Normal,0.)).xyz;
	vout.Uv       = vin.Uv;

    return vout;
}

float4 PS(VertexOut vin) : SV_TARGET{

    /*
    vin.Uv = frac(vin.Uv);
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

struct Material{
    float4 diffuse;
    float3 FresnelR0;
    float  Roughness;
    float4x4 matTransform;
};
    */
    float2 normalxz = normalMap.SampleLevel(defaultSampler,vin.Uv,0).xy;
    float3 normal = float3(normalxz.x,sqrt(1. - dot(normalxz,normalxz)),normalxz.y);
    float3 worldPos = vin.WorldPos;

    float3 viewDir = normalize(cameraPos - worldPos);
    float3 lightDir = normalize(-lights[MAIN_LIGHT_INDEX].vec);

    float3 halfDir = normalize(viewDir + lightDir);

	float facing = saturate(dot(normal,viewDir));

	float3 diffuse = lerp(oceanColorSwallow,oceanColorDeep,facing).xyz * lights[MAIN_LIGHT_INDEX].intensity * saturate(dot(normal, lightDir) + ambient);
	float3 specular = lights[MAIN_LIGHT_INDEX].intensity * pow(max(0, dot(normal, halfDir)), 50);

	float3 result =  diffuse;

    return float4(result ,1.);
}