#ifndef _OBJECT_PASS_
#define _OBJECT_PASS_

#ifndef OBJECT_PASS_REGISTER
#define OBJECT_PASS_REGISTER b0
#endif

struct Material{
    float4 diffuse;
    float3 FresnelR0;
    float  Roughness;
};

cbuffer ObjectPass : register(OBJECT_PASS_REGISTER){
    float4x4 world;
    float4x4 transInvWorld;
    Material mat;
};

#endif //_OBJECT_PASS_