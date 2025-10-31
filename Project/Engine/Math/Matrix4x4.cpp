#include "Matrix4x4.h"
#include <cmath>
#include "Matrix3x3.h"

Matrix4x4 operator*(const Matrix4x4 &m1, const Matrix4x4 &m2)
{
	Matrix4x4 result{ 0 };
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			for (int k = 0; k < 4; k++)
			{
				result.m[i][j] += m1.m[i][k] * m2.m[k][j];
			}
		}
	}
	return result;
}

Vector3 operator*(const Vector3 &v, const Matrix4x4 &m)
{
	Vector3 result{};
	result.x = v.x * m.m[0][0] + v.y * m.m[1][0] + v.z * m.m[2][0] + m.m[3][0];
	result.y = v.x * m.m[0][1] + v.y * m.m[1][1] + v.z * m.m[2][1] + m.m[3][1];
	result.z = v.x * m.m[0][2] + v.y * m.m[1][2] + v.z * m.m[2][2] + m.m[3][2];
	return result;
}

Vector3 ConvertToTransform(const Vector3 &v, const Matrix4x4 &m)
{
	Vector3 result{};
	result.x = v.x * m.m[0][0] + v.y * m.m[1][0] + v.z * m.m[2][0] + m.m[3][0];
	result.y = v.x * m.m[0][1] + v.y * m.m[1][1] + v.z * m.m[2][1] + m.m[3][1];
	result.z = v.x * m.m[0][2] + v.y * m.m[1][2] + v.z * m.m[2][2] + m.m[3][2];
	float w = v.x * m.m[0][3] + v.y * m.m[1][3] + v.z * m.m[2][3] + m.m[3][3];
	if (w != 0)
	{
		result.x /= w;
		result.y /= w;
		result.z /= w;
	}
	return result;
}

Matrix4x4 MakeIdentityMatrix()
{
	Matrix4x4 identity{
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};
	return identity;
}

