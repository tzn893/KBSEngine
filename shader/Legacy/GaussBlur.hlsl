RWTexture2D<float4> texInput : register(u0);
RWTexture2D<float4> texOutput: register(u1);

#ifndef CUSTOM_GAUSS_KERNEL
static const uint kernelHalf = 2;
static const float kernel[5] = {0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216}; 
#else
cbuffer Kernel{
    uint kernelHalf;
    float kernel[49];
};
#endif

[numthreads(16,16,1)]
void GaussHorizontal(uint3 id : SV_DISPATCHTHREADID){
    float4 res = 0.;
    for(uint i = - kernelHalf;i <= kernelHalf;i++){
        res += texInput[uint2(id.x + i,id.y)] * (i + kernelHalf);
    }
    texOutput[id.xy] = res / (float)(kernelHalf * 2 + 1);
}

[numthreads(16,16,1)]
void GaussVertical(uint3 id : SV_DISPATCHTHREADID){
    float4 res = 0.;
    for(uint i = - kernelHalf;i <= kernelHalf;i++){
        res += texInput[uint2(id.x,id.y + i)] * (i + kernelHalf);
    }
    texOutput[id.xy] = res / (float)(kernelHalf * 2 + 1);
}