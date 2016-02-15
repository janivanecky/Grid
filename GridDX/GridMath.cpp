#include "GridMath.h"

const Matrix4x4 Math::GetOrthographicsProjectionRH(const float left, const float right,
										   const float bottom, const float top,
										   const float near, const float far)
{
	Matrix4x4 result = {0};
	result[0] = 2.0f / (right - left);
	result[5] = 2.0f / (top - bottom);
	result[10] = 1.0f / (far - near);
	result[12] = - (right + left) / (right - left);
	result[13] = - (top + bottom) / (top - bottom);
	result[14] = - (near) / (far - near);
	result[15] = 1;
	return result;
}

const Matrix4x4 Math::Transpose(const Matrix4x4& m)
{
	/*
		0 4 8  12
		1 5 9  13
		2 6 10 14
		3 7 11 15
	*/
	Matrix4x4 r = m;
	
	r[1] = m[4];
	r[2] = m[8];
	r[3] = m[12];
	
	r[4] = m[1];
	r[6] = m[9];
	r[7] = m[13];

	r[8] = m[2];
	r[9] = m[6];
	r[11] = m[14];

	r[12] = m[3];
	r[13] = m[7];
	r[14] = m[11];

	return r;
}

const Matrix4x4 Math::GetTranslation(const float dx, const float dy, const float dz)
{
	Matrix4x4 result = {0};

	result[0] = 1;
	result[5] = 1;
	result[10] = 1;
	result[12] = dx;
	result[13] = dy;
	result[14] = dz;
	result[15] = 1;

	return result;
}

const Matrix4x4 Math::GetRotation(const float angle, Vector3 axis)
{
	float c = Cos(angle);
	float s = Sin(angle);

	Matrix4x4 result = { 0 };
	float ax = axis.x;
	float ay = axis.y;
	float az = axis.z;
	result[0] = c + (1 - c) * ax * ax;
	result[1] = (1-c) * ax * ay + s * az;
	result[2] = (1-c) * ax * az - s * ay;
	result[3] = 0;

	result[4] = (1 - c) * ax * ay - s * az;
	result[5] = c + (1 - c) * ay * ay;
	result[6] = (1 - c) * ay * az + s * ax;
	result[7] = 0;

	result[8] = (1 - c) * ax * az + s * ay;
	result[9] = (1 - c) * ay * az - s * ax;
	result[10] = c + (1 - c) * az * az;
	result[11] = 0;
	
	result[12] = 0;
	result[13] = 0;
	result[14] = 0;
	result[15] = 1;

	return result;
}

const Matrix4x4 Math::GetIdentity()
{
	Matrix4x4 result = {0};
	result.v1 = Vector4(1,0,0,0);
	result.v2 = Vector4(0,1,0,0);
	result.v3 = Vector4(0,0,1,0);
	result.v4 = Vector4(0,0,0,1);
	return result;
}

const Matrix4x4 Math::GetReflectionY()
{
	Matrix4x4 result = GetIdentity();
	result.v2 = -result.v2;
	return result;
}

const Matrix4x4 Math::GetScale(const float sx, const float sy, const float sz)
{
	Matrix4x4 result = GetIdentity();
	result[0] = sx;
	result[5] = sy;
	result[10] = sz;
	return result;
}

float Abs(float x)
{
	return x < 0 ? -x : x;
}

short Floor(float x)
{
	return (short)(x + 32768.0f) - 32768;
}

#include <math.h>
const float Math::Sin(const float t)
{
	float B = 1.27323954474f;
	float C = -0.40528473456f;
	float P = 0.225f; 
	float sign = t < 0 ? -1.0f : 1.0f;
	float x = t - Floor((t + PI * sign) / PI2) * PI2;
	float y = B * x + C * x * Abs(x);
	y = P * (y * Abs(y) - y) + y;
	return sinf(t);
}

const float Math::Cos(const float t)
{
	return Sin(t + PIHALF);
}

float Math::Fmod(float x, float d)
{
	if (d == 0)
	{
		return 0;
	}
	while (x >= d)
	{
		x -= d;
	}
	return x;
}

float Math::AngularDistance(float alpha, float beta)
{
	float d = Abs(alpha - beta);
    if (d > PI)
        d = PI2 - d;
    return d;
}

float Math::Length(Vector2 v)
{
	float result = sqrtf(v.x * v.x + v.y * v.y);
	return result;
}

float Math::Sqrt(float x)
{
	return sqrtf(x);
}

#include <arm_neon.h>
float Math::Min(float a, float b)
{
	__n64 nA;
	nA.n64_f32[0] = a;
	__n64 nB;
	nB.n64_f32[0] = b;
	__n64 result = vmin_f32(nA, nB);
	return result.n64_f32[0];
}

float Math::Max(float a, float b)
{
	__n64 nA;
	nA.n64_f32[0] = a;
	__n64 nB;
	nB.n64_f32[0] = b;
	__n64 result = vmax_f32(nA, nB);
	return result.n64_f32[0];
}

float Math::Dot(Vector2 v1, Vector2 v2)
{
	float result = v1.x * v2.x + v1.y * v2.y;
	return result;
}

float Math::Distance(Vector2 v1, Vector2 v2)
{
	float result = Math::Length(v1 - v2);
	return result;
}

Vector2 Math::Normalize(Vector2 v)
{
	Vector2 result = v / Length(v);
	return result;
}

float Math::Abs(float x)
{
	__n64 input;
	input.n64_f32[0] = x;
	__n64 result = vabs_f32(input);
	return result.n64_f32[0];
}

int16 Math::Abs(int16 x)
{
	__n64 input;
	input.n64_i16[0] = x;
	__n64 result = vabs_s16(input);
	return result.n64_i16[0];
}

int32 Math::Abs(int32 x)
{
	__n64 input;
	input.n64_i32[0] = x;
	__n64 result = vabs_s32(input);
	return result.n64_i32[0];
}

float Math::Floor(float x)
{
	return floorf(x);
}

float Math::Pow(float base, float exponent)
{
	float result = powf(base, exponent);
	return result;
}

float Math::Smoothstep(float edge1, float edge2, float x)
{
    x = Clamp((x - edge1)/(edge2 - edge1), 0.0, 1.0); 
    return x*x*(3 - 2*x);
}

float Math::ExponentialFunction(float amplitude, float coef, float t)
{
	float result = amplitude * Math::Pow(2.0f, coef * t);
	return result;
}

float Math::Clamp(float x, float lower, float higher)
{
	if (x < lower)
		return lower;
	if (x > higher)
		return higher;
	return x;
}

int32 Math::Clamp(int32 x, int32 lower, int32 higher)
{
	if (x < lower)
		return lower;
	if (x > higher)
		return higher;
	return x;
}

