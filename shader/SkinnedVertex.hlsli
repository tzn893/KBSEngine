struct SkinnedVertexIn{
    float3 Position : POSITION;
    float3 Normal   : NORMAL;
    float2 Uv       : TEXCOORD0;
    float3 Tangent  : TANGENT;
    uint4  boneIndices : TEXCOORD1;
    float4 boneWeights : TEXCOORD2;
};

#ifndef BONE_ARRAY_REGISTER 
#define BONE_ARRAY_REGISTER t0
#endif

StructuredBuffer<float4x4> bones : register(BONE_ARRAY_REGISTER);

struct LocalVert {
    float3 localPos;
    float3 localNormal;
    float3 localTangent;
};


LocalVert BlendLocalVertex(SkinnedVertexIn vin){
    float w0 = vin.boneWeights.x,w1 = vin.boneWeights.y,
        w2 = vin.boneWeights.z,w3 = vin.boneWeights.w;
    float4x4 b0 = vin.boneIndices.x,b1 = vin.boneIndices.y,
        b2 = vin.boneIndices.z,b3 = vin.boneIndices.w;

    float4x4 trans = bones[b0];
    float3 pos = 0.,normal = 0.,tang = 0.;
    pos += mul(trans,float4(vin.Position,1.)).xyz * w0;
    //we assume that bones doesn't contain any scaling transformation
    //so the only rotation can effect the normals and tangents
    //which means that multipling trans matrix directly on normals and tangents
    //equals multipling the inverse-transpose of the trans matrix on normals and trangents
    normal += mul((float3x3)trans,vin.Normal) * w0;
    tang += mul((float3x3)trans,vin.Tangent) * w0;

    trans = bones[b1];
    pos += mul(trans,float4(vin.Position,1.)).xyz * w1;
    normal += mul((float3x3)trans,vin.Normal) * w1;
    tang += mul((float3x3)trans,vin.Tangent) * w1;

    trans = bones[b2];
    pos += mul(trans,float4(vin.Position,1.)).xyz * w2;
    normal += mul((float3x3)trans,vin.Normal) * w2;
    tang += mul((float3x3)trans,vin.Tangent) * w2;

    trans = bones[b3];
    pos += mul(trans,float4(vin.Position,1.)).xyz * w3;
    normal += mul((float3x3)trans,vin.Normal) * w3;
    tang += mul((float3x3)trans,vin.Tangent) * w3;

    LocalVert v;
    v.localPos = pos;
    v.localNormal = normalize(normal);
    v.localTangent = normalize(tang);

    return v;
} 
