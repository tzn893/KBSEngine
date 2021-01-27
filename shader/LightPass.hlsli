#ifndef _LIGHT_PASS_
#define _LIGHT_PASS_

#ifndef LIGHT_PASS_REGISTER
#define LIGHT_PASS_REGISTER b2
#endif

#ifndef MAX_LIGHT_STRUCT_NUM
#define MAX_LIGHT_STRUCT_NUM 8
#endif

#ifndef LIGHT_TYPE_POINT
#define LIGHT_TYPE_POINT 0
#endif

#ifndef LIGHT_TYPE_DIRECTIONAL
#define LIGHT_TYPE_DIRECTIONAL 1
#endif

#define MAIN_LIGHT_INDEX 0

struct Light{
    float3 intensity;
    float  fallStart;
    float3 vec;
    float  fallEnd;
    uint    lightType;
    float3 padding;
};

cbuffer LightPass : register( LIGHT_PASS_REGISTER ){
    float4 ambient;
    Light lights[MAX_LIGHT_STRUCT_NUM];
    uint  lightNum;
};

#endif //_LIGHT_PASS_