Matrix4x4 MakeInVerse(const Matrix4x4 &m)
{
#pragma region 行列の大きさを求める
	float length =
		+m.m[0][0] * m.m[1][1] * m.m[2][2] * m.m[3][3]
		+ m.m[0][0] * m.m[1][2] * m.m[2][3] * m.m[3][1]
		+ m.m[0][0] * m.m[1][3] * m.m[2][1] * m.m[3][2]

		- m.m[0][0] * m.m[1][3] * m.m[2][2] * m.m[3][1]
		- m.m[0][0] * m.m[1][2] * m.m[2][1] * m.m[3][3]
		- m.m[0][0] * m.m[1][1] * m.m[2][3] * m.m[3][2]

		- m.m[0][1] * m.m[1][0] * m.m[2][2] * m.m[3][3]
		- m.m[0][2] * m.m[1][0] * m.m[2][3] * m.m[3][1]
		- m.m[0][3] * m.m[1][0] * m.m[2][1] * m.m[3][2]

		+ m.m[0][3] * m.m[1][0] * m.m[2][2] * m.m[3][1]
		+ m.m[0][2] * m.m[1][0] * m.m[2][1] * m.m[3][3]
		+ m.m[0][1] * m.m[1][0] * m.m[2][3] * m.m[3][2]

		+ m.m[0][1] * m.m[1][2] * m.m[2][0] * m.m[3][3]
		+ m.m[0][2] * m.m[1][3] * m.m[2][0] * m.m[3][1]
		+ m.m[0][3] * m.m[1][1] * m.m[2][0] * m.m[3][2]

		- m.m[0][3] * m.m[1][2] * m.m[2][0] * m.m[3][1]
		- m.m[0][2] * m.m[1][1] * m.m[2][0] * m.m[3][3]
		- m.m[0][1] * m.m[1][3] * m.m[2][0] * m.m[3][2]

		- m.m[0][1] * m.m[1][2] * m.m[2][3] * m.m[3][0]
		- m.m[0][2] * m.m[1][3] * m.m[2][1] * m.m[3][0]
		- m.m[0][3] * m.m[1][1] * m.m[2][2] * m.m[3][0]

		+ m.m[0][3] * m.m[1][2] * m.m[2][1] * m.m[3][0]
		+ m.m[0][2] * m.m[1][1] * m.m[2][3] * m.m[3][0]
		+ m.m[0][1] * m.m[1][3] * m.m[2][2] * m.m[3][0];
#pragma endregion end
#pragma region 逆行列を求める
	Matrix4x4 result{};
#pragma region Matrixe4x4[0][n]
	result.m[0][0] = 1.0f / length
		* (+m.m[1][1] * m.m[2][2] * m.m[3][3]
			+ m.m[1][2] * m.m[2][3] * m.m[3][1]
			+ m.m[1][3] * m.m[2][1] * m.m[3][2]

			- m.m[1][3] * m.m[2][2] * m.m[3][1]
			- m.m[1][2] * m.m[2][1] * m.m[3][3]
			- m.m[1][1] * m.m[2][3] * m.m[3][2]);

	result.m[0][1] = 1.0f / length
		* (-m.m[0][1] * m.m[2][2] * m.m[3][3]
			- m.m[0][2] * m.m[2][3] * m.m[3][1]
			- m.m[0][3] * m.m[2][1] * m.m[3][2]

			+ m.m[0][3] * m.m[2][2] * m.m[3][1]
			+ m.m[0][2] * m.m[2][1] * m.m[3][3]
			+ m.m[0][1] * m.m[2][3] * m.m[3][2]);

	result.m[0][2] = 1.0f / length
		* (+m.m[0][1] * m.m[1][2] * m.m[3][3]
			+ m.m[0][2] * m.m[1][3] * m.m[3][1]
			+ m.m[0][3] * m.m[1][1] * m.m[3][2]

			- m.m[0][3] * m.m[1][2] * m.m[3][1]
			- m.m[0][2] * m.m[1][1] * m.m[3][3]
			- m.m[0][1] * m.m[1][3] * m.m[3][2]);

	result.m[0][3] = 1.0f / length
		* (-m.m[0][1] * m.m[1][2] * m.m[2][3]
			- m.m[0][2] * m.m[1][3] * m.m[2][1]
			- m.m[0][3] * m.m[1][1] * m.m[2][2]

			+ m.m[0][3] * m.m[1][2] * m.m[2][1]
			+ m.m[0][2] * m.m[1][1] * m.m[2][3]
			+ m.m[0][1] * m.m[1][3] * m.m[2][2]);
#pragma endregion end
#pragma region Marix4x4[1][n]
	result.m[1][0] = 1.0f / length
		* (-m.m[1][0] * m.m[2][2] * m.m[3][3]
			- m.m[1][2] * m.m[2][3] * m.m[3][1]
			- m.m[1][3] * m.m[2][0] * m.m[3][2]

			+ m.m[1][3] * m.m[2][2] * m.m[3][0]
			+ m.m[1][2] * m.m[2][0] * m.m[3][3]
			+ m.m[1][0] * m.m[2][3] * m.m[3][2]);

	result.m[1][1] = 1.0f / length
		* (+m.m[0][0] * m.m[2][2] * m.m[3][3]
			+ m.m[0][2] * m.m[2][3] * m.m[3][0]
			+ m.m[0][3] * m.m[2][0] * m.m[3][2]

			- m.m[0][3] * m.m[2][2] * m.m[3][0]
			- m.m[0][2] * m.m[2][0] * m.m[3][3]
			- m.m[0][0] * m.m[2][3] * m.m[3][2]);

	result.m[1][2] = 1.0f / length
		* (-m.m[0][0] * m.m[1][2] * m.m[3][3]
			- m.m[0][2] * m.m[1][3] * m.m[3][0]
			- m.m[0][3] * m.m[1][0] * m.m[3][2]

			+ m.m[0][3] * m.m[1][2] * m.m[3][0]
			+ m.m[0][2] * m.m[1][0] * m.m[3][3]
			+ m.m[0][0] * m.m[1][3] * m.m[3][2]);

	result.m[1][3] = 1.0f / length
		* (+m.m[0][0] * m.m[1][2] * m.m[2][3]
			+ m.m[0][2] * m.m[1][3] * m.m[3][0]
			+ m.m[0][3] * m.m[1][0] * m.m[2][2]

			- m.m[0][3] * m.m[1][2] * m.m[2][0]
			- m.m[0][2] * m.m[1][0] * m.m[2][3]
			- m.m[0][0] * m.m[1][3] * m.m[2][2]);
#pragma endregion end
#pragma region Marix4x4[2][n]
	result.m[2][0] = 1.0f / length
		* (+m.m[1][0] * m.m[2][1] * m.m[3][3]
			+ m.m[1][1] * m.m[2][3] * m.m[3][0]
			+ m.m[1][3] * m.m[2][0] * m.m[3][1]

			- m.m[1][3] * m.m[2][1] * m.m[3][0]
			- m.m[1][1] * m.m[2][0] * m.m[3][3]
			- m.m[1][0] * m.m[2][3] * m.m[3][1]);

	result.m[2][1] = 1.0f / length
		* (-m.m[0][0] * m.m[2][1] * m.m[3][3]
			- m.m[0][1] * m.m[2][3] * m.m[3][0]
			- m.m[0][3] * m.m[2][0] * m.m[3][1]

			+ m.m[0][3] * m.m[2][1] * m.m[3][0]
			+ m.m[0][1] * m.m[2][0] * m.m[3][3]
			+ m.m[0][0] * m.m[2][3] * m.m[3][1]);

	result.m[2][2] = 1.0f / length
		* (+m.m[0][0] * m.m[1][1] * m.m[3][3]
			+ m.m[0][1] * m.m[1][3] * m.m[3][0]
			+ m.m[0][3] * m.m[1][0] * m.m[3][1]

			- m.m[0][3] * m.m[1][1] * m.m[3][0]
			- m.m[0][1] * m.m[1][0] * m.m[3][3]
			- m.m[0][0] * m.m[1][3] * m.m[3][1]);

	result.m[2][3] = 1.0f / length
		* (-m.m[0][0] * m.m[1][1] * m.m[2][3]
			- m.m[0][1] * m.m[1][3] * m.m[2][0]
			- m.m[0][3] * m.m[1][0] * m.m[2][1]

			+ m.m[0][3] * m.m[1][1] * m.m[2][0]
			+ m.m[0][1] * m.m[1][0] * m.m[2][3]
			+ m.m[0][0] * m.m[1][3] * m.m[2][1]);
#pragma endregion end
#pragma region Marix4x4[3][n]
	result.m[3][0] = 1.0f / length
		* (-m.m[1][0] * m.m[2][1] * m.m[3][2]
			- m.m[1][1] * m.m[2][2] * m.m[3][0]
			- m.m[1][2] * m.m[2][0] * m.m[3][1]

			+ m.m[1][2] * m.m[2][1] * m.m[3][0]
			+ m.m[1][1] * m.m[2][0] * m.m[3][2]
			+ m.m[1][0] * m.m[2][2] * m.m[3][1]);

	result.m[3][1] = 1.0f / length
		* (+m.m[0][0] * m.m[2][1] * m.m[3][2]
			+ m.m[0][1] * m.m[2][2] * m.m[3][0]
			+ m.m[0][2] * m.m[2][0] * m.m[3][1]

			- m.m[0][2] * m.m[2][1] * m.m[3][0]
			- m.m[0][1] * m.m[2][0] * m.m[3][2]
			- m.m[0][0] * m.m[2][2] * m.m[3][1]);

	result.m[3][2] = 1.0f / length
		* (-m.m[0][0] * m.m[1][1] * m.m[3][2]
			- m.m[0][1] * m.m[1][2] * m.m[3][0]
			- m.m[0][2] * m.m[1][0] * m.m[3][1]

			+ m.m[0][2] * m.m[1][1] * m.m[3][0]
			+ m.m[0][1] * m.m[1][0] * m.m[3][2]
			+ m.m[0][0] * m.m[1][2] * m.m[3][1]);

	result.m[3][3] = 1.0f / length
		* (+m.m[0][0] * m.m[1][1] * m.m[2][2]
			+ m.m[0][1] * m.m[1][2] * m.m[2][0]
			+ m.m[0][2] * m.m[1][0] * m.m[2][1]

			- m.m[0][2] * m.m[1][1] * m.m[2][0]
			- m.m[0][1] * m.m[1][0] * m.m[2][2]
			- m.m[0][0] * m.m[1][2] * m.m[2][1]);
#pragma endregion end

	return result;
}

