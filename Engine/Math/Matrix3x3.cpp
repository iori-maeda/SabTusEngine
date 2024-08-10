#include "Matrix3x3.h"
#include <cmath>
Matrix3x3 operator*(const Matrix3x3& m1, const Matrix3x3& m2)
{
	Matrix3x3 result{ 0 };
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			for (int k = 0; k < 3; k++) {
				result.m[i][j] += m1.m[i][k] * m2.m[k][j];
			}
		}
	}
	return result;
}

Matrix3x3 MakeIdentityMatrix3x3()
{
	Matrix3x3 identity{
		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 1.0f,
	};
	return identity;
}

Matrix3x3 MakeInVerse3x3(const Matrix3x3& m)
{
#pragma region 行列の大きさを求める
	float length =
		+m.m[0][0] * m.m[1][1] * m.m[2][2]
		+ m.m[0][1] * m.m[1][2] * m.m[2][0]
		+ m.m[0][2] * m.m[1][0] * m.m[2][1]

		- m.m[0][2] * m.m[1][1] * m.m[2][0]
		- m.m[0][1] * m.m[1][0] * m.m[2][2]
		- m.m[0][0] * m.m[1][2] * m.m[2][1];
#pragma endregion end
#pragma region 逆行列を求める
	Matrix3x3 result{};
	result.m[0][0] = 1.0f / length * (m.m[1][1] * m.m[2][2] - m.m[1][2] * m.m[2][1]);
	result.m[0][1] = 1.0f / length * -(m.m[0][1] * m.m[2][2] - m.m[0][2] * m.m[2][1]);
	result.m[0][2] = 1.0f / length * (m.m[0][1] * m.m[1][2] - m.m[0][2] * m.m[1][1]);

	result.m[1][0] = 1.0f / length * -(m.m[1][0] * m.m[2][2] - m.m[1][2] * m.m[2][0]);
	result.m[1][1] = 1.0f / length * (m.m[0][0] * m.m[2][2] - m.m[0][2] * m.m[2][0]);
	result.m[1][2] = 1.0f / length * -(m.m[0][0] * m.m[1][2] - m.m[0][2] * m.m[1][0]);

	result.m[2][0] = 1.0f / length * (m.m[1][0] * m.m[2][1] - m.m[1][1] * m.m[2][0]);
	result.m[2][1] = 1.0f / length * -(m.m[0][0] * m.m[2][1] - m.m[0][2] * m.m[2][0]);
	result.m[2][2] = 1.0f / length * (m.m[0][0] * m.m[1][1] - m.m[0][1] * m.m[1][0]);
	return result;
}

Matrix3x3 MakeScaleMatrix3x3(const Vector2& s)
{
	Matrix3x3 scale{
		s.x,0.0f,0.0f,
		0.0f,s.y,0.0f,
		0.0f,0.0f,1.0f
	};
	return scale;
}

Matrix3x3 MakeRotateMatrix3x3(const Vector2& rotation)
{
	//Rotate 
	float sin = std::sinf(rotation.x);
	float cos = std::cosf(rotation.x);
	Matrix3x3 rotate = {
		 cos, sin,0.0f,
		-sin, cos,0.0f,
		0.0f,0.0f,1.0f,
	};
	return rotate;
}

Matrix3x3 MakeTranslateMatrix3x3(const Vector2& t)
{
	Matrix3x3 translate = {
		1.0f,0.0f,0.0f,
		0.0f,1.0f,0.0f,
		t.x,t.y,1.0f
	};
	return translate;
}

Matrix3x3 MakeAffineMatrix3x3(const Vector2& scale, const Vector2& rotation, const Vector2& translate)
{
	return MakeScaleMatrix3x3(scale) * MakeRotateMatrix3x3(rotation) * MakeTranslateMatrix3x3(translate);
}