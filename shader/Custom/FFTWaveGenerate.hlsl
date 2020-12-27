cbuffer Constant : register(b0){
    float time;
    float3 trash1;
    float L;
    float3 trash2;
}

#ifndef GRID_ROW_NUM
#define GRID_ROW_NUM 64
#endif

static const float g = 9.81;

#define HEIGHT_MAP 0
#define GRANDIENT_X 1
#define GRANDIENT_Z 2

RWTexture2D<float2> Maps[3] : register(u0);
RWTexture2D<float2> HMap : register(u3);
RWTexture2D<float2> HConjMap : register(u4);

float2 complexMul(float2 c1,float2 c2){
    return float2(
        c1.x * c2.x - c1.y * c2.y,
        c1.x * c2.y + c1.y * c2.x
    );
}

float2 complexExp(float im){
    return float2(cos(im),sin(im));
}

float2 HTile(uint2 pos,float t){
    float2 h0 = HMap[pos],h0conj = HConjMap[pos];
    float2 k = ((float2)pos * 2. - GRID_ROW_NUM) / L;
    float  w0 = sqrt(length(k) * g) * time;
    return complexMul(h0,complexExp(w0)) + complexMul(h0conj,complexExp(-w0));
}


[numthreads(16,16,1)]
//normaly a 4x4x1 thread group will be dispatched
void GenerateHFrequence(uint3 dispatchID : SV_DISPATCHTHREADID){
    uint2 pos = dispatchID.xy;

    float2 k = ((float2)pos * 2. - GRID_ROW_NUM) / L;

    float2 ht = HTile(pos,time);
    Maps[HEIGHT_MAP][pos] = ht;
    Maps[GRANDIENT_X][pos] = complexMul(ht,float2(0,k.x));
    Maps[GRANDIENT_Z][pos] = complexMul(ht,float2(0,k.y));
}

