#pragma once
#include "LightStatus.hlsli"

struct Basic_SurfaceStatus
{
    float3 worldPos;
    float3 normal;
    float4 ambient;
    float4 diffuse;
    float4 specular;
    float shininess;
};

float4 CaluclateDirectionalLightColor(const LightStatus light, float3 cameraPosition, Basic_SurfaceStatus surface)
{
    // EyeVector
    const float3 kToEyeDir = normalize(cameraPosition - surface.worldPos);
    const float3 kLightDirectionNormal = -normalize(light.direction);
    
    const float4 kLightColor = light.color * light.intensity;
    
    // diffuse
    const float kNdotL = saturate(dot(surface.normal, kLightDirectionNormal));
    const float4 kLightDiffuse = surface.diffuse * kLightColor * kNdotL;
    
    // specular
    const float3 kHalfVector = normalize(kLightDirectionNormal + kToEyeDir);
    const float kNdotH = saturate(dot(surface.normal, kHalfVector));
    const float kSpeclarIntensity = pow(saturate(kNdotH), surface.shininess);
    const float4 kLightSpecular = surface.specular * kLightColor * kSpeclarIntensity;
                
    return kLightDiffuse; // + kLightSpecular;
}

float4 CaluclatePointLightColor(const LightStatus light, float3 cameraPosition, Basic_SurfaceStatus surface)
{
    // EyeVector
    const float3 kToEyeDir = normalize(cameraPosition - surface.worldPos);
    const float4 kLightColor = light.color * light.intensity;
    
    const float3 kToLight = light.position - surface.worldPos;
    const float kToLightLength = length(kToLight);
    
                
    const float3 kToLightNormal = normalize(kToLight);
    const float kNdotL = saturate(dot(surface.normal, kToLightNormal));
    const float3 kHalfVector = normalize(kToLightNormal + kToEyeDir);
    const float kNdotH = saturate(dot(surface.normal, kHalfVector));
    const float kSpeclarIntensity = pow(saturate(kNdotH), surface.shininess);
                    // 距離減衰
    const float kRange = max(light.range, 0.0001f);
    const float kAttenuationFactor = pow(saturate(1.0f - kToLightLength / kRange), light.decay);
                  
    const float4 kLightDiffuse = surface.diffuse * kLightColor * kNdotL * kAttenuationFactor;
    const float4 kLightSpecular = surface.specular * kLightColor * kSpeclarIntensity * kAttenuationFactor;
               
    return kLightDiffuse + kLightSpecular;
}

float4 CaluclateSpotLightColor(const LightStatus light, float3 cameraPosition, Basic_SurfaceStatus surface)
{
    const float3 kToLight = light.position - surface.worldPos;
    const float kToLightLength = length(kToLight);
     
    const float3 kToEyeDir = normalize(cameraPosition - surface.worldPos);
    const float4 kLightColor = light.color * light.intensity;
    
    const float3 kLightDirectionNormal = -normalize(light.direction);
    const float3 kToLightNormal = normalize(kToLight);
    const float kNdotL = saturate(dot(surface.normal, kToLightNormal));
    const float3 kHalfVector = normalize(kToLightNormal + kToEyeDir);
    const float kNdotH = saturate(dot(surface.normal, kHalfVector));
    const float kSpeclarIntensity = pow(saturate(kNdotH), surface.shininess);
                    // 距離減衰
    const float kRange = max(light.range, 0.0001f);
    const float kAttenuationFactor = pow(saturate(1.0f - kToLightLength / kRange), light.decay);
                    // 範囲減衰
    const float kCosAngle = dot(kToLightNormal, kLightDirectionNormal);
    const float kCosAngleDiff = max(light.cosFallOffStart - light.cosAngle, 0.0001f);
    const float kFalloffFactor = saturate((kCosAngle - light.cosAngle) / kCosAngleDiff);
                
    const float4 kLightDiffuse = surface.diffuse * kLightColor * kNdotL * kAttenuationFactor * kFalloffFactor;
    const float4 kLightSpecular = surface.specular * kLightColor * kSpeclarIntensity * kAttenuationFactor * kFalloffFactor;
               
    return kLightDiffuse + kLightSpecular;
}
