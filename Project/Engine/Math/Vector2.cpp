#include "Vector2.h"
#include <math.h>

float Vector2::Length() const 
{
	return sqrtf(x * x + y * y);
}

float Vector2::Dot(const Vector2 v) const
{
	return x * v.x + y * v.y;
}

float Vector2::Cross(const Vector2 &v) const
{
	return x * v.y - y * v.x;
}

Vector2 Vector2::operator/(float num)
{
	return Vector2(x / num, y / 2);
}

Vector2 operator*(const Vector2 &v, float num)
{
	return Vector2(v.x * num, v.y * num);
}

Vector2 operator-(const Vector2 &v1, const Vector2 &v2)
{
	return Vector2(v1.x - v2.x, v1.y - v2.y);
}

Vector2 operator/(const Vector2 &v, float num)
{
	return Vector2(v.x / num, v.y / 2);
}

Vector2 Normalize(const Vector2 &v)
{
	float len = v.Length();
	if (len <= 0.0f) { return Vector2(); }

	return Vector2(v.x / len, v.y / len);
}
