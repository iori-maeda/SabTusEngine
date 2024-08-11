
struct TransformationMatrix
{
	float4x4 wvp;
};

ConstantBuffer<TransformationMatrix> gTransformationMatrix : register(b0);

struct Output
{
	float4 position : SV_POSITION;
};

struct Input
{
	float4 position : POSITION0;
};


Output main( Input input)
{
	Output output;
	output.position = mul(input.position, gTransformationMatrix.wvp);
	return output;
}