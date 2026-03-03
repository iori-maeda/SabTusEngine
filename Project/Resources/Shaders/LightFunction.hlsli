
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

//float4 CreateDirectionalLightColor(const LightStatus light)
//{
//    const float3 kLightDirectionNormal = -normalize(light.direction);
//    const float kNdotL = saturate(dot(normal, kLightDirectionNormal));
//    const float3 kHalfVector = normalize(kLightDirectionNormal + kToEyeDir);
//    const float kNdotH = saturate(dot(normal, kHalfVector));
                    
//    const float kSpeclarIntensity = pow(saturate(kNdotH), gMeshMaterial.shininess);
//    const float4 kLightDiffuse = kObjectDiffuseColor * kLightColor * kNdotL;
//    const float4 kLightSpecular = kObjectSpecularColor * kLightColor * kSpeclarIntensity;
                
//    finaleColor += kLightDiffuse + kLightSpecular;
//}