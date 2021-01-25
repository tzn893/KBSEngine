
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
    return color / (Luminance(color) + exposure);
}

float3 InvReinhardMapping(float3 color,float exposure){
    return color / (-Luminance(color) + exposure) * exposure;
}


#define TM_STANDERD ReinhardMapping
#define ITM_STANDERD InvReinhardMapping           