
float Luminance(float3 color){
    return dot(color,float3(0.25f, 0.50f, 0.25f));
}

float3 ACESMapping(float3 color,float lum){
    color *= (1. / lum);
    const float A = 2.51f;
    const float B = .03f;
    const float C = 2.43f;
    const float D = .59f;
    const float E = .14f;

    return ((A * color + B) * color) / ((C * color + D) * color + E);
}

float3 ReinhardMapping(float3 color,float exposure){
    return color / max(Luminance(color) + exposure,1e-3);
}

float3 InvReinhardMapping(float3 color,float exposure){
    return color /max(-Luminance(color) + 1.,1e-3) * exposure;
}

#ifndef DISABLE_TONEMAPPING
    #define TM_STANDERD ReinhardMapping
    #define ITM_STANDERD InvReinhardMapping
#else
    #define TM_STANDERD
    #define ITM_STANDERD
#endif