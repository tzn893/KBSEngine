struct VertexIn{
    float3 Position : POSITION;
    float3 Normal   : NORMAL;
    float2 Uv       : TEXCOORD0;
    float3 Tangent  : TANGENT;
};

struct VertexOut{
    float4 ScreenPos : SV_POSITION;
    float3 WorldNorm : NORMAL;
    float3 WorldPos  : POSITION;
    float2 Uv        : TEXCOORD0;
};

#include "ObjectPass.hlsli"
#include "CameraPass.hlsli"
#include "LightPass.hlsli"

#include "ToneMapUtil.hlsli"

cbuffer ToonShadingSetting : register(b3){
    float divL;
    float divM;
    float divH;
};


Texture2D gDiffuse : register(t0);
SamplerState sp    : register(s0);


VertexOut VS(VertexIn vin){
    VertexOut vout;
    vout.WorldPos  = mul(world,float4(vin.Position,1.)).xyz;
    vout.ScreenPos = mul(perspect,mul(view,float4(vout.WorldPos,1.)));
    vout.WorldNorm = mul(transInvWorld,float4(vin.Normal,0.)).xyz;

    vout.Uv        = mul((float3x3)mat.matTransform,float3(vin.Uv,1.)).xy;

	return vout;
}

float4 PS(VertexOut vout) : SV_TARGET{
    
    float3 albedo = gDiffuse.Sample(sp,vout.Uv).rgb;
    float3 color = albedo * ambient.rgb;
    
    for(uint i = 0;i != MAIN_LIGHT_INDEX + 1;i++){
        float3 lightDir,lightIntensity;
        
        if(lights[i].lightType == LIGHT_TYPE_DIRECTIONAL){
            lightDir = -lights[i].vec;
            lightIntensity = lights[i].intensity;
        }else if(lights[i].lightType == LIGHT_TYPE_POINT){
            lightDir = normalize(lights[i].vec - vout.WorldPos);
            lightIntensity = CalcAttenuation(length(lights[i].vec - vout.WorldPos),lights[i].fallStart,lights[i].fallEnd)
                * lights[i].intensity;
        }

        float  NdotL = dot(lightDir,vout.WorldNorm);
        float  lIten = 0.;
        if(NdotL < divL){
            lIten = .3f;
        }else{
            lIten = smoothstep(divL,divL + 2e-3,NdotL) * .7 + .3;
        }
        color += albedo * lIten * lightIntensity;
    }


    return float4(color,1.);
}