#define CAMERA_PASS_REGISTER b1
#define LIGHT_PASS_REGISTER b0

#include "../Util/LightPassUtil.hlsli"
#include "../Util/CameraPassUtil.hlsli"

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

#include "../Util/GBufferUtil.hlsli"
#include "../Util/PBRLightingUtil.hlsli"
#include "../Util/ToneMapUtil.hlsli"

TextureCube gIrradianceMap : register(t4);
SamplerState irrSp         : register(s1);

TextureCube gSpecularMap   : register(t5);
Texture2D   gLutMap        : register(t6);

#ifndef MAX_SPECULAR_LOD 
#define MAX_SPECULAR_LOD 4
#endif



static const float3 F0 = float3(.04,.04,.04);

float4 PS(VertexOut vin) : SV_TARGET{
    float3 worldPos,normal,diffuse,emission;
    float metallic,roughness;

    GBufferData data = UnpackGBuffer(vin.Texcoord);
    GBufferDiscard(data.worldPos);

    worldPos = data.worldPos.xyz,normal = data.worldNormal,
    diffuse = data.diffuse,emission = data.emission;
    metallic = data.metallic,roughness = data.roughness;

    float3 result = 0.;
    result += emission;

    //compute the result from environment
    float3 irradiance =  diffuse * gIrradianceMap.Sample(irrSp,-normal).rgb;
    float3 F1 = lerp(F0,diffuse,metallic);
    float3 kD = (1. - F1) * (1. - metallic);

    float3 viewDir = normalize(cameraPos - worldPos);
    float3 refViewDir = reflect(viewDir,normal);

    float3 F = FresnelSchlickRoughness(max(dot(normal,viewDir),0.),F1,roughness);

    float3 prefilteredColor = gSpecularMap.SampleLevel(irrSp,refViewDir,roughness * MAX_SPECULAR_LOD);
    float2 brdfLut = gLutMap.Sample(irrSp,float2(max(dot(normal,viewDir),0.),roughness));
    float3 ambientSpec = prefilteredColor * (F * brdfLut.x + brdfLut.y);

    float3 ambientBrdf = ambient.xyz * ITM_STANDERD(kD * irradiance + ambientSpec,exposure);

    result += ambientBrdf;
    //compute the result of direct light
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
