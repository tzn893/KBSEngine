#define  USE_IBL_K
#include "PBRLightingUtil.hlsli"

#ifndef SAMPLE_NUM
#define SAMPLE_NUM 512
#endif

RWTexture2D<float2> lutTex : register(u0);

cbuffer TexState : register(b0){
     uint texWidth;
     uint texHeight;
};

float2 IntegrateBRDF(float NdotV,float roughness){
    float3 V = float3(sqrt(1. - NdotV * NdotV),0.,NdotV);

    float2 result = 0.;
    float3 N = float3(0.,0.,1.);
    for(uint i = 0;i != SAMPLE_NUM;i++){
        float3 H = ImportanceGGX(i,SAMPLE_NUM,N,roughness);
        float3 L = normalize(2. * dot(V,H) * H - V);

        float NdotL = max(L.z,0.);
        float NdotH = max(H.z,1e-6);
        float VdotH = max(dot(V,H),1e-6);

        if(NdotL > 0.){
            float G = GeometrySmith(N,V,L,roughness);
            float Gv = (G * VdotH) / (NdotH * max(NdotV,1e-6));
            float Fc = pow(1. - VdotH,5.);

            result += float2((1. - Fc) , Fc) * Gv;
        }
    }
    result = result / (float)SAMPLE_NUM;
    return result;
}

[numthreads(8,8,1)]
void PrepareLUT(uint3 id : SV_DISPATCHTHREADID){
    float NdotV = (float)id.x / (float)texWidth;
    float roughness = (float)id.y / (float)texHeight;
	lutTex[id.xy] = IntegrateBRDF(NdotV,roughness);
}
