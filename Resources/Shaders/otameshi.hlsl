#include "Basic3D.hlsli"

struct Material
{
    float4 Ka;
    float4 Kd;
    float4 Ks;
    int32_t enableLighting;
};

struct DirectionalLight
{
    float4 color;
    float3 direction;
    float intensity;
};

ConstantBuffer<Material> gMaterial : register(b0);
ConstantBuffer<DirectionalLight> gDirectionalLight : register(b1);

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct Output
{
    float4 color : SV_TARGET0;
};

Output main(VertexOutput input)
{
    Output output;

    float reflectIntensity = dot(normalize(input.normal), -gDirectionalLight.direction);
    float halfLambert = pow(reflectIntensity * 0.5 + 0.5, 2.0f);
    float4 lightColor = gDirectionalLight.color * halfLambert * gDirectionalLight.intensity;

    float4 texColor = gTexture.Sample(gSampler, input.uv);

    float4 ambient = gMaterial.enableLighting ? gMaterial.Ka * texColor : float4(0.0, 0.0, 0.0, 0.0);

    float4 diffuse = gMaterial.Kd * texColor;
    diffuse = gMaterial.enableLighting ? diffuse * lightColor : diffuse;
    
    output.color = ambient;
    output.color += diffuse;
	
    return output;
}