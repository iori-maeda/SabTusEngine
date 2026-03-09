

enum LightType
{
    DIRECTIONAL = 0,
    POINT = 1,
    SPOT = 2
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

struct ObjectStatus
{
    float3 position;
    float3 normal;
    float4 diffuseColor;
    float4 specularColor;
    float shininess;
};

float4 CaluclateDirectionalLightColor(const LightStatus light, float3 cameraPosition, ObjectStatus objStatus)
{
    // EyeVector
    const float kToEyeDir = normalize(cameraPosition - objStatus.position);
    const float3 kLightDirectionNormal = -normalize(light.direction);
    
    const float kLightColor = light.color * light.intensity;
    
    // diffuse
    const float kNdotL = saturate(dot(objStatus.normal, kLightDirectionNormal));
    const float4 kLightDiffuse = objStatus.diffuseColor * kLightColor * kNdotL;
    
    // specular
    const float3 kHalfVector = normalize(kLightDirectionNormal + kToEyeDir);
    const float kNdotH = saturate(dot(objStatus.normal, kHalfVector));            
    const float kSpeclarIntensity = pow(saturate(kNdotH), objStatus.shininess);
    const float4 kLightSpecular = objStatus.specularColor * kLightColor * kSpeclarIntensity;
                
    return kLightDiffuse + kLightSpecular;
}

float4 CaluclateDirectionalLightColorByPBR(const LightStatus light, float cameraPosition, ObjectStatus objStatus)
{
    
}