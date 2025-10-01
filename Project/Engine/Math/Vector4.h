#pragma once
class Vector4
{
public:
	float x;
	float y;
	float z;
	float w;
public:
	Vector4()
		:x(0.0f), y(0.0f), z(0.0f), w(0.0f)
	{};
	Vector4(float x, float y, float z, float w)
		:x(x), y(y), z(z), w(w)
	{};

	void operator*=(const Vector4 &v)
	{
		x *= v.x;
		y *= v.y;
		z *= v.z;
		w *= v.w;
	}

	void operator*=(const float n)
	{
		x /= n;
		y /= n;
		z /= n;
		w /= n;
	}
};

Vector4 operator/(const Vector4& v, float n);