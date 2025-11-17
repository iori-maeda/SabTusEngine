#pragma once
#include "Vector3.h"
#include "Matrix3x3.h"

struct Matrix4x4
{
public:
	float m[4][4];
};
Matrix4x4 operator*(const Matrix4x4 &m1, const Matrix4x4 &m2);

Vector3 operator*(const Vector3 &v, const Matrix4x4 &m);

/// <summary>
/// ワールド座標の取得
/// </summary>
/// <param name=""></param>
/// <param name=""></param>
/// <returns></returns>
Vector3 ConvertToTransform(const Vector3 &, const Matrix4x4 &);

/// <summary>
/// 単位行列作成
/// </summary>
/// <returns>単位行列のMatrix4x4</returns>
Matrix4x4 MakeIdentityMatrix();
/// <summary>
/// 逆行列作成
/// </summary>
/// <param name="m">逆行列を作る行列</param>
/// <returns>引数に渡された行列の逆行列</returns>
Matrix4x4 MakeInVerse(const Matrix4x4 &m);
/// <summary>
/// スカラー倍行列の作成
/// </summary>
/// <param name="s">各方向への倍率を格納したベクトル</param>
/// <returns>拡大縮小行列のMatrix4x4</returns>
Matrix4x4 MakeScaleMatrix(const Vector3 &s);
/// <summary>
/// 回転行列の作成と合成
/// 回転順序はz -> x -> y->
/// </summary>
/// <param name="angle">各軸の角度を格納したベクトル</param>
/// <returns>回転行列のMatrix4x4</returns>
Matrix4x4 MakeRotateMatrix(const Vector3 &angle);
/// <summary>
/// X軸を中心に回転
/// </summary>
/// <param name="angle">radian</param>
/// <returns></returns>
Matrix4x4 MakeRotateX(float angle);
/// <summary>
/// Y軸を中心に回転
/// </summary>
/// <param name="angle">radian</param>
/// <returns></returns>
Matrix4x4 MakeRotateY(float angle);
/// <summary>
/// Z軸を中心に回転
/// </summary>
/// <param name="angle">radian</param>
/// <returns></returns>
Matrix4x4 MakeRotateZ(float angle);
/// <summary>
/// 平行移動行列の作成
/// </summary>
/// <param name="translate">各軸への平行移動量を格納したベクトル</param>
/// <returns></returns>
Matrix4x4 MakeTranslateMatrix(const Vector3 &t);
/// <summary>
/// アフィン行列の作成
/// </summary>
/// <param name="scale">各方向への倍率を格納したベクトル</param>
/// <param name="rotate">各軸の角度を格納したベクトル</param>
/// <param name="translate">各軸への平行移動量を格納したベクトル</param>
/// <returns></returns>
Matrix4x4 MakeAffineMatrix(const Vector3 &scale, const Vector3 &rotate, const Vector3 &translate);
/// <summary>
/// 透視投影行列
/// </summary>
/// <param name="fovY">縦画角</param>
/// <param name="aspectRatio">アスペクト比</param>
/// <param name="nearClip">近平面への距離</param>
/// <param name="farClip">遠平面への距離</param>
/// <returns>投資投影行列</returns>
Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip);
/// <summary>
/// 平行投影行列
/// </summary>
/// <param name="left"></param>
/// <param name="top"></param>
/// <param name="width"></param>
/// <param name="height"></param>
/// <param name="minDepth"></param>
/// <param name="maxDepth"></param>
/// <returns></returns>
Matrix4x4 MakeOrthoGraphicsMatrix(float left, float top, float width, float height, float minDepth, float maxDepth);
/// <summary>
/// ビューポート行列
/// </summary>
/// <param name="left"></param>
/// <param name="top"></param>
/// <param name="width"></param>
/// <param name="height"></param>
/// <param name="minDepth"></param>
/// <param name="maxDepth"></param>
/// <returns></returns>
Matrix4x4 MakeViewPortMatrix(float left, float top, float width, float height, float minDepth, float maxDeppth);
/// <summary>
/// 転置行列の作成
/// </summary>
/// <param name="m">転置する行列</param>
/// <returns></returns>
Matrix4x4 MakeTransposeMatrix(const Matrix4x4 &m);
/// <summary>
/// 任意軸回転行列の作成
/// </summary>
/// <param name="axis"></param>
/// <param name="angle"></param>
/// <returns></returns>
Matrix4x4 MakeRotateAxisAngle(const Vector3& axis, float angle);

/// <summary>
/// 回転行列の取得
///</summary>
/// <param name="m">ワールド行列</param>
/// <returns></returns>
Matrix3x3 MakeRotationMatrix3x3(const Matrix4x4& m);
