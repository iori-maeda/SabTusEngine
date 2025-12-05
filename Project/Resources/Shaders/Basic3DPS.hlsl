#include "Basic3D.hlsli"

enum LightType
{
    DIRECTIONAL = 0,
    POINT = 1,
    SPOT = 2
};

struct MeshMaterial
{
    float4 Ka;
    float4 Kd;
    float4 Ks;
    float shininess;
};

struct Essential
{
    uint numLights;
};

struct ObjectMaterial
{
    float4 color;
    uint32_t enableLighting;
};

struct LightStatus
{
    float4 color;
    float3 direction;
    float intensity;
    float3 position;
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

ConstantBuffer<MeshMaterial> gMeshMaterial : register(b0);
ConstantBuffer<Essential> gEssential : register(b1);
ConstantBuffer<Camera> gCamera : register(b2);
ConstantBuffer<ObjectMaterial> gObjectMaterial : register(b3);

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
    const float4 kObjectAmbientColor = gMeshMaterial.Ka * texColor * gObjectMaterial.color;
    const float4 kObjectDiffuseColor = gMeshMaterial.Kd * texColor * gObjectMaterial.color;
    const float4 kObjectSpecularColor = gMeshMaterial.Ks * gObjectMaterial.color;
    
    if (gObjectMaterial.enableLighting == 0)
    {
        output.color = kObjectAmbientColor + kObjectDiffuseColor + kObjectSpecularColor;
        output.color.a = texColor.a;
        return output;
    }
    
    // View Direction
    const float3 kToEyeDir = normalize(gCamera.worldPosition - input.worldPosition);
    
    const float3 kNormal = normalize(input.normal);
    float4 finaleColor = kObjectAmbientColor;
    
    for (uint i = 0; i < gEssential.numLights; i++)
    {
        // 共通処理
        // 光源色
        const float4 kLightColor = gLights[i].color * gLights[i].intensity;
        
        switch (gLights[i].type)
        {
            case DIRECTIONAL:
                {
                    const float3 kLightDirectionNormal = -normalize(gLights[i].direction);
                    const float kNdotL = saturate(dot(kNormal, kLightDirectionNormal));
                    const float3 kHalfVector = normalize(kLightDirectionNormal + kToEyeDir);
                    const float kNdotH = saturate(dot(kNormal, kHalfVector));
                    
                    const float kSpeclarIntensity = pow(saturate(kNdotH), gMeshMaterial.shininess);
                    const float4 kLightDiffuse = kObjectDiffuseColor * kLightColor * kNdotL;
                    const float4 kLightSpecular = kObjectSpecularColor * kLightColor * kSpeclarIntensity;
                
                    finaleColor += kLightDiffuse + kLightSpecular;
                }
                break;
            
            case POINT:
                {
                    const float3 kToLight = gLights[i].position - input.worldPosition;
                    const float kToLightLength = length(kToLight);
                    if (kToLightLength > gLights[i].range)
                    {
                        continue;
                    }
                
                    const float3 kToLightNormal = normalize(kToLight);
                    const float kNdotL = saturate(dot(kNormal, kToLightNormal));
                    const float3 kHalfVector = normalize(kToLightNormal + kToEyeDir);
                    const float kNdotH = saturate(dot(kNormal, kHalfVector));
                    const float kSpeclarIntensity = pow(saturate(kNdotH), gMeshMaterial.shininess);
                    // 距離減衰
                    const float kRange = max(gLights[i].range, 0.0001f);
                    const float kAttenuationFactor = pow(saturate(1.0f - kToLightLength / kRange), gLights[i].decay);
                  
                    const float4 kLightDiffuse = kObjectDiffuseColor * kLightColor * kNdotL * kAttenuationFactor;
                    const float4 kLightSpecular = kObjectSpecularColor * kLightColor * kSpeclarIntensity * kAttenuationFactor;
               
                    finaleColor += kLightDiffuse + kLightSpecular;
                }
                break;
            
            case SPOT:
                {
                    const float3 kToLight = gLights[i].position - input.worldPosition;
                    const float kToLightLength = length(kToLight);
                    if (kToLightLength > gLights[i].range)
                    {
                        continue;
                    }
                
                    const float3 kLightDirectionNormal = -normalize(gLights[i].direction);
                    const float3 kToLightNormal = normalize(kToLight);
                    const float kNdotL = saturate(dot(kNormal, kToLightNormal));
                    const float3 kHalfVector = normalize(kToLightNormal + kToEyeDir);
                    const float kNdotH = saturate(dot(kNormal, kHalfVector));
                    const float kSpeclarIntensity = pow(saturate(kNdotH), gMeshMaterial.shininess);
                    // 距離減衰
                    const float kRange = max(gLights[i].range, 0.0001f);
                    const float kAttenuationFactor = pow(saturate(1.0f - kToLightLength / kRange), gLights[i].decay);
                    // 範囲減衰
                    const float kCosAngle = dot(kToLightNormal, kLightDirectionNormal);
                    const float kCosAngleDiff = max(gLights[i].cosFallOffStart - gLights[i].cosAngle, 0.0001f);
                    const float kFalloffFactor = saturate((kCosAngle - gLights[i].cosAngle) / kCosAngleDiff);
                
                    const float4 kLightDiffuse = kObjectDiffuseColor * kLightColor * kNdotL * kAttenuationFactor * kFalloffFactor;
                    const float4 kLightSpecular = kObjectSpecularColor * kLightColor * kSpeclarIntensity * kAttenuationFactor * kFalloffFactor;
               
                    finaleColor += kLightDiffuse + kLightSpecular;
                }
                break;
            
            default:
                break;
        }
    }
    
    output.color = finaleColor;
    output.color.a = texColor.a;
    return output;
}

// 半透明オブジェクトが消えるので注意
//if (output.color.a <= 0.5f)
//{
//    discard;
//}

// Phong Reflection Model
//float3 reflectDir = reflect(gDirectionalLight.direction, normalize(input.normal));
//float RdotE = dot(reflectDir, toEyeDir);
//float specularIntensity = pow(saturate(RdotE), gMaterial.shininess);