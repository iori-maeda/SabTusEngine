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
    int numLights;
};

struct SpotLight
{
    float4 color;
    float3 position;
    float intensity;
    float3 direction;
    float distance;
    float decay;
    float cosFallOffStart;
    float cosAngle;
    int numLights;
};

struct Camera
{
    float3 worldPosition;
};

ConstantBuffer<Material> gMaterial : register(b0);
ConstantBuffer<DirectionalLight> gDirectionalLight : register(b1);
StructuredBuffer<PointLight> gPointLight : register(b3);
StructuredBuffer<SpotLight> gSpotLight : register(b4);
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
    
    float4 ambientColor = gMaterial.Ka * texColor;
    float4 baseDiffuseColor = gMaterial.Kd * texColor;
    float4 baseSpecularColor = gMaterial.Ks;
	
    // View Direction
    float3 toEyeDir = normalize(gCamera.worldPosition - input.worldPosition);
    
    // Directional Light
    float4 directionalLightColor = gDirectionalLight.color * gDirectionalLight.intensity;
    float NdotDirectional = saturate(dot(normalize(input.normal), -gDirectionalLight.direction));
    
    // Blinn-Phong Reflection Model
    // Phongと違いLightの反射ベクトルではなくハーフベクトルを利用する
    float3 specularHalfVector = normalize(-gDirectionalLight.direction + toEyeDir);
    float NdotSpecular = saturate(dot(normalize(input.normal), specularHalfVector));
    float specularIntensity = pow(NdotSpecular, gMaterial.shininess);
    float4 specularRefrectionColor = baseSpecularColor * specularIntensity * directionalLightColor;
    
    // Point Light
    float4 pointLightColor = gPointLight.color * gPointLight.intensity;
    float3 toPointLight = normalize(gPointLight.position - input.worldPosition);
    float3 pointHalfVector = normalize(toPointLight + toEyeDir);
    float NdotPointLight = saturate(dot(input.normal, pointHalfVector));
    float specularIntensityToPoint = pow(NdotPointLight, gMaterial.shininess);
    float pointLightDistance = length(gPointLight.position - input.worldPosition);
    float factor = gPointLight.decay <= 0.1 ? 0.1f : pow(saturate(-pointLightDistance / gPointLight.radius + 1.0f), gPointLight.decay);
    float lambartIntensityToPoint = saturate(dot(normalize(input.normal), toPointLight));
    float4 attenuationPointLightColor = pointLightColor * factor;
    
    // Spot Light
    float4 spotLightColor = gSpotLight.color * gSpotLight.intensity;
    float3 spotLightDirectionOnSurface = normalize(input.worldPosition - gSpotLight.position);
    // 角度減衰
    float cosAngle = dot(spotLightDirectionOnSurface, normalize(gSpotLight.direction));
    float cosDivide = gSpotLight.cosFallOffStart >= gSpotLight.cosAngle ? gSpotLight.cosFallOffStart - gSpotLight.cosAngle : 0.0f;
    if (cosDivide < 0.0f)
    {
        output.color = float4(1.0f, 0.0f, 1.0f, 1.0f);
        return output;
    }
    float fallOffFactor = saturate((cosAngle - gSpotLight.cosAngle) / (gSpotLight.cosFallOffStart - gSpotLight.cosAngle));
    // 距離減衰
    float spotDistance = length(gSpotLight.position - input.worldPosition);
    float spotDistanceFactor = gSpotLight.decay <= 0.1 ? 0.1f : pow(saturate(1.0f - spotDistance / gSpotLight.distance), gSpotLight.decay);
    // 減衰後の色
    float4 attenuationSpotLightColor = spotLightColor * spotDistanceFactor * fallOffFactor;
    // Diffuse Intensity
    float lambartIntensityToSpot = saturate(dot(normalize(input.normal), -spotLightDirectionOnSurface));
    // Specular Intensity
    float3 spotHaldfVector = normalize(-spotLightDirectionOnSurface + toEyeDir);
    float specularIntensityToSpot = pow(saturate(dot(input.normal, spotHaldfVector)), gMaterial.shininess);
    
    // Ambient
    float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    ambient.rgb = gMaterial.Ka.rgb * texColor.rgb;
    ambient.a = gMaterial.Ka.a * texColor.a;
	
    // Diffuse
    float4 diffuse = gMaterial.Kd * texColor;
    diffuse = gMaterial.enableLighting ? diffuse * directionalLightColor * NdotDirectional : diffuse;
    diffuse.a = gMaterial.Kd.a * texColor.a;
    
    // Specular
    float4 specular = float4(0.0f, 0.0f, 0.0f, 0.0f);
    specular = gMaterial.enableLighting ? specularRefrectionColor : float4(0.0f, 0.0f, 0.0f, 0.0f);
    specular.a = gMaterial.Ks.a;
    
    // Point Ligh Diffuse
    float4 pointLightDiffuseColor = baseDiffuseColor * lambartIntensityToPoint * attenuationPointLightColor;
    // Point Light Specilar
    float4 pointLightSpecularColor = baseSpecularColor * specularIntensityToPoint * attenuationPointLightColor;
    // Spot Light Diffuse
    float4 spotLightDiffuseColor = baseDiffuseColor * lambartIntensityToSpot * attenuationSpotLightColor;
    // Spot Light Specular
    float4 spotLightSpecularColor = baseSpecularColor * specularIntensityToSpot * attenuationSpotLightColor;
    
    // ADS合成
    output.color = ambient + diffuse + specular + pointLightDiffuseColor + pointLightSpecularColor + spotLightDiffuseColor + spotLightSpecularColor;
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