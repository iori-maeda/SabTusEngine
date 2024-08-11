
struct Material
{
	float4 color;
};

ConstantBuffer<Material> gMaterial : register(b0);

struct Output
{
	float4 color : SV_TARGET0;
};

Output main()
{
	Output output;
	output.color = gMaterial.color;
	return output;
}