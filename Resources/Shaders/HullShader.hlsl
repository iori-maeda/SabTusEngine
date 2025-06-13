#include "Header.hlsli"

HS_CONSTANT_DATA_OUTPUT CalcHSPatchConstants(
	InputPatch<HS_CONTROL_POINT_OUTPUT, NUM_CONTROL_POINTS> ip,
	uint PatchID : SV_PrimitiveID)
{
    HS_CONSTANT_DATA_OUTPUT output;

    output.EdgeTessFactor[0] = 5;
    output.EdgeTessFactor[1] = 5;
    output.EdgeTessFactor[2] = 5;
    output.InsideTessFactor = 15;

    return output;
}

[domain("tri")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("CalcHSPatchConstants")]
HS_CONTROL_POINT_OUTPUT main(
	InputPatch<VertexOutput, NUM_CONTROL_POINTS> ip,
	uint i : SV_OutputControlPointID)
{
    HS_CONTROL_POINT_OUTPUT output;

    output.vPosition = ip[i].worldPos;
    output.normal = ip[i].normal;
    output.uv = ip[i].uv;

    return output;
}
