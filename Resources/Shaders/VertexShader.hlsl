#include "Header.hlsli"

struct Input
{
    float4 position : POSITION0;
    float2 uv : TEXCOORD0;
    float3 normal : NORMAL0;
};


VertexOutput main(Input input)
{
    VertexOutput output;
    output.position = mul(input.position, gTransformationMatrix.wvp);
    output.uv = input.uv;
    output.normal = normalize(mul(input.normal, (float3x3) gTransformationMatrix.world));
    output.worldPos = mul(input.position, gTransformationMatrix.world).xyz;
   
    return output;
}