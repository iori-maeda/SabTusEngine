#include "Header.hlsli"

struct DS_OUTPUT
{
    float4 vPosition : SV_POSITION;
};

//[domain("tri")]
//DS_OUTPUT main(
//	HS_CONSTANT_DATA_OUTPUT input,
//	float3 domain : SV_DomainLocation,
//	const OutputPatch<HS_CONTROL_POINT_OUTPUT, NUM_CONTROL_POINTS> patch)
//{
//	DS_OUTPUT Output;

//	Output.vPosition = float4(
//		patch[0].vPosition*domain.x+patch[1].vPosition*domain.y+patch[2].vPosition*domain.z,1);

//	return Output;
//}

[domain("tri")]
DS_OUTPUT main(HS_CONSTANT_DATA_OUTPUT input,
               const OutputPatch<HS_CONTROL_POINT_OUTPUT, NUM_CONTROL_POINTS> patch,
               float2 uv : SV_DomainLocation)
{
    DS_OUTPUT output;
    float3 pos =
        lerp(
            lerp(patch[0].vPosition, patch[1].vPosition, uv.x),
            lerp(patch[2].vPosition, patch[0].vPosition, uv.x),
            uv.y);

    // 波を加える（ここで「海」らしさを作る）
    float wave = sin(pos.x * 4.0) * 0.3 + cos(pos.z * 3.0) * 0.2;
    pos.y += wave;

    output.vPosition = float4(pos, 1);
    return output;
}

