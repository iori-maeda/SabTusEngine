#include "Basic3D.hlsli"

struct Material
{
	float4 Ka;
	float4 Kd;
	float4 Ks;
    float shininess;
	int32_t enableLighting;
};

struct DirectionalLight
{
	float4 color;
	float3 direction;
	float intensity;
};


struct Camera
{
    float3 worldPosition;
};

ConstantBuffer<Material> gMaterial : register(b0);
ConstantBuffer<DirectionalLight> gDirectionalLight : register(b1);
ConstantBuffer<Camera> gCamera : register(b2);

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

	float reflectIntensity= dot(normalize(input.normal), -gDirectionalLight.direction);
	float halfLambert = pow(reflectIntensity * 0.5 + 0.5, 2.0f);
	float4 lightColor = gDirectionalLight.color * halfLambert * gDirectionalLight.intensity;

	float4 texColor = gTexture.Sample(gSampler, input.uv);
	
    if (texColor.a <= 0.0)
    {
        discard;
    }
	

    float4 ambient = float4(0.0, 0.0, 0.0, 0.0);
    ambient.rgb = gMaterial.enableLighting ? gMaterial.Ka.rgb * texColor.rgb : float3(0.0, 0.0, 0.0);
    ambient.a = gMaterial.enableLighting ? gMaterial.Ka.a * texColor.a : 0.0;
	
	float4 diffuse = texColor;
	diffuse.rgb = gMaterial.Kd.rgb * texColor.rgb;
	diffuse.a = gMaterial.Kd.a * texColor.a;
	
    float4 specular = float4(0.0, 0.0, 0.0, 0.0);
    float3 toEyeDir = normalize(gCamera.worldPosition - input.worldPosition);
    float3 reflectDir = reflect(gDirectionalLight.direction, normalize(input.normal));
    float3 halkfVector = normalize(toEyeDir - gDirectionalLight.direction);
	float NdotH = saturate(dot(normalize(input.normal), halkfVector));
    //float RdotE = dot(reflectDir, toEyeDir);
    float RdotE = NdotH;
	float specularIntensity = pow(saturate(RdotE), gMaterial.shininess);
	specular.rgb = gMaterial.enableLighting ?  specularIntensity * lightColor.rgb : float3(0.0, 0.0, 0.0);
    specular.a = gMaterial.enableLighting ? specularIntensity * lightColor.a : 0.0;

	output.color = ambient;
	output.color += gMaterial.enableLighting ? diffuse * lightColor : diffuse;
	output.color += specular;
    output.color.a = texColor.a; // テクスチャのアルファを使用
	
	// 半透明オブジェクトが消えるので注意
    //if (output.color.a <= 0.5f)
    //{
    //    discard;
    //}
	return output;
}