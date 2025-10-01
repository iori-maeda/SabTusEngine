#include "Vector4.h"

Vector4 operator/(const Vector4 &v, float n)
{
	return Vector4(v.x / n, v.y / n, v.z / n, v.w / n);
}
