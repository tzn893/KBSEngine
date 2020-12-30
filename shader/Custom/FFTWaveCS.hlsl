cbuffer WaveConstant : register(b0){
    float4 WindAndSeed;		//风和随机种子 xy为风, zw为两个随机种子

    int N;					//fft纹理大小
    float OceanLength;		//海洋长度
    float A;				//phillips谱参数，影响波浪高度
    float Time;				//时间
    int Ns;					//Ns = pow(2,m-1); m为第几阶段
    float Lambda;			//偏移影响
    float HeightScale;		//高度影响
    float BubblesScale;	    //泡沫强度
    float BubblesThreshold; //泡沫阈值
    int   Ended;            //fft 过程是否到最后阶段

    //float2 MeshLength;
};

//static uint rngState;           //随机
static const float PI = 3.1415926;
static const float g = 9.81;

RWTexture2D<float2> HeightMap : register(u0);
RWTexture2D<float2> GradientX : register(u1);
RWTexture2D<float2> GradientZ : register(u2);
RWTexture2D<float2> GaussianMap : register(u3);

RWTexture2D<float2> FFTInput : register(u4);
RWTexture2D<float2> FFTOutput : register(u5);

//计算弥散
float dispersion(float2 k){
    return sqrt(g * length(k));
}

//Donelan-Banner方向拓展
float DonelanBannerDirectionalSpreading(float2 k){
    float betaS;
    float omegap = 0.855f * g / length(WindAndSeed.xy);
    float ratio = dispersion(k) / omegap;

    if (ratio < 0.95f)
    {
        betaS = 2.61f * pow(ratio, 1.3f);
    }
    if(ratio >= 0.95f && ratio < 1.6f)
    {
        betaS = 2.28f * pow(ratio, -1.3f);
    }
    if(ratio > 1.6f)
    {
        float epsilon = -0.4f + 0.8393f * exp(-0.567f * log(ratio * ratio));
        betaS = pow(10, epsilon);
    }
    float theta = atan2(k.y, k.x) - atan2(WindAndSeed.y, WindAndSeed.x);

    return betaS / max(1e-7f, 2.0f * tanh(betaS * PI) * pow(cosh(betaS * theta), 2));
}
//正余弦平方方向拓展
float PositiveCosineSquaredDirectionalSpreading(float2 k){
    float theta = atan2(k.y, k.x) - atan2(WindAndSeed.y, WindAndSeed.x);
    if (theta > - PI / 2.0f && theta < PI / 2.0f)
    {
        return 2.0f / PI * pow(cos(theta), 2);
    }
    else
    {
        return 0;
    }
}
//计算phillips谱
float phillips(float2 k){
    float kLength = length(k);
    kLength = max(0.001f, kLength);
    // kLength = 1;
    float kLength2 = kLength * kLength;
    float kLength4 = kLength2 * kLength2;

    float windLength = length(WindAndSeed.xy);
    float  l = windLength * windLength / g;
    float l2 = l * l;

    float damping = 0.001f;
    float L2 = l2 * damping * damping;

    //phillips谱
    return  A * exp(-1.0f / (kLength2 * l2)) / kLength4 * exp(-kLength2 * L2);
}

//复数相乘
float2 complexMultiply(float2 c1, float2 c2){
    return float2(c1.x * c2.x - c1.y * c2.y,
    c1.x * c2.y + c1.y * c2.x);
}

float2 KTrans(uint2 pos){
    return 2 * PI * pos / N - PI;
}

float2 complexExp(float w){
    return float2(cos(w),sin(w));
}

