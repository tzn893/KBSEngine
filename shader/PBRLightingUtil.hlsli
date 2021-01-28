#ifndef PI
#define PI 3.1415926
#endif

float3 FresnelSchlick(float cosTheta,float3 F0){
    float t1 = 1. - cosTheta,t12 = t1 * t1;
    return F0 + (1.0 - F0) * t12 * t12 * t1;
}

float  DistributionGGX(float3 N,float3 H,float roughness){
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N,H),0.0);
    float NdotH2 = NdotH * NdotH;

    float denom = (NdotH2 * (a2 - 1.) + 1.);
    denom = (denom * denom * PI);
    return a2 / denom;
}

float GeometrySchlickGGX(float NdotV,float roughness){
    float r = (roughness + 1.);
    float k = r * r / 8.;

    float denom = NdotV * (1. - k) + k;
    float nom = NdotV;

    return nom / denom;
}

float GeometrySmith(float3 N,float3 V,float3 L,float roughness){
    float NdotV = max(dot(N,V),0.);
    float NdotL = max(dot(N,L),0.);
    float ggx1 = GeometrySchlickGGX(NdotV,roughness);
    float ggx2 = GeometrySchlickGGX(NdotL,roughness);

    return ggx1 * ggx2;
}


float3 CT_BRDF(float3 N,float3 L,float3 V,float3 Diffuse,
            float Roughness,float Metallic,float3 Radiance,
            float3 F0){
    F0 = lerp(F0,Diffuse,Metallic);

    float3 H = normalize(L + V);

    float HdotL = max(dot(H,L),0.);
    
    float NDF = DistributionGGX(N,H,Roughness);
    float F   = FresnelSchlick(HdotL,F0);
    float G   = GeometrySmith(N,V,L,Roughness);

    float Spnom = NDF * F * G;
    float Spdenom = max(4 * max(0.,dot(N,V)) * max(0.,dot(N,L)),1e-4);

    float Spec = Spnom / Spdenom;

    float3 kD = (1. - F) * (1. - Metallic);
    float NdotL = max(dot(N,L),0.);

    return (kD * Diffuse / PI + Spec) * NdotL * Radiance;
}

#define BRDF CT_BRDF