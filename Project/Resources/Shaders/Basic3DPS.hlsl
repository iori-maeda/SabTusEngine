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

struct PointLight
{
    float4 color;
    float3 position;
    float intensity;
    float radius;
    float decay;
};

struct Camera
{
    float3 worldPosition;
};

ConstantBuffer<Material> gMaterial : register(b0);
ConstantBuffer<DirectionalLight> gDirectionalLight : register(b1);
ConstantBuffer<PointLight> gPointLight : register(b3);
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
    output.color = float4(0.0f, 0.0f, 0.0f, 1.0f); // Initialize output color

    float4 texColor = gTexture.Sample(gSampler, input.uv);
	
    if (texColor.a <= 0.0f)
    {
        discard;
    }
	
    // Directional Light Color
    float4 directionalLightColor = gDirectionalLight.color * gDirectionalLight.intensity;
    
    // NdotDirectional
    float NdotDirectional = saturate(dot(normalize(input.normal), -gDirectionalLight.direction));
    
    // Blinn-Phong Reflection Model
    // Phongと違いLightの反射ベクトルではなくハーフベクトルを利用する
    float3 toEyeDir = normalize(gCamera.worldPosition - input.worldPosition);
    float3 halfVector = normalize(-gDirectionalLight.direction + toEyeDir);
    float NdotH = saturate(dot(normalize(input.normal), halfVector));
    float specularIntensity = pow(NdotH, gMaterial.shininess);
    float4 specularRefrectionColor = gMaterial.Ks * specularIntensity * directionalLightColor;
    
    // Point Light
    float4 pointLightColor = gPointLight.color * gPointLight.intensity;
    float3 toPointLight = normalize(gPointLight.position - input.worldPosition);
    float3 halfVectorToPoint = normalize(toPointLight + toEyeDir);
    float NdotPointLight = saturate(dot(input.normal, halfVectorToPoint));
    float specularIntensityToPoint = pow(NdotPointLight, gMaterial.shininess);
    float distance = length(gPointLight.position - input.worldPosition);
    float factor = gPointLight.decay <= 0.1 ? 0.1f : pow(saturate(-distance / gPointLight.radius + 1.0f), gPointLight.decay);
    float lambartIntensityToPoint = saturate(dot(normalize(input.normal), toPointLight));
    float4 attenuationPointLightColor = pointLightColor * factor;
    
     // Ambient
    float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    ambient.rgb = gMaterial.Ka.rgb * texColor.rgb;
    ambient.a = gMaterial.Ka.a * texColor.a;
	
    //Diffuse
    float4 diffuse = gMaterial.Kd * texColor;
    diffuse = gMaterial.enableLighting ? diffuse * directionalLightColor * NdotDirectional : diffuse;
    diffuse.a = gMaterial.Kd.a * texColor.a;
    
    // Specular
    float4 specular = float4(0.0f, 0.0f, 0.0f, 0.0f);
    specular = gMaterial.enableLighting ? specularRefrectionColor : float4(0.0f, 0.0f, 0.0f, 0.0f);
    specular.a = gMaterial.Ks.a;
    
    // Point Ligh Diffuse
    float4 pointLightDiffuseColor = gMaterial.Kd * lambartIntensityToPoint * attenuationPointLightColor;
    // Point Light Specilar
    float4 pointLightSpecularColor = gMaterial.Ks * specularIntensityToPoint * attenuationPointLightColor;
    
    // ADS合成
    output.color = ambient + diffuse + specular + pointLightDiffuseColor + pointLightSpecularColor;
    output.color.a = texColor.a;
    return output;
}

    // HalfLambart
    //float halfLambert = pow(NdotDirectional * 0.5 + 0.5, 2.0f);

    // 半透明オブジェクトが消えるので注意
    //if (output.color.a <= 0.5f)
    //{
    //    discard;
    //}

    // Phong Reflection Model
    //float3 reflectDir = reflect(gDirectionalLight.direction, normalize(input.normal));
    //float RdotE = dot(reflectDir, toEyeDir);
    //float specularIntensity = pow(saturate(RdotE), gMaterial.shininess);