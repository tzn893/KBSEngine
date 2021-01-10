RWTexture2D<float4> renderTarget : register(u0);
cbuffer Constant : register(b0) {
    float width,height;
    float healthFactor;
};



[numthreads(16,16,1)]
void main(uint3 did : SV_DISPATCHTHREADID){
    float2 uv = float2((float)did.x / width,(float)did.y / height);
    float dis = length(uv - .5);
    float blendFactor = 0.;
    if(dis < .1) blendFactor = .1 * healthFactor;
    else if(dis > .1 && dis < .4) blendFactor = (2. * dis - .1) * healthFactor;
    else blendFactor = .7 * healthFactor;

    renderTarget[did.xy] = renderTarget[did.xy] * (1. - blendFactor) + float4(1.,0.,0.,1.) * blendFactor;
}