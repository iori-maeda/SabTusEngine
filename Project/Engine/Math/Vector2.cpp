#include "Vector2.h"
#include <math.h>

float Vector2::Length()
{
	return sqrtf(x * x + y * y);
}

float Vector2::Dot(const Vector2 v) const
{
	return x * v.x + y * v.y;
}

float Vector2::Cross(const Vector2& v) const
{
	return x * v.y - y * v.x;
}

Vector2 Vector2::operator/(float num)
{
	return Vector2(x / num, y / 2);
}

Vector2 operator/(const Vector2& v, float num)
{
	return Vector2(v.x / num, v.y / 2);
}
