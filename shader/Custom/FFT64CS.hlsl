int bitInverse64(int index){
    int rv = 0;
    rv |= (index & 0b1) << 5;
    rv |= (index & 0b10) << 3;
    rv |= (index & 0b100) << 1;
    rv |= (index & 0b1000) >> 1;
    rv |= (index & 0b10000) >> 3;
    rv |= (index & 0b100000) >> 5;
    return rv;
}

#ifdef TRY_UNROLL_LOOPS
#define UNROLL(x) [unroll(x)]
#else
#define UNROLL(x) (x);
#endif


//u0,u1,u2
RWTexture2D<float2> targetTextures[3] : register(u0);

cbuffer FFTState{
    float length;
    float3 trash;
};

const float PI = 3.14159265;

float2 W(int N,int K){
    float w = (float)K * PI * 2 / (float)N;
    return float2(cos(w),sin(w));
}

float2 complexMul(float2 c1,float2 c2){
    return float2(c1.x * c2.x - c1.y * c2.y,
                c1.x * c2.y + c1.y * c2.x);
}


void PerformFFTOnDir64(uint2 hori,uint2 start,uint mapI){
    float2 buffer[2][64];
    int input = 0,output = 1;

    UNROLL(64) for(int i = 0;i != 64;i++){
        buffer[output][bitInverse64(i)] = targetTextures[mapI][start + hori * i];
    }

    for(int netWidth = 2;netWidth < 64;netWidth = netWidth << 1){
        int tmp = input;
        input = output;
        output = tmp;
        for(int groupStart = 0;groupStart < 64;groupStart += netWidth){
            int halfWidth = netWidth / 2;
            for(int index = 0;index < groupStart / 2;index++){
                float2 WT =  complexMul(W(netWidth,index),buffer[input][index + groupStart + halfWidth]);
                buffer[output][index + groupStart] = buffer[input][index + groupStart] + WT;
                buffer[output][index + groupStart + halfWidth] = buffer[input][index + groupStart] - WT;
            }
        }
    }

    UNROLL(64) for(int i = 0;i != 64;i++){
         targetTextures[mapI][start + hori * i] = buffer[output][i];
    }
}



[numthreads(64,1,3)]
//perform fft transform on a 64x64 texture
//only 1x1x1 group will be dispatched
void PerformFFT64(uint3 threadID : SV_DISPATCHTHREADID){
    uint ky = threadID.x,mapI = threadID.z;

    int2 vert = int2(0,1),hori = int2(1,0);
    int2 start = ky * vert;
    PerformFFTOnDir64(hori,start,mapI);

    //synconize all the threads in this group
    GroupMemoryBarrierWithGroupSync();
    
    int2 start = ky * hori;
    PerformFFTOnDir64(vert,start,mapI);

    UNROLL(64) for(int i = 0;i != 64;i++){
        float sign = (float)((ky + i) & 1 ) * 2. - 1.;
        targetTexture[mapI][int2(i,ky)] *= sign;
    }
}