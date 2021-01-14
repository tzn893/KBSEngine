RWTexture2D<float4> renderTarget : register(u0);

cbuffer Const : register(b0){
    float exposure;
};

[numthreads(16,16,1)]
void Map(uint3 id : SV_DISPATCHTHREADID){
    const float gamma = 1. / 2.2;

    float3 color = renderTarget[id.xy].rgb;
    color = 1. - exp(- color * exposure);

    renderTarget[id.xy].rgb = pow(color,gamma);
}