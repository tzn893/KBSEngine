RWTexture2D<float4> renderTarget : register(u0);
RWTexture2D<float4> tempTexture  : register(u1);

cbuffer Const : register(b0){
    float3 bloomThreshold;
};

[numthread(16,16,1)]
void BloomingClip(uint3 id : SV_DISPATCHTHREADID){
    float3 color = renderTarget[id.xy].rgb;
    if(dot(color,bloomThreshold) > 1.){
        tempTexture[id.xy] = float4(color,1.);
    }else{
        tempTexture[id.xy] = float4(0.,0.,0.,1.);
    }
}

[numthread(16,16,1)]
void BloomingBind(uint3 id : SV_DISPATCHTHREADID){
    renderTarget[id.xy].rgb = renderTarget[id.xy].rgb + tempTexture[id.xy].rgb;
}