int bitInverse64(int index){
    int rv = 0;
    rv |= (index & 1) << 5;
    rv |= (index & 2) << 3;
    rv |= (index & 4) << 1;
    rv |= (index & 8) >> 1;
    rv |= (index & 16) >> 3;
    rv |= (index & 32) >> 5;
    return rv;
}

#ifdef TRY_UNROLL_LOOPS
#define UNROLL(x) [unroll(x)]
#else
#define UNROLL(x) (x);
#endif


//u0,u1,u2
RWTexture2D<float2> inputTexture : register(u0);
RWTexture2D<float2> outputTexture : register(u1);

cbuffer FFTState : register(b0){
    //float2 timeLength;
    //float2 trash1;
    //int3   NSA;//(netWidth,stageNum,adjust)
    
    int netWidth;
    float3 trash2;
    int stageNum;
    float3 trash3;
    int adjust;
    float trash4;
};

static const float PI = 3.14159265;

float2 W(int N,int K){
    float w = (float)K * PI * 2. / (float)N;
    return float2(cos(w),sin(w));
}

float2 complexMul(float2 c1,float2 c2){
    return float2(c1.x * c2.x - c1.y * c2.y,
                c1.x * c2.y + c1.y * c2.x);
}

[numthreads(16,16,1)]
//a 4x4 group will be dispatched
void PerformFFTHorizontal(uint3 id : SV_DISPATCHTHREADID){
    uint2 xy = id.xy;
    //if the dispatch is the last on this direction 
    //the adjust value will be set to 1
    uint NS = pow(2,stageNum),NSHalf = NS / 2;
    uint2 iid = xy;
    iid.x = floor(iid.x / NS) * NSHalf + iid.x % NSHalf;

    float2 x0 = inputTexture[iid];
    float2 x1 = inputTexture[uint2(iid.x + NSHalf,iid.y)];

    float2 wt = complexMul(W(NS,xy.x),x1);
    if(adjust == 1){
        wt *= -1;
    }
    float2 output = x0 + wt;
    if(adjust == 1){
        output *= ((xy.x + 1) % 2) * 2. - 1.;
    }
    outputTexture[xy] = output;
}


[numthreads(16,16,1)]
//a 4x4 group will be dispatched
void PerformFFTVertical(uint3 id : SV_DISPATCHTHREADID){
    uint2 xy = id.xy;
    
    uint NS = pow(2,stageNum),NSHalf = NS / 2;
    uint2 iid = xy;
    iid.y = floor(iid.y / NS) * NSHalf + iid.y % NSHalf;

    float2 x0 = inputTexture[iid];
    float2 x1 = inputTexture[uint2(iid.x,iid.y + NSHalf)];

    float2 wt = complexMul(W(NS,xy.y),x1);
    if(adjust == 1){
        wt *= -1;
    }
    float2 output = x0 + wt;
    if(adjust == 1){
        output *= ((xy.y + 1) % 2) * 2. - 1.;
    }
    //float2 output = inputTexture[xy];
    //inputTexture[xy] = sin(xy);
	outputTexture[xy] = output;
}
