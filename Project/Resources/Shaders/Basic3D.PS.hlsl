#include "Basic3D.hlsli"
#include "LightStatus.hlsli"
#include "BasicLightFunction.hlsli"
#include "PBR_LightFunction.hlsli"
#include "Basic3DStructs.PS.hlsli"

ConstantBuffer<MeshMaterial> gMeshMaterial : register(b0);
ConstantBuffer<Essential> gEssential : register(b1);
ConstantBuffer<Camera> gCamera : register(b2);
ConstantBuffer<ObjectMaterial> gObjectMaterial : register(b3);
ConstantBuffer<FogStatus> gFogStatus : register(b4);

StructuredBuffer<LightStatus> gLights : register(t0);

Texture2D<float4> gTexture : register(t1);
Texture2D<float4> gNormalTexture : register(t2);
Texture2D<float4> gRoughnessTextureOrARMTexture : register(t3);
Texture2D<float4> gMetallicTexture : register(t4);
SamplerState gSampler : register(s0);

struct Output
{
    float4 color : SV_TARGET0;
};

float3 CaluclateNormalByTexture(VertexOutput vertex, float3 normalMapData)
{
    float3 normal = normalize(vertex.normal);
    float3 tangent = normalize(vertex.tangent);
    // 再直行させるための計算式
    tangent = normalize(tangent - dot(tangent, normal) * normal);
    
    float3 binormal = vertex.binormal;
    float3x3 tangentBinormalMat = float3x3(tangent, binormal, normal);
    
    float3 newNormal = (normalMapData * 2.0f - 1.0f);
    
    return normalize(mul(newNormal, tangentBinormalMat));
}

float3 CaluclateLuminance(float4 baseColor)
{
    // 彩度の取得
    float3 lumiVec = float3(0.299f, 0.587f, 0.114f);
    float luminance = dot(baseColor.rgb, lumiVec);
    return float3(luminance, luminance, luminance);
}

float4 CaluclateFogColor(FogStatus fogStatus, float4 baseColor, float4x4 viewMat, float3 worldPosition)
{
    // Camera to Vertex Length
    float3 toCamera = mul(float4(worldPosition, 1.0f), viewMat).xyz;
    float toCameraLen = length(toCamera);
    
    // grayScale
    float3 grayScale = CaluclateLuminance(baseColor);
    
    float fogFactor = exp(-pow(toCameraLen * fogStatus.density, fogStatus.power));
    
    float thresholdMax = fogStatus.thresholdEnd > fogStatus.thresholdStart ? fogStatus.thresholdEnd : fogStatus.thresholdStart;
    float fogRatio = smoothstep(fogStatus.thresholdEnd, thresholdMax, fogFactor);
    fogFactor = saturate(fogFactor);
    if (fogRatio <= 0.0f)
    {
        return float4(0.0f, 0.0f, 0.0f, 0.0f);
    }
    float3 fogColor = float3(0.3f, 0.3f, 0.3f);
    float3 desaturateColor = lerp(grayScale, baseColor.rgb, fogRatio);
    float4 resultColor;
    resultColor.rgb = lerp(fogColor, desaturateColor, fogRatio);
    resultColor.a = fogRatio;
    
    return resultColor;
}

