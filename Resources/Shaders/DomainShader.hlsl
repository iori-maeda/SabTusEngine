#include "Header.hlsli"

[domain("tri")]
DS_OUTPUT main(
	HS_CONSTANT_DATA_OUTPUT input,
	float3 domain : SV_DomainLocation,
	const OutputPatch<HS_CONTROL_POINT_OUTPUT, NUM_CONTROL_POINTS> patch)
{
    DS_OUTPUT output;

    
    float4 worldPos = float4(
		patch[0].vPosition * domain.x 
    + patch[1].vPosition * domain.y 
    + patch[2].vPosition * domain.z, 
    1);
    
    float3 normal =
    patch[0].normal * domain.x 
    + patch[1].normal * domain.y 
    + patch[2].normal * domain.z;
    
    float2 uv = patch[0].uv * domain.x + patch[1].uv * domain.y + patch[2].uv * domain.z;
    
    output.uv = uv;
    output.normal = normalize(normal);
    float3 wave = sin(length(worldPos) * gWaveBuffer.frequency + gWaveBuffer.time) * gWaveBuffer.amplitude;
   
    worldPos.xyz += wave;
    output.vPosition = mul(worldPos, gTransformationMatrix.wvp);

    return output;
}