#define COMERA_PASS_REGISTER b1
#define LIGHT_PASS_REGISTER b0

#include "LightPass.hlsli"
#include "CameraPass.hlsli"

struct VertexIn{
    float2 Position : POSITION;
    float2 Texcoord : TEXCOORD0;
};

struct VertexOut{
    float4 ScreenPos : SV_POSITION;
    float2 Texcoord  : TEXCOORD0;
};

VertexOut VS(VertexIn vin){
    VertexOut vout;
    vout.ScreenPos = float4(vin.Position,0.,1.);
    vout.Texcoord  = vin.Texcoord;
    return vout;
}

/*
Texture2D GBuffer[3] : register(t0);
SamplerState sp : register(s0);


void UnpackGBuffer(float2 uv,out float3 worldPos,out float3 worldNor,out float3 worldDif,out float metallic,out float roughness){
    float4 pos = GBuffer[0].Sample(sp,uv);
    if(pos.w < 1e-4f) discard;
    worldPos = pos.xyz;
    worldNor = GBuffer[1].Sample(sp,uv).xyz;
    float4 gdata2 = GBuffer[2].Sample(sp,uv);
    worldDif = gdata2.xyz;
    worldSpecular = gdata2.w;
}*/

#include "GBufferUtil.hlsli"
#include "PBRLightingUtil.hlsli"

float CalcAttenuation(float d, float falloffStart, float falloffEnd)
{
    return 1. / (1. + d * d);//return saturate((falloffEnd-d) / (falloffEnd - falloffStart));
}

static const float3 F0 = float3(.04,.04,.04);

float4 PS(VertexOut vin) : SV_TARGET{
    float3 worldPos,normal,diffuse;
    float metallic,roughness;

    GBufferData data = UnpackGBuffer(vin.Texcoord);
    GBufferDiscard(data.worldPos);

    worldPos = data.worldPos.xyz,normal = data.worldNormal,
    diffuse = data.diffuse;
    metallic = data.metallic,roughness = data.roughness;

    float3 result = ambient.xyz * diffuse;
    float3 viewDir = normalize(cameraPos - worldPos);

    for(int i = 0;i != lightNum;i++){
        float3 lightDir,lightIntensity;
        
        if(lights[i].lightType == LIGHT_TYPE_DIRECTIONAL){
            lightDir = -lights[i].vec;
            lightIntensity = lights[i].intensity;
        }else if(lights[i].lightType == LIGHT_TYPE_POINT){
            lightDir = normalize(lights[i].vec - worldPos);
            lightIntensity = CalcAttenuation(length(lights[i].vec - worldPos),lights[i].fallStart,lights[i].fallEnd)
                * lights[i].intensity;
        }

        float3 Lo = BRDF(normal,lightDir,viewDir,diffuse,
            roughness,metallic,lightIntensity,F0);

        result += Lo;
    }

    return float4(result,1.);
}
