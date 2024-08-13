#include "Basic3D.hlsli"

struct Material
{
	float4 color;
};

ConstantBuffer<Material> gMaterial : register(b0);

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct Output
{
	float4 color : SV_TARGET0;
};

Output main(VertexOutput input)
{
	Output output;
	output.color = gTexture.Sample(gSampler, input.uv) * gMaterial.color;
	return output;
}