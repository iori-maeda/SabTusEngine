#include "Basic3D.hlsli"

struct TransformationMatrix
{
	float4x4 wvp;
	float4x4 world;
};

ConstantBuffer<TransformationMatrix> gTransformationMatrix : register(b0);

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
	output.normal = normalize(mul(input.normal, (float3x3)gTransformationMatrix.world));
	return output;
}