Matrix4x4 MakeScaleMatrix(const Vector3 &s)
{
	Matrix4x4 scale{
		s.x,0.0f,0.0f,0.0f,
		0.0f,s.y,0.0f,0.0f,
		0.0f,0.0f,s.z,0.0f,
		0.0f,0.0f,0.0f,1.0f,
	};
	return scale;
}

Matrix4x4 MakeRotateMatrix(const Vector3 &rotation)
{
	// X Rotate 
	float sin = std::sinf(rotation.x);
	float cos = std::cosf(rotation.x);
	Matrix4x4 rotateX = {
		1.0f,0.0f,0.0f,0.0f,
		0.0f, cos, sin,0.0f,
		0.0f,-sin, cos,0.0f,
		0.0f,0.0f,0.0f,1.0f,
	};

	// Y Rotate
	sin = std::sinf(rotation.y);
	cos = std::cosf(rotation.y);
	Matrix4x4 rotateY = {
		  cos,0.0f,-sin,0.0f,
		 0.0f,1.0f,0.0f,0.0f,
		  sin,0.0f, cos,0.0f,
		 0.0f,0.0f,0.0f,1.0f,
	};

	// Z Rotate
	sin = std::sinf(rotation.z);
	cos = std::cosf(rotation.z);
	Matrix4x4 rotateZ = {
		  cos, sin,0.0f,0.0f,
		 -sin, cos,0.0f,0.0f,
		 0.0f,0.0f,1.0f,0.0f,
		 0.0f,0.0f,0.0f,1.0f
	};


	return rotateZ * rotateX * rotateY;
}

