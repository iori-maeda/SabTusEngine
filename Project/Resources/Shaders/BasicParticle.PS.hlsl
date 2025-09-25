#include "BasicParticle.hlsli"

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
    output.color = float4(0.0, 0.0, 0.0, 1.0); // Initialize output color

    float4 texColor = gTexture.Sample(gSampler, input.uv);
	
    output.color = texColor * gMaterial.color;
	
    // 半透明オブジェクトが消えるので注意
    if (output.color.a == 0.0f)
    {
        discard;
    }
    return output;
}