#pragma once

class Vector4;

class Vector3
{
public:
	float x;
	float y;
	float z;

	void operator=(const Vector4& v);
	void operator+=(const Vector3& v);
	Vector3() :x(0.0f), y(0.0f), z(0.0f) {};
	Vector3(float x, float y, float z) :x(x), y(y), z(z) {};

	float Length() const;
};

Vector3 operator+(const Vector3&, const Vector3&);
Vector3 operator-(const Vector3&, const Vector3&);
Vector3 operator*(const Vector3&, float);

Vector3 Normalize(const Vector3&);
float Dot(const Vector3& v1, const Vector3& v2);
Vector3 Cross(const Vector3& v1, const Vector3& v2);