Matrix4x4 MakeRotateX(float angle)
{
	// X Rotate 
	float sin = std::sinf(angle);
	float cos = std::cosf(angle);
	Matrix4x4 rotateX = {
		1.0f,0.0f,0.0f,0.0f,
		0.0f, cos, sin,0.0f,
		0.0f,-sin, cos,0.0f,
		0.0f,0.0f,0.0f,1.0f,
	};
	return rotateX;
}

Matrix4x4 MakeRotateY(float angle)
{
	// Y Rotate
	float sin = std::sinf(angle);
	float cos = std::cosf(angle);
	Matrix4x4 rotateY = {
		  cos,0.0f,-sin,0.0f,
		 0.0f,1.0f,0.0f,0.0f,
		  sin,0.0f, cos,0.0f,
		 0.0f,0.0f,0.0f,1.0f,
	};
	return rotateY;
}

Matrix4x4 MakeRotateZ(float angle)
{
	// Z Rotate
	float sin = std::sinf(angle);
	float cos = std::cosf(angle);
	Matrix4x4 rotateZ = {
		  cos, sin,0.0f,0.0f,
		 -sin, cos,0.0f,0.0f,
		 0.0f,0.0f,1.0f,0.0f,
		 0.0f,0.0f,0.0f,1.0f
	};
	return rotateZ;
}

