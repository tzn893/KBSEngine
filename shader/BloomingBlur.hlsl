RWTexture2D<float4> output : register(u0);
RWTexture2D<float4> input  : register(u1);

static float kernel[5] = {0.1415094,0.226415,0.26415, 0.226415,0.1415094};

cbuffer State : register(b0){
    uint width,height;
};

groupshared float3 color[256];

void PackColor(uint2 xy,float3 c){
    color[xy.y * 12 + xy.x + 26] = c;
}

float3 LoadColor(uint2 xy){
    return color[xy.y * 12 + xy.x + 26];
}

float3 BlurHorizontal(uint2 xy){
    float3 a = LoadColor(xy - uint2(2,0));
    float3 b = LoadColor(xy - uint2(1,0));
    float3 c = LoadColor(xy);
    float3 d = LoadColor(xy + uint2(1,0));
    float3 e = LoadColor(xy + uint2(2,0));
    return a * kernel[0] + b * kernel[1] + c  * kernel[2] +
        d  * kernel[3] + e  * kernel[4];
}

float3 BlurVertical(uint2 xy){
    float3 a = LoadColor(xy - uint2(0,2));
    float3 b = LoadColor(xy - uint2(0,1));
    float3 c = LoadColor(xy);
    float3 d = LoadColor(xy + uint2(0,1));
    float3 e = LoadColor(xy + uint2(0,2));
    return a * kernel[0] + b * kernel[1] + c  * kernel[2] +
        d  * kernel[3] + e  * kernel[4];
}

[numthreads(8,8,1)]
void Blur(uint3 did : SV_DISPATCHTHREADID,
            uint3 gid : SV_GROUPTHREADID){
    uint2 tid = did.xy,xy = gid.xy;
    float3 color = input[tid].rgb;
    PackColor(xy,color);
    if(xy.x < 2){
        uint2 id = tid;
        id.x = max(id.x - 2,0);
        PackColor(xy - uint2(2,0),input[id].rgb);
    }
    if(8 - xy.x <= 2){
        uint2 id = tid;
        id.x = min(id.x + 2,width);
        PackColor(xy + uint2(2,0),input[id].rgb);
    }
    if(xy.y < 2){
        uint2 id = tid;
        id.y = max(id.y - 2,0);
        PackColor(xy - uint2(0,2),input[id].rgb);
    }
    if(8 - xy.y <= 2){
        uint2 id = tid;
        id.y = min(id.y + 2,height);
        PackColor(xy + uint2(0,2),input[id].rgb);
    }
    GroupMemoryBarrierWithGroupSync();

    color = BlurVertical(xy);
    GroupMemoryBarrierWithGroupSync();
    PackColor(xy,color);

    GroupMemoryBarrierWithGroupSync();
    color = BlurHorizontal(xy);

    output[did.xy] = float4(color,1.);
}