RWTexture2D<float4> outputTex0 : register(u0);
RWTexture2D<float4> outputTex1 : register(u1);
RWTexture2D<float4> outputTex2 : register(u2);
RWTexture2D<float4> outputTex3 : register(u3);

Texture2D<float4> inputTex : register(t0);
SamplerState mipSampler : register(s0);

cbuffer MipState : register(b0){
    uint SrcMip;
    uint TarMipNum;
    float2 TexelSize;
    uint2  TextureSize;
};

groupshared float gR[64];
groupshared float gG[64];
groupshared float gB[64];
groupshared float gA[64];

void StoreColor(uint index,float4 color){
    gR[index] = color.r;
    gG[index] = color.g;
    gB[index] = color.b;
    gA[index] = color.a;
}

float4 LoadColor(uint index){
    return float4(gR[index],gG[index],gB[index],gA[index]);
}


[numthreads(8,8,1)]
void GenerateMipmap(uint gid : SV_GROUPINDEX,uint3 did : SV_DISPATCHTHREADID){
    float4 srcColor;
    uint2 tid = did.xy;
	uint index = gid;

    if(TextureSize.x % 2 == 0 && TextureSize.y % 2 == 0){
        float2 uv = ((float2)tid + .5 )* TexelSize;
        srcColor = inputTex.SampleLevel(mipSampler,uv,SrcMip); 
    }else if(TextureSize.x % 2 == 0){
        float2 uv = ((float2)tid + float2(.5,.25)) * TexelSize;
        float2 off = float2(0.,.5) * TexelSize;
        srcColor = (inputTex.SampleLevel(mipSampler,uv,SrcMip) +
            inputTex.SampleLevel(mipSampler,uv + off,SrcMip)) * .5;
    }else if(TextureSize.y % 2 == 0){
        float2 uv = ((float2)tid + float2(.25,.5)) * TexelSize;
        float2 off = float2(0.5,0.) * TexelSize;
        srcColor = (inputTex.SampleLevel(mipSampler,uv,SrcMip) +
            inputTex.SampleLevel(mipSampler,uv + off,SrcMip)) * .5;
    }else{
        float2 uv = ((float2)tid + .25) * TexelSize;
        srcColor = inputTex.SampleLevel(mipSampler,uv,SrcMip);
        srcColor += inputTex.SampleLevel(mipSampler,uv + float2(.5,0.),SrcMip);
        srcColor += inputTex.SampleLevel(mipSampler,uv + float2(0.,.5),SrcMip);
        srcColor += inputTex.SampleLevel(mipSampler,uv + float2(.5,.5),SrcMip);
        srcColor *= .25;
    }
    
    float2 uv = ((float2)tid + .5 )* TexelSize;
    srcColor = inputTex.SampleLevel(mipSampler,uv,SrcMip); 

    outputTex0[tid] = srcColor;
    
    if(TarMipNum <= 1) return;

    StoreColor(index,srcColor);
    GroupMemoryBarrierWithGroupSync();

    if((gid & 0x9) == 0){
        float4 color = LoadColor(index) + LoadColor(index + 0x1)
        + LoadColor(index + 0x8) + LoadColor(index + 0x9);
        color *= .25;
        outputTex1[tid / 2] = color;
        StoreColor(index,color);
    }

    if(TarMipNum <= 2) return;
    GroupMemoryBarrierWithGroupSync();

    if((gid & 0x1B) == 0){
        float4 color = LoadColor(index) + LoadColor(index + 0x2) +
        LoadColor(index + 0x10) + LoadColor(index + 0x12);
        color *= .25;
        outputTex2[tid / 4] = color;
        StoreColor(index,color);
    }
    if(TarMipNum <= 3) return;
    GroupMemoryBarrierWithGroupSync();

    if(gid == 0){
        float4 color = LoadColor(index) + LoadColor(index + 0x4) +
        LoadColor(index + 0x20) + LoadColor(index + 24);
        color *= .25;
        outputTex3[tid / 8] = color;
    }

}
