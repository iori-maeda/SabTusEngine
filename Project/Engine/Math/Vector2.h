#pragma once
class Vector2
{
public:
	float x;
	float y;

	Vector2()
		:x(0.0f), y(0.0f)
	{};
	Vector2(const float &x, const float &y)
		:x(x), y(y)
	{};

	// ベクトルの長さ
	float Length() const;
	// 内積
	float Dot(const Vector2 v) const;
	// 外積
	float Cross(const Vector2 &v) const;

	Vector2 operator/(float num);
	friend Vector2 operator*(const Vector2 &v, float num);
	friend Vector2 operator-(const Vector2 &v1, const Vector2 &v2);
	friend Vector2 operator/(const Vector2 &v, float num);
};


Vector2 Normalize(const Vector2 &v);