struct GBufferData{
    float3 worldNormal;
    float metallic;
    float3 diffuse;
    float roughness;
    float3 emission;
};

#ifdef GBUFFER_OUTPUT

/*
GBuffer 0(worldNormal 3,metallic  1)
GBuffer 1(diffuse     3,roughness 1)
GBuffer 2(emission    3,spare     1)
*/
struct GBufferOutput {
    float4 gbuffer0 : SV_TARGET0;
    float4 gbuffer1 : SV_TARGET1;
    float4 gbuffer2 : SV_TARGET2;
};




#endif


#ifdef GBUFFER_INPUT

#ifndef GBUFFER_BASE_REGISTER 
#define GBUFFER_BASE_REGISTER t0
#endif

#ifndef GBUFFER_DEPTH_BUFFER_BASE_REGISTER
#define GBUFFER_DEPTH_BUFFER_BASE_REGISTER t3
#endif

Texture2D gbuffer[3] : register(GBUFFER_BASE_REGISTER);
Texture2D depthBuffer : register(GBUFFER_DEPTH_BUFFER_BASE_REGISTER);



#endif