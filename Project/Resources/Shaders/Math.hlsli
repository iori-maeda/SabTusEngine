#pragma once
float4x4 TranslateMat(float3 position)
{
    float4x4 translateMat =
    {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        position.x, position.y, position.z, 1.0f
    };
    return translateMat;
}

float4x4 ScaleMatrix(float3 scale)
{
    float4x4 scaleMat =
    {
        scale.x, 0.0f, 0.0f, 0.0f,
        0.0f, scale.y, 0.0f, 0.0f,
        0.0f, 0.0f, scale.y, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
    return scaleMat;
}

float4x4 RotateMatrix(float3 rotation)
{
    // X Rotate 
    float sinX = sin(rotation.x);
    float cosX = cos(rotation.x);
    float4x4 rotateX =
    {
        1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, cosX, sinX, 0.0f,
	0.0f, -sinX, cosX, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f,
    };

    // Y Rotate
    float sinY = sin(rotation.y);
    float cosY = cos(rotation.y);
    float4x4 rotateY =
    {
        cosY, 0.0f, -sinY, 0.0f,
	 0.0f, 1.0f, 0.0f, 0.0f,
	  sinY, 0.0f, cosY, 0.0f,
	 0.0f, 0.0f, 0.0f, 1.0f,
    };

    // Z Rotate
    float sinZ = sin(rotation.z);
    float cosZ = cos(rotation.z);
    float4x4 rotateZ =
    {
        cosZ, sinZ, 0.0f, 0.0f,
	 -sinZ, cosZ, 0.0f, 0.0f,
	 0.0f, 0.0f, 1.0f, 0.0f,
	 0.0f, 0.0f, 0.0f, 1.0f
    };


    return mul(rotateZ, mul(rotateX, rotateY));
}
