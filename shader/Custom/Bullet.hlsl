#define CAMERA_PASS_REGISTER b0
#include "../CameraPass.hlsli"

struct Bullet{
    float3 Position;
    float  trash;
};

StructuredBuffer<Bullet> bulletBuffer : register(t0);


float4 VS(float3 pos : POSITION,uint iid : SV_INSTANCEID) : SV_POSITION{
    float3 worldPos = pos + bulletBuffer[iid].Position;
    float4 screenPos = mul(perspect,mul(view,float4(worldPos,1.)));
    return screenPos;
}


float4 PS() : SV_TARGET{
    return float4(1.,1.,1.,1.);
}