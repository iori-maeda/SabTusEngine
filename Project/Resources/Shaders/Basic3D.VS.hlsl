#include "Basic3D.hlsli"

struct TransformationMatrix
{
    float4x4 wvp;
    float4x4 world;
    float4x4 worldInvederseTranspose;
};

ConstantBuffer<TransformationMatrix> gTransformationMatrix : register(b0);

struct Input
{
    float4 position : POSITION0;
    float2 uv : TEXCOORD0;
    float3 normal : NORMAL0;
    float4 tangent : TANGENT0;
};


VertexOutput main(Input input)
{
    VertexOutput output;
    output.position = mul(input.position, gTransformationMatrix.wvp);
    output.uv = input.uv;
    output.normal = normalize(mul(input.normal, (float3x3) (gTransformationMatrix.worldInvederseTranspose))); //normalize(mul(input.normal, (float3x3)gTransformationMatrix.world));
    output.tangent = normalize(mul(input.tangent.xyz, (float3x3) (gTransformationMatrix.worldInvederseTranspose)));
    output.binormal = normalize(cross(output.tangent, output.normal) * input.tangent.w);
    output.worldPosition = mul(input.position, gTransformationMatrix.world).xyz;
    return output;
}