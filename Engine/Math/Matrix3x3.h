#pragma once
#include "Vector2.h"
struct Matrix3x3
{
	float m[3][3];
};

Matrix3x3 operator*(const Matrix3x3& m1, const Matrix3x3& m2);

/// <summary>
/// 単位行列作成
/// </summary>
/// <returns>単位行列のMatrix3x3</returns>
Matrix3x3 MakeIdentityMatrix3x3();
/// <summary>
/// 逆行列作成
/// </summary>
/// <param name="m">逆行列を作る行列</param>
/// <returns>引数に渡された行列の逆行列</returns>
Matrix3x3 MakeInVerse3x3(const Matrix3x3& m);
/// <summary>
/// スカラー倍行列の作成
/// </summary>
/// <param name="s">各方向への倍率を格納したベクトル</param>
/// <returns>拡大縮小行列のMatrix3x3</returns>
Matrix3x3 MakeScaleMatrix3x3(const Vector2& s);
/// <summary>
/// 回転行列の作成と合成
/// </summary>
/// <param name="angle">各軸の角度を格納したベクトル</param>
/// <returns>回転行列のMatrix3x3</returns>
Matrix3x3 MakeRotateMatrix3x3(const Vector2& angle);
/// <summary>
/// 平行移動行列の作成
/// </summary>
/// <param name="translate">各軸への平行移動量を格納したベクトル</param>
/// <returns></returns>
Matrix3x3 MakeTranslateMatrix3x3(const Vector2& t);
/// <summary>
/// アフィン行列の作成
/// </summary>
/// <param name="scale">各方向への倍率を格納したベクトル</param>
/// <param name="rotate">各軸の角度を格納したベクトル</param>
/// <param name="translate">各軸への平行移動量を格納したベクトル</param>
/// <returns></returns>
Matrix3x3 MakeAffineMatrix3x3(const Vector2& scale, const Vector2& rotate, const Vector2& translate);