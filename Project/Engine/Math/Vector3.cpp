#include "Vector3.h"
#include "Vector4.h"
#include <cmath>

void Vector3::operator=(const Vector4 &v)
{
	x = v.x;
	y = v.y;
	z = v.z;
}

void Vector3::operator+=(const Vector3 &v)
{
	x += v.x;
	y += v.y;
	z += v.z;
}

float Vector3::Length() const
{
	return sqrtf(x * x + y * y + z * z);
}

Vector3 operator+(const Vector3 &v1, const Vector3 &v2)
{
	return Vector3(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z);
}

Vector3 operator-(const Vector3 &v1, const Vector3 &v2)
{
	Vector3 result{
		v1.x - v2.x,
		v1.y - v2.y,
		v1.z - v2.z
	};

	return result;
}

Vector3 operator*(const Vector3 &v, float num)
{
	return Vector3(v.x * num, v.y * num, v.z * num);
}

Vector3 Normalize(const Vector3 &v)
{
	float length = std::sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
	return Vector3(v.x / length, v.y / length, v.z / length);
}

float Dot(const Vector3 &v1, const Vector3 &v2)
{
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

Vector3 Cross(const Vector3 &v1, const Vector3 &v2)
{
	return {
		v1.y * v2.z - v1.z * v2.y,
		v1.z * v2.x - v1.x * v2.z,
		v1.x * v2.y - v1.y * v2.x,
	};
}
