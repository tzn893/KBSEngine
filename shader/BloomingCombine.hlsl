RWTexture2D<float4> RT : register(u0);
Texture2D<float4> Tex : register(t0);
SamplerState sp : register(s0);
cbuffer State : register(b0){
    uint width,height;
    float weights0;
    float weights1;
    float weights2;
    float weights3;
};


[numthreads(8,8,1)]
void Combine(uint3 id : SV_DISPATCHTHREADID){
    float3 result = RT[id.xy].rgb;
    float2 uv = (float2)id.xy / float2(width,height);

    result += Tex.SampleLevel(sp,uv,0.) * weights0;
    result += Tex.SampleLevel(sp,uv,1.) * weights1;
    result += Tex.SampleLevel(sp,uv,2.) * weights2;
    result += Tex.SampleLevel(sp,uv,3.) * weights3;

    RT[id.xy] = float4(result,1.);
}