Texture2D<float4> renderTarget : register(t0);
RWTexture2D<float4> tempTexture0  : register(u0);
/*
RWTexture2D<float4> tempTexture1  : register(u1);
RWTexture2D<float4> tempTexture2  : register(u2);
RWTexture2D<float4> tempTexture3  : register(u3);
*/
SamplerState sp : register(s0);

cbuffer State : register(b0){
    float threshold;
    uint  width,height;
};
/*
groupshared float gR[64];
groupshared float gG[64];
groupshared float gB[64];

void PackColor(uint index,float3 color){
    gR[index] = color.r,gG[index] = color.g,gB[index] = color.b;
}
float3 UnpackColor(uint index){
    return float3(gR[index],gG[index],gB[index]);
}
*/
float2 GetUv(uint2 xy){
    return (float2)xy / float2(width,height);
}

#include "ToneMapUtil.hlsli"

float3 SampleColor(uint2 xy){
    float3 c1 = renderTarget.SampleLevel(sp,GetUv(xy + uint2(-1, 1)),0.).rgb;
    float3 c2 = renderTarget.SampleLevel(sp,GetUv(xy + uint2( 1, 1)),0.).rgb;
    float3 c3 = renderTarget.SampleLevel(sp,GetUv(xy + uint2(-1,-1)),0.).rgb;
    float3 c4 = renderTarget.SampleLevel(sp,GetUv(xy + uint2( 1,-1)),0.).rgb;

    const float eps = 1e-4;

    float l1 = Luminance(c1),l2 = Luminance(c2),l3 = Luminance(c3),l4 = Luminance(c4);
    
    c1 *= max(eps,l1 - threshold) / (l1 + eps);
    c2 *= max(eps,l2 - threshold) / (l2 + eps);
    c3 *= max(eps,l3 - threshold) / (l3 + eps);
    c4 *= max(eps,l4 - threshold) / (l4 + eps);

    float w1,w2,w3,w4,w;
    const float offset = 1.;
    w1 = 1. / (l1 * l1 + offset);
    w2 = 1. / (l2 * l2 + offset);
    w3 = 1. / (l3 * l3 + offset);
    w4 = 1. / (l4 * l4 + offset);
    w = w1 + w2 + w3 + w4;

    return (c1 * w1 + c2 * w2 + c3 * w3 + c4 * w4) / w;
}

[numthreads(8,8,1)]
void DownSample(uint gid : SV_GROUPINDEX,uint3 did : SV_DISPATCHTHREADID){
    uint2 tid = did.xy;
	uint index = gid;
    float3 srcColor = SampleColor(tid);
    
    //PackColor(index,srcColor);
    tempTexture0[tid] = float4(srcColor, 1.);

	/*float3 color = 0.;
	if ((index & 0x9) == 0) {
		color = UnpackColor(index) + UnpackColor(index + 0x1)
			+ UnpackColor(index + 0x8) + UnpackColor(index + 0x9);
		color *= .25;
		tempTexture1[tid / 2] = float4(color, 1.);
    }

	GroupMemoryBarrierWithGroupSync();
	PackColor(index, color);

    if((index & 0x1B) == 0){
        color = UnpackColor(index) + UnpackColor(index + 0x2) +
        UnpackColor(index + 0x10) + UnpackColor(index + 0x12);
        color *= .25;
		tempTexture2[tid / 4] = float4(color, 1.);
    }
	GroupMemoryBarrierWithGroupSync();
	PackColor(index, color);

    if(index == 0){
        float3 color = UnpackColor(index) + UnpackColor(index + 0x4) +
        UnpackColor(index + 0x20) + UnpackColor(index + 24);
        color *= .25;
		GroupMemoryBarrierWithGroupSync();
		tempTexture3[tid / 8] = float4(color,1.);
    }*/
}

