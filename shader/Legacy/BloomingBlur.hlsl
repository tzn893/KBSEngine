RWTexture2D<float4> output : register(u0);
RWTexture2D<float4> input  : register(u1);

static float kernel[5] = {0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216};

cbuffer State : register(b0){
    uint width,height;
};

groupshared float3 color[256];

void PackColor(uint2 xy,float3 c){
    color[xy.y * 16 + xy.x + 36] = c;
}

float3 LoadColor(uint2 xy){
    return color[xy.y * 16 + xy.x + 36];
}

float3 BlurHorizontal(uint2 xy){
    float3 a0 = LoadColor(xy - uint2(4,0)),a1 = LoadColor(xy + uint2(4,0));
    float3 b0 = LoadColor(xy - uint2(3,0)),b1 = LoadColor(xy + uint2(3,0));
    float3 c0 = LoadColor(xy - uint2(2,0)),c1 = LoadColor(xy + uint2(2,0));
    float3 d0 = LoadColor(xy - uint2(1,0)),d1 = LoadColor(xy + uint2(1,0));
    float3 e = LoadColor(xy);

    return (a0 + a1)* kernel[4] + (b0 + b1) * kernel[3] + (c0 + c1) * kernel[2] +
        (d0 + d1)  * kernel[1] + e  * kernel[0];
}

float3 BlurVertical(uint2 xy){
    float3 a0 = LoadColor(xy - uint2(0,4)),a1 = LoadColor(xy + uint2(0,4));
    float3 b0 = LoadColor(xy - uint2(0,3)),b1 = LoadColor(xy + uint2(0,3));
    float3 c0 = LoadColor(xy - uint2(0,2)),c1 = LoadColor(xy + uint2(0,2));
    float3 d0 = LoadColor(xy - uint2(0,1)),d1 = LoadColor(xy + uint2(0,1));
    float3 e = LoadColor(xy);

    return (a0 + a1)* kernel[4] + (b0 + b1) * kernel[3] + (c0 + c1) * kernel[2] +
        (d0 + d1)  * kernel[1] + e  * kernel[0];
}

[numthreads(8,8,1)]
void Blur(uint3 did : SV_DISPATCHTHREADID,
            uint3 gid : SV_GROUPTHREADID){
    uint2 tid = did.xy,xy = gid.xy;
    float3 color = input[tid].rgb;
    PackColor(xy,color);
    if(xy.x < 4){
        uint2 id = tid;
        id.x = max(id.x - 4,0);
        PackColor(xy - uint2(4,0),input[id].rgb);
    }
    if(8 - xy.x <= 4){
        uint2 id = tid;
        id.x = min(id.x + 4,width);
        PackColor(xy + uint2(4,0),input[id].rgb);
    }
    if(xy.y < 4){
        uint2 id = tid;
        id.y = max(id.y - 4,0);
        PackColor(xy - uint2(0,4),input[id].rgb);
    }
    if(8 - xy.y <= 4){
        uint2 id = tid;
        id.y = min(id.y + 4,height);
        PackColor(xy + uint2(0,4),input[id].rgb);
    }
    GroupMemoryBarrierWithGroupSync();

    color = BlurVertical(xy);
    GroupMemoryBarrierWithGroupSync();
    PackColor(xy,color);

    GroupMemoryBarrierWithGroupSync();
    color = BlurHorizontal(xy);

    output[did.xy] = float4(color,1.);
}