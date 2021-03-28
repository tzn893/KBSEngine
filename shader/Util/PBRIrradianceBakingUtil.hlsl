// #ifndef SAMPLE_NUM
// #define SAMPLE_NUM 20
// #endif

#define PI 3.1415926

//#define SAMPLE_NUM2_INV 1.f / (SAMPLE_NUM * SAMPLE_NUM)



static const float4 quads[6] =
{
    float4(-1.0f, -1.0f, 0.f,  1.f),
    float4( 1.0f,  1.0f, 0.f,  1.f),
    float4( 1.0f, -1.0f, 0.f,  1.f),
    float4(-1.0f,  1.0f, 0.f,  1.f),
    float4(-1.0f, -1.0f, 0.f,  1.f),
    float4( 1.0f,  1.0f, 0.f,  1.f)
};

/*
cbuffer Proj : register(b0){
    float4x4 Mright;
    float4x4 Mleft ;
    float4x4 Mup   ;
    float4x4 Mdown ;
    float4x4 Mfront;
    float4x4 Mback ;
};
*/

cbuffer Proj : register(b0){
    float4x4 proj;
};

TextureCube srcTex : register(t0);
SamplerState sp  : register(s0);

struct Vout{
    float3 Pos : TEXCOORD0;
    float4 NDC : SV_POSITION;
};

/*
struct Vout{
    float3 Pright : TEXCOORD0;
    float3 Pleft  : TEXCOORD1;
    float3 Pup    : TEXCOORD2;
    float3 Pdown  : TEXCOORD3;
    float3 Pfront : TEXCOORD4;
    float3 Pback  : TEXCOORD5;
    float4 NDC    : SV_POSITION;
};

struct Pout{
    float4 pright : SV_TARGET0;
    float4 pleft  : SV_TARGET1;
    float4 pup    : SV_TARGET2;
    float4 pdown  : SV_TARGET3;
    float4 pfront : SV_TARGET4;
    float4 pback  : SV_TARGET5;
};
*/
Vout VS(uint vid : SV_VERTEXID){ 
	Vout v;
    v.NDC = quads[vid];
    v.Pos = mul(proj,quads[vid]).xyz;
    // v.Pright = mul(Mright,quads[vid]).xyz;
    // v.Pleft  = mul(Mleft ,quads[vid]).xyz;
    // v.Pdown  = mul(Mdown ,quads[vid]).xyz;
    // v.Pup    = mul(Mup   ,quads[vid]).xyz;
    // v.Pfront = mul(Mfront,quads[vid]).xyz;
    // v.Pback  = mul(Mback ,quads[vid]).xyz;
    return v;
}

float4 SampleInSemiSphere(float3 normal){
    float3 up    = float3(0.,1.,0.);
    float3 right   = normalize(cross(up,normal));
    up = normalize(cross(normal,right));

    float3 result = 0.;

    float sampleDelta = .025f;
    float sampleCount = 0.f;


    for(float phi = 0.f;phi < 2 * PI;phi += sampleDelta){
        //float phi = 2 * PI / SAMPLE_NUM * (float)n1;
        for(float theta = 0.f;theta < PI * .5;theta += sampleDelta){
            //float theta = PI * .5 / SAMPLE_NUM * (float)n2;
            float3 dir = float3(cos(phi) * sin(theta),sin(phi) * sin(theta),cos(theta));
            dir = right * dir.x + up * dir.y + normal * dir.z;
            result += srcTex.Sample(sp,dir).rgb * sin(theta * 2)  * .5;

            sampleCount += 1.f;
        }
    }

    result *= PI / sampleCount;

    return float4(result,1.);
}

float4 PS(Vout v) : SV_TARGET{
    return SampleInSemiSphere(normalize(v.Pos));
}

/*
Pout PS(Vout v) : SV_TARGET{
    Pout p;
    p.pright = SampleInHalfSphere(normalize(v.Pright));
    p.pleft  = SampleInHalfSphere(normalize(v.Pleft ));
    p.pdown  = SampleInHalfSphere(normalize(v.Pdown));
    p.pup    = SampleInHalfSphere(normalize(v.Pup));
    p.pfront = SampleInHalfSphere(normalize(v.Pfront));
    p.pback  = SampleInHalfSphere(normalize(v.Pback));
    return p;
}
*/