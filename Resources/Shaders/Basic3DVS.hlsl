#include "Basic3D.hlsli"

struct TransformationMatrix
{
	float4x4 wvp;
};

ConstantBuffer<TransformationMatrix> gTransformationMatrix : register(b0);

struct Input
{
	float4 position : POSITION0;
	float2 uv : TEXCOORD0;
};


VertexOutput main(Input input)
{
	VertexOutput output;
	output.position = mul(input.position, gTransformationMatrix.wvp);
	output.uv = input.uv;
	return output;
}