#define CAMERA_PASS_REGISTER b1
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

#include "GBufferUtil.hlsli"
#include "PBRLightingUtil.hlsli"
#include "ToneMapUtil.hlsli"

TextureCube gIrradianceMap : register(t3);
SamplerState irrSp         : register(s1);

float CalcAttenuation(float d, float falloffStart, float falloffEnd)
{
    return 1. / (1. + d * d) * step(d,falloffEnd);//return saturate((falloffEnd-d) / (falloffEnd - falloffStart));
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

    float3 viewDir = normalize(cameraPos - worldPos);
    float3 refViewDir = reflect(viewDir,normal);
    float3 result = ambient.xyz * diffuse * ITM_STANDERD(gIrradianceMap.Sample(irrSp,refViewDir).rgb,exposure);

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
