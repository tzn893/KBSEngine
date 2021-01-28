
struct GBufferData{
    float4 worldPos;
    float3 worldNormal;
    float metallic;
    float3 diffuse;
    float roughness;
};


#ifdef GBUFFER_OUTPUT
    struct GBufferOut{
        float4 Buffer1 : SV_TARGET0;
        float4 Buffer2 : SV_TARGET1;
        float4 Buffer3 : SV_TARGET2;
    };

    /*
    GBuffer1 (worldpos    3,exists    1)
    GBuffer2 (worldNormal 3,metallic  1)
    GBuffer3 (diffuse     3,roughness 1)
    */

    GBufferOut Pack2GBuffer(GBufferData data){
        GBufferOut output;
        output.Buffer1 = float4(data.worldPos.xyz,1.f);
        output.Buffer2.xyz = normalize(data.worldNormal);
        output.Buffer3.xyz = data.diffuse;
        output.Buffer3.w = data.roughness;
        output.Buffer2.w = data.metallic;

        return output;
    }
#else

    #ifndef GBUFFER_REGISTER
        #define GBUFFER_REGISTER t0
    #endif

    #ifndef GBUFFER_SAMPLER_REGISTER
        #define GBUFFER_SAMPLER_REGISTER s0
    #endif

    Texture2D GBuffer[3] : register(t0);
    SamplerState gsp : register(s0);


    GBufferData UnpackGBuffer(float2 uv){
        float4 pos = GBuffer[0].Sample(gsp,uv);
        GBufferData data;
        data.worldPos = pos;
        float4 gdata1 = GBuffer[1].Sample(gsp,uv);
        data.worldNormal = gdata1.xyz;
        data.metallic = gdata1.w;
        float4 gdata2 = GBuffer[2].Sample(gsp,uv);
        data.diffuse = gdata2.xyz;
        data.roughness = gdata2.w;
        return data;
    }

    void GBufferDiscard(float4 pos){
        if(pos.w < 1e-4) discard;
    }
#endif