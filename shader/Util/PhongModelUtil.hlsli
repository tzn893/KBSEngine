#ifndef _PHONG_MODEL_
#define _PHONG_MODEL_

//#include "LightPass.hlsli"
//#include "ObjectPass.hlsli"

#ifndef _OBJECT_PASS_
struct Material{
    float4 diffuse;
    float3 FresnelR0;
    float  Roughness;
    float4x4 matTransform;
};
#endif //_OBJECT_PASS_

#ifndef _LIGHT_PASS_
struct Light{
    float3 intensity;
    float  fallStart;
    float3 vec;
    float  fallEnd;
    int    lightType;
    float3 padding;
};
#endif //_LIGHT_PASS_

float CalcAttenuation(float d, float falloffStart, float falloffEnd)
{
    // Linear falloff.
    return saturate((falloffEnd-d) / (falloffEnd - falloffStart));
}

// Schlick gives an approximation to Fresnel reflectance (see pg. 233 "Real-Time Rendering 3rd Ed.").
// R0 = ( (n-1)/(n+1) )^2, where n is the index of refraction.
float3 SchlickFresnel(float3 R0, float3 normal, float3 lightVec)
{
    float cosIncidentAngle = saturate(dot(normal, lightVec));

    float f0 = 1.0f - cosIncidentAngle;
    float3 reflectPercent = R0 + (1.0f - R0)*(f0*f0*f0*f0*f0);

    return reflectPercent;
}

float3 BlinnPhong(float3 lightStrength, float3 lightVec, float3 normal, float3 toEye, Material mat)
{
    const float m = (1. -  mat.Roughness) * 256.0f;
    float3 halfVec = normalize(toEye + lightVec);

    float roughnessFactor = (m + 8.0f)*pow(max(dot(halfVec, normal), 0.0f), m) / 8.0f;
    float3 fresnelFactor = SchlickFresnel(mat.FresnelR0, halfVec, lightVec);

    float3 specAlbedo = fresnelFactor*roughnessFactor;

    // Our spec formula goes outside [0,1] range, but we are 
    // doing LDR rendering.  So scale it down a bit.
    specAlbedo = specAlbedo / (specAlbedo + 1.0f);

    return (mat.diffuse.rgb + specAlbedo) * lightStrength;
}

float3 ComputePointLight(Light light,Material mat,float3 toEye,float3 Pos,float3 normal){
    float3 lightVec = normalize(light.vec - Pos);
    float3 lightStrength = light.intensity;

    float dis = length(light.vec - Pos);
    if(dis > light.fallEnd) return float3(0.f,0.f,0.f);

    lightStrength *= max(dot(lightVec,normal),0.);
    lightStrength *=  CalcAttenuation(dis,light.fallStart,light.fallEnd);

    return BlinnPhong(lightStrength,lightVec,normal,toEye,mat);
}

float3 ComputeDirectionalLight(Light light,Material mat,float3 toEye,float3 Pos,float3 normal){
    float3 lightVec = - normalize(light.vec);
    float3 lightStrength = light.intensity;
    
    lightStrength *= max(dot(lightVec,normal),0.);

    return BlinnPhong(lightStrength,lightVec,normal,toEye,mat);
}

#endif //_PHONG_MODEL_