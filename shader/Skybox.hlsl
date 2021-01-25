#define CAMERA_PASS_REGISTER b0
#include "CameraPass.hlsli"
#include "ToneMapUtil.hlsli"

struct VertexIn{
    float3 Position : POSITION;
};

struct VertexOut{
    float3 relaPos : POSITION;
    float4 Position : SV_Position;
};

TextureCube skyBox : register(t0);
SamplerState skySampler : register(s0);


VertexOut VS(VertexIn input){
    VertexOut output;
    
    float3 worldPos = input.Position + cameraPos;
    output.Position = mul(perspect,mul(view,float4(worldPos,1.f))).xyzz;
    output.relaPos =  -input.Position;

    return output;
}

float4 PS(VertexOut input) : SV_TARGET {
    float3 target = skyBox.Sample(skySampler,input.relaPos);

    target = ITM_STANDERD(target,exposure);

    return float4(target,1.);
}