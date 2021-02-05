RWTexture2D<float4> RTTex : register(u0);
Texture2D<float4> Tex : register(t0);
SamplerState rtSp : register(s0);

cbuffer State : register(b0){
    uint width,height;
    float threshold;
};

#include "ToneMapUtil.hlsli"

static const float exposure = 1.f;
static const float r_min_reduce = 1. / 128.f;
static const float r_mul_reduce = 1. / 8.f;
static const float2 max_span = float2(8.f,8.f);

float2 GetUv(uint2 xy){
    return float2(xy.x / (float)width,xy.y / (float)height);
}

float SampleLuminance(uint2 xy){
    float3 color = Tex.SampleLevel(rtSp,GetUv(xy),0).rgb;
    color = TM_STANDERD(color,exposure);
    return Luminance(color);
}

[numthreads(8,8,1)]
void CalculateLuminance(uint3 id : SV_DISPATCHTHREADID){
    if(id.x == 0 || id.x == width || id.y == 0 || id.y == height)
        return;
	uint2 xy = id.xy;
    float Lm,Lld,Lrd,Llu,Lru;
    Lm = SampleLuminance(xy);
    Lld = SampleLuminance(xy + uint2(-1,-1));
    Llu = SampleLuminance(xy + uint2(-1, 1));
    Lru = SampleLuminance(xy + uint2( 1, 1));
    Lrd = SampleLuminance(xy + uint2( 1,-1));
    
    float Lmax = max(Lm,max(Lld,max(Llu,max(Lru,Lrd))));
    float Lmin = min(Lm,min(Lld,min(Llu,min(Lru,Lrd))));

    if(Lmax - Lmin <= Lmax * threshold || Lm > threshold){
        return;
    }
    
    float2 sampleDir = 0.;
    sampleDir.x = -((Llu + Lru) - (Lld + Lrd));
    sampleDir.y = ((Llu + Lld) - (Lru + Lrd));

    //the brighter the area is,less the area will be blured
    float sampleDirReduce = max((Llu + Lru + Lld + Lrd) * .25 * r_mul_reduce,r_min_reduce);
    float sampleDirFactor = 1. / (min(abs(sampleDir.x),abs(sampleDir.y)) + sampleDirReduce);

    float2 currUv = GetUv(xy);
    float2 sampleStep = clamp(sampleDir * sampleDirFactor,-max_span,max_span) * float2(1. / width,1. / height);
    float3 blurRes2 = 0.;
    blurRes2 += (Tex.SampleLevel(rtSp,currUv + sampleStep * (1. / 3. - .5),0.).rgb + 
                    Tex.SampleLevel(rtSp,currUv + sampleStep * (2. / 3. - .5),0.).rgb) * .5;
    
    float3 blurRes4 = blurRes2;
    blurRes4 += (Tex.SampleLevel(rtSp,currUv + sampleStep * .5,0.).rgb + 
                    Tex.SampleLevel(rtSp,currUv - sampleStep * .5,0.).rgb) * .5;
    blurRes4 *= .5;

    //prevent race condition
    GroupMemoryBarrierWithGroupSync();

    float L4 = Luminance(TM_STANDERD(blurRes4,exposure)),
        L2 = Luminance(TM_STANDERD(blurRes2,exposure));
    if(L4 > Lmin && L4 < Lmax){
        RTTex[id.xy] = float4(blurRes4,1.);
    }else if(L2 > Lmin && L2 < Lmax){
        RTTex[id.xy] = float4(blurRes2,1.);
    }
};