Matrix4x4 MakeTranslateMatrix(const Vector3 &t)
{
	Matrix4x4 translate = {
		1.0f,0.0f,0.0f,0.0f,
		0.0f,1.0f,0.0f,0.0f,
		0.0f,0.0f,1.0f,0.0f,
		t.x,t.y,t.z,1.0f
	};
	return translate;
}

Matrix4x4 MakeAffineMatrix(const Vector3 &scale, const Vector3 &rotation, const Vector3 &translate)
{
	return MakeScaleMatrix(scale) * MakeRotateMatrix(rotation) * MakeTranslateMatrix(translate);
}

Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip)
{
	Matrix4x4 result = MakeIdentityMatrix();
	result.m[0][0] = 1.0f / aspectRatio * 1.0f / tan(fovY / 2.0f);
	result.m[1][1] = 1.0f / tan(fovY / 2.0f);
	result.m[2][2] = farClip / (farClip - nearClip);
	result.m[2][3] = 1.0f;
	result.m[3][2] = -nearClip * farClip / (farClip - nearClip);
	result.m[3][3] = 0.0f;

	return result;
}

Matrix4x4 MakeOrthoGraphicsMatrix(float left, float top, float width, float height, float minDepth, float maxDepth)
{
	Matrix4x4 result = MakeIdentityMatrix();
	result.m[0][0] = 2.0f / (width - left);
	result.m[1][1] = 2.0f / (top - height);
	result.m[2][2] = 1.0f / (maxDepth - minDepth);
	result.m[3][0] = (left + width) / (left - width);
	result.m[3][1] = (top + height) / (height - top);
	result.m[3][2] = minDepth / (minDepth - maxDepth);
	result.m[3][3] = 1.0f;
	return result;
}

Matrix4x4 MakeViewPortMatrix(float left, float top, float width, float height, float minDepth, float maxDeppth)
{
	Matrix4x4 result = MakeIdentityMatrix();
	result.m[0][0] = width / 2.0f;
	result.m[1][1] = -height / 2.0f;
	result.m[2][2] = maxDeppth - minDepth;
	result.m[3][0] = left + width / 2.0f;
	result.m[3][1] = top + height / 2.0f;
	result.m[3][2] = minDepth;
	return result;
}

Matrix4x4 MakeTransposeMatrix(const Matrix4x4 &m)
{
	Matrix4x4 result = MakeIdentityMatrix();
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			result.m[i][j] = m.m[j][i];
		}
	}
	return result;
}

Matrix4x4 MakeRotateAxisAngle(const Vector3 &axis, float angle)
{
	Matrix4x4 result{};

	float cos = std::cosf(angle);
	float invertCos = 1.0f - cos;

	float sin = std::sinf(angle);

	result.m[0][0] = (axis.x * axis.x) * invertCos + cos;
	result.m[0][1] = (axis.x * axis.y) * invertCos + axis.z * sin;
	result.m[0][2] = (axis.x * axis.z) * invertCos - axis.y  * sin;
	result.m[0][3] = 0.0f;

	
	result.m[1][0] = (axis.x * axis.y) * invertCos - axis.z * sin;
	result.m[1][1] = (axis.y * axis.y) * invertCos + cos;
	result.m[1][2] = (axis.y * axis.z) * invertCos + axis.x  * sin;
	result.m[1][3] = 0.0f;

	result.m[2][0] = (axis.x * axis.z) * invertCos + axis.y * sin;
	result.m[2][1] = (axis.y * axis.z) * invertCos - axis.x * sin;
	result.m[2][2] = (axis.z* axis.z) * invertCos + cos;
	result.m[2][3] = 0.0f;

	result.m[3][0] = 0.0f;
	result.m[3][1] = 0.0f;
	result.m[3][2] = 0.0f;
	result.m[3][3] = 1.0f;

	return result;
}

Matrix3x3 MakeRotationMatrix3x3(const Matrix4x4 &m)
{
	Matrix3x3 result = MakeIdentityMatrix3x3();

	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			result.m[i][j] = m.m[i][j];
		}
	}
	return result;
}