Output main(VertexOutput input)
{
    Output output;
    output.color = float4(0.0f, 0.0f, 0.0f, 1.0f); // Initialize output color

    // BaseTexColor
    float4 albedoColor = gTexture.Sample(gSampler, input.uv);
    // PBR_TextureData
    float4 pbrTextureData = gRoughnessTextureOrARMTexture.Sample(gSampler, input.uv);
    
    // invisible discard
    if (albedoColor.a <= 0.0f)
    {
        discard;
    }
    
    // Base Color
    const float4 kBaseAmbientColor = gMeshMaterial.Ka * gObjectMaterial.color;
    const float4 kBaseDiffuseColor = gMeshMaterial.Kd * gObjectMaterial.color;
    
    // Surface Color
    const float4 kObjectAmbientColor = gObjectMaterial.useTexture ? kBaseAmbientColor * albedoColor : kBaseAmbientColor;
    const float4 kObjectDiffuseColor = gObjectMaterial.useTexture ? kBaseDiffuseColor * albedoColor : kBaseDiffuseColor;
    const float4 kObjectSpecularColor = gMeshMaterial.Ks * gObjectMaterial.color;
    
    // MeshMaterial Non Setting ? BaseAmbient tone down : use BaseAmbient
    const bool checkMaximAmbient = length(gMeshMaterial.Ka) >= length(gMeshMaterial.Kd);
    const float4 useAmbientColor = checkMaximAmbient ? float4(kObjectAmbientColor.rgb * 0.5f, 1.0f) : kBaseAmbientColor;
    
    // non Lighting
    if (gObjectMaterial.enableLighting == 0)
    {
        output.color = kObjectAmbientColor + kObjectDiffuseColor + kObjectSpecularColor;
        output.color.a = albedoColor.a;
        return output;
    }
    
    // NormalMapData
    float4 normalMapData = gNormalTexture.Sample(gSampler, input.uv);
    // NormalMap -> useNormal
    float3 caluclateNormal = CaluclateNormalByTexture(input, normalMapData.rgb);
	// Use NormalMapData ? caluclateNormal : input.normal
    const float3 kNormal = gObjectMaterial.useNormal ? caluclateNormal : normalize(input.normal);
    
    Basic_SurfaceStatus basicSurface;
    basicSurface.worldPos = input.worldPosition;
    basicSurface.normal = kNormal;
    basicSurface.ambient = useAmbientColor;
    basicSurface.diffuse = kObjectDiffuseColor;
    basicSurface.specular = kObjectSpecularColor;
    basicSurface.shininess = gMeshMaterial.shininess <= 0.0f ? 1.0f : gMeshMaterial.shininess;
    
    const float kBaseMetallic = gMeshMaterial.isUseArmTex ? pbrTextureData[gMeshMaterial.metallicChannel] : gMetallicTexture.Sample(gSampler, input.uv)[gMeshMaterial.metallicChannel];
    
    PBR_SurfaceStatus pbrSurface;
    pbrSurface.albedo = albedoColor * gObjectMaterial.color;
    pbrSurface.normal = kNormal;
    pbrSurface.roughness = saturate(pbrTextureData[gMeshMaterial.roughnessChannel] + gObjectMaterial.roughness);
    pbrSurface.metallic = saturate(kBaseMetallic + gObjectMaterial.metallic);
    pbrSurface.worldPos = input.worldPosition;
    
    // View Direction
    const float3 kToEyeDir = normalize(gCamera.worldPosition - input.worldPosition);
    
    float4 finaleColor = useAmbientColor;
    
    for (uint i = 0; i < gEssential.numLights; i++)
    {
        // 共通処理
        // 光源色
        const float4 kLightColor = gLights[i].color * gLights[i].intensity;
        
        if (gLights[i].type != DIRECTIONAL)
        {
            const float3 kToLight = gLights[i].position - input.worldPosition;
            const float kToLightLength = length(kToLight);
            if (kToLightLength > gLights[i].range)
            {
                continue;
            }
        }
        
        switch (gLights[i].type)
        {
            case DIRECTIONAL:
                {
                    float4 basicLightColor = CaluclateDirectionalLightColor(gLights[i], gCamera.worldPosition, basicSurface);
                    float4 pbrLightColor = CaluclateDirectionalLightPBR(gLights[i], gCamera.worldPosition, pbrSurface);
                    finaleColor += gObjectMaterial.caluclatePBR ? pbrLightColor : basicLightColor;
                }
                break;
            
            case POINT:
                {
                    float4 basicPointLightColor = CaluclatePointLightColor(gLights[i], gCamera.worldPosition, basicSurface);
                    float4 pbrPointLightColor = CaluclatePointLightColorPBR(gLights[i], gCamera.worldPosition, pbrSurface);
                    finaleColor += gObjectMaterial.caluclatePBR ? pbrPointLightColor : basicPointLightColor;
                }
                break;
            
            case SPOT:
                {
                    float4 basicSpotLightColor = CaluclateSpotLightColor(gLights[i], gCamera.worldPosition, basicSurface);
                    float4 pbrSpotLightColor = CaluclateSpotLightColorPBR(gLights[i], gCamera.worldPosition, pbrSurface);
                    finaleColor += gObjectMaterial.caluclatePBR ? pbrSpotLightColor : basicSpotLightColor;
                }
                break;
            
            default:
                break;
        }
    }
    
    float4 fogColor = CaluclateFogColor(gFogStatus, finaleColor, gCamera.viewMatrix, input.worldPosition);
    
    output.color = length(fogColor) > 0.0f ? fogColor : finaleColor;
    //output.color.a = pbrTextureData.a;
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