[numthreads(16,16,1)]
void GenerateSpectrums(uint3 id : SV_DISPATCHTHREADID){
    uint2 pos = id.xy;
    float2 k = KTrans(pos);
    float2 r = GaussianMap[pos];

    float2 h0 = r * sqrt( abs(phillips(k) * DonelanBannerDirectionalSpreading(k)) * .5);
    float2 h0conj = r * sqrt(abs(phillips(-k) * DonelanBannerDirectionalSpreading(-k)) * .5);
    h0conj.y *= -1;

    float wt = dispersion(k) * Time;

    h0 = complexMultiply(complexExp(wt),h0);
    h0conj = complexMultiply(complexExp(-wt),h0conj);

    float2 ht = h0 + h0conj;

    HeightMap[pos] = ht;
    k = k / max(1e-3f,length(k));
    GradientX[pos] = float2(complexMultiply(ht,float2(0,-k.x)));
    GradientZ[pos] = float2(complexMultiply(ht,float2(0,-k.y)));
}




[numthreads(16,16,1)]
void FFTVertical(uint3 id : SV_DISPATCHTHREADID){
    uint NsFull = Ns * 2;

    uint2 oid = id.xy;
    uint2 iid = oid;
    iid.y = floor(iid.y / NsFull) * Ns + iid.y % Ns;

    uint2 iid1 = iid + uint2(0,N * .5f);
    float2 x0 = FFTInput[iid];
    float2 x1 = FFTInput[iid1];

    float wAngle = 2 * PI * id.y / NsFull;
    float2 w0 = float2(cos(wAngle),sin(wAngle));
    if(Ended){
        w0 *= -1.f;
    }

    float2 output = complexMultiply(w0,x1) + x0;

    if(Ended){
        int v = int(id.y - N * .5);
        output *= ((v + 1)% 2) * 1. + (v % 2) * (-1.);
    }

    FFTOutput[oid] = output;
}


[numthreads(16,16,1)]
void FFTHorizontal(uint3 id : SV_DISPATCHTHREADID){
    uint NsFull = Ns * 2;

    uint2 oid = id.xy;
    uint2 iid = oid;
    iid.x = floor(iid.x / NsFull) * Ns + iid.x % Ns;

    uint2 iid1 = iid + uint2(N * .5f,0);
    float2 x0 = FFTInput[iid];
    float2 x1 = FFTInput[iid1];

    float wAngle = 2 * PI * id.x / NsFull;
    float2 w0 = float2(cos(wAngle),sin(wAngle));
    if(Ended){
        w0 *= -1.f;
    }

    float2 output = complexMultiply(w0,x1) + x0;

    if(Ended){
       int v = int(id.x - N * .5);
       output *= ((v + 1)% 2) * 1. + (v % 2) * (-1.);
    }

    FFTOutput[id.xy] = output;
}

float3 gradient(uint2 pos){
    return float3(length(GradientX[pos]) / (N * N) * HeightScale,
    length(HeightMap[pos]) / (N * N) * HeightScale,
    length(GradientZ[pos]) / (N * N) * HeightScale) ;
}

[numthreads(16,16,1)]
void GenerateHeightNormal(uint3 id : SV_DISPATCHTHREADID){
    uint2 pos = id.xy;
   
    //GradientX 会作为法线贴图输出 GradientY 会作为泡沐贴图输出
    /*float3 gright = gradient(float2((pos.x + 1 + N) % N,pos.y));
    float3 gleft  = gradient(float2((pos.x - 1 + N) % N,pos.y));
    float3 gup    = gradient(float2(pos.x,(pos.y + 1 + N) % N));
    float3 gdown  = gradient(float2(pos.x,(pos.y - 1 + N) % N));

    float gridLen = OceanLength / (N - 1.f);
    float3 pright = float3(gright.x + gridLen,gright.yz);
    float3 pleft = float3(gleft.x - gridLen,gleft.yz);
    float3 pup = float3(gup.xy,gup.z + gridLen);
    float3 pdown = float3(gdown.xy,gdown.z - gridLen);*/

    float3 gp = gradient(pos);

    float3 normal = normalize(float3(gp.x,1.,gp.z));
    float height = gp.y;

    HeightMap[pos] = height;
    GradientX[pos] = normal.xz;
}