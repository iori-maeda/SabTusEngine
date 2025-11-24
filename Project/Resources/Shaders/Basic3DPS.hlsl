#include "Basic3D.hlsli"

struct Material
{
    float4 Ka;
    float4 Kd;
    float4 Ks;
    float shininess;
    int32_t enableLighting;
};

struct Essential
{
    uint numLights;
};

struct LightStatus
{
    float4 color;
    float3 direction;
    float intensity;
    float3 position;
    float distance;
    float range;
    float decay;
    float cosFallOffStart;
    float cosAngle;
    uint type;
};

struct Camera
{
    float3 worldPosition;
};

ConstantBuffer<Material> gMaterial : register(b0);
ConstantBuffer<Camera> gCamera : register(b2);
ConstantBuffer<Essential> gEssential : register(b1);
StructuredBuffer<LightStatus> gLights : register(t1);

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
    
    // Base Color
    float4 ambientColor = gMaterial.Ka * texColor;
    float4 baseDiffuseColor = gMaterial.Kd * texColor;
    float4 baseSpecularColor = gMaterial.Ks;
    
    // View Direction
    float3 toEyeDir = normalize(gCamera.worldPosition - input.worldPosition);
    
    // Directional Light
    float4 directionalLightColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float NdotDirectional = 0.0f;
    
    // Blinn-Phong Reflection Model
    // Phongと違いLightの反射ベクトルではなくハーフベクトルを利用する
    float3 specularHalfVector = float3(0.0f, 0.0f, 0.0f);
    float NdotSpecular = 0.0f;
    float specularIntensity = 0.0f;
    float4 specularRefrectionColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
    
    // Point Light
    float4 pointLightColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float3 toPointLight = float3(0.0f, 0.0f, 0.0f);
    float3 pointHalfVector = float3(0.0f, 0.0f, 0.0f);
    float NdotPointLight = 0.0f;
    float specularIntensityToPoint = 0.0f;
    float pointLightDistance = 0.0f;
    float factor = 0.0f;
    float lambartIntensityToPoint = 0.0f;
    float4 attenuationPointLightColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
    
    // Spot Light
    float4 spotLightColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float3 spotLightDirectionOnSurface = float3(0.0f, 0.0f, 0.0f);
    // 角度減衰
    float cosAngle = 0.0f;
    float cosDivide = 0.0f;
  
    float fallOffFactor = 0.0f;
    // 距離減衰
    float spotDistance = 0.0f;
    float spotDistanceFactor = 0.0f;
    // 減衰後の色
    float4 attenuationSpotLightColor = float4(0.0f, 0.0f, 0.0f, 0.0f);;
    // Diffuse Intensity
    float lambartIntensityToSpot = 0.0f;
    // Specular Intensity
    float3 spotHaldfVector = float3(0.0f, 0.0f, 0.0f);
    float specularIntensityToSpot = 0.0f;
    
    
    for (uint i = 0; i < gEssential.numLights; i++)
    {
        switch (gLights[i].type)
        {
            case 0:
                // Directional Light
                directionalLightColor = gLights[i].color * gLights[i].intensity;
                NdotDirectional = saturate(dot(normalize(input.normal), -gLights[i].direction));
    
                // Blinn-Phong Reflection Model
                // Phongと違いLightの反射ベクトルではなくハーフベクトルを利用する
                specularHalfVector = normalize(-gLights[i].direction + toEyeDir);
                NdotSpecular = saturate(dot(normalize(input.normal), specularHalfVector));
                specularIntensity = pow(NdotSpecular, gMaterial.shininess);
                
                // 合成
                specularRefrectionColor += baseSpecularColor * specularIntensity * directionalLightColor;
                break;
            
            case 1:
                if (gLights[i].range < length(input.worldPosition - gLights[i].position))
                {
                    continue;
                }
                pointLightColor = gLights[i].color * gLights[i].intensity;
                toPointLight = normalize(gLights[i].position - input.worldPosition);
                pointHalfVector = normalize(toPointLight + toEyeDir);
                NdotPointLight = saturate(dot(input.normal, pointHalfVector));
                specularIntensityToPoint = pow(NdotPointLight, gMaterial.shininess);
                pointLightDistance = length(gLights[i].position - input.worldPosition);
                factor = gLights[i].decay <= 0.1 ? 0.1f : pow(saturate(-pointLightDistance / gLights[i].range + 1.0f), gLights[i].decay);
                lambartIntensityToPoint = saturate(dot(normalize(input.normal), toPointLight));
            
                // 合成
                attenuationPointLightColor += pointLightColor * factor;
                break;
            
            case 2:
                if (gLights[i].range < length(input.worldPosition - gLights[i].position))
                {
                    continue;
                }
                // Spot Light
                spotLightColor = gLights[i].color * gLights[i].intensity;
                spotLightDirectionOnSurface = normalize(input.worldPosition - gLights[i].position);
                // 角度減衰
                cosAngle = dot(spotLightDirectionOnSurface, normalize(gLights[i].direction));
                cosDivide = gLights[i].cosFallOffStart >= gLights[i].cosAngle ? gLights[i].cosFallOffStart - gLights[i].cosAngle : 0.0f;
                if (cosDivide < 0.0f)
                {
                    output.color = float4(1.0f, 0.0f, 1.0f, 1.0f);
                    return output;
                }
                fallOffFactor = saturate((cosAngle - gLights[i].cosAngle) / (gLights[i].cosFallOffStart - gLights[i].cosAngle));
                // 距離減衰
                spotDistance = length(gLights[i].position - input.worldPosition);
                spotDistanceFactor = gLights[i].decay <= 0.1 ? 0.1f : pow(saturate(1.0f - spotDistance / gLights[i].distance), gLights[i].decay);
                // Diffuse Intensity
                lambartIntensityToSpot = saturate(dot(normalize(input.normal), -spotLightDirectionOnSurface));
                // Specular Intensity
                spotHaldfVector = normalize(-spotLightDirectionOnSurface + toEyeDir);
                specularIntensityToSpot = pow(saturate(dot(input.normal, spotHaldfVector)), gMaterial.shininess);
                // 減衰後の色
                attenuationSpotLightColor += spotLightColor * spotDistanceFactor * fallOffFactor;
                break;
            
            default:
                break;
        }
    }
    
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