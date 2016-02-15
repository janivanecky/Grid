#pragma once

#define Assert(value) ((value) ? 1 : *((char *)0) = 0)
#define PI 3.14159265359f
#define PI2 6.28318530718f
#define PIHALF 1.57079632679f

struct Vector2
{
    float x;
    float y;

	Vector2()
	{
		this->x = 0;
		this->y = 0;
	}

    Vector2(float x, float y)
    {
        this->x = x;
        this->y = y;
    }

	const Vector2 operator-(const Vector2 v2)
	{
		return Vector2(this->x - v2.x, this->y - v2.y);
	}

	const Vector2 operator-(const Vector2 v2) const
	{
		return Vector2(this->x - v2.x, this->y - v2.y);
	}

	const Vector2 operator-()
	{
		return Vector2(-this->x , -this->y);
	}

	const Vector2 operator*(const float v)
	{
		return Vector2(this->x * v, this->y * v);
	}

	const Vector2 operator*(const float v) const
	{
		return Vector2(this->x * v, this->y * v);
	}

	const Vector2 operator/(const float v)
	{
		return Vector2(this->x / v, this->y / v);
	}

	const Vector2 operator+(const Vector2 v2)
	{
		return Vector2(this->x + v2.x, this->y + v2.y);
	}

	const Vector2& operator+=(const Vector2 v2)
	{
		this->x += v2.x;
		this->y += v2.y;
		return *this;
	}

	const Vector2& operator*=(const float x)
	{
		this->x *= x;
		this->y *= x;
		return *this;
	}

	const Vector2 operator+(const Vector2 v2) const
	{
		return Vector2(this->x + v2.x, this->y + v2.y);
	}

};

struct IntVector2
{
	int x;
	int y;

	IntVector2()
	{
		this->x = 0;
		this->y = 0;
	}

	IntVector2(int x, int y)
	{
		this->x = x;
		this->y = y;
	}

	const IntVector2 operator+(const IntVector2 v2)
	{
		return IntVector2(this->x + v2.x, this->y + v2.y);
	}
};

struct Vector3
{
	float x;
	float y;
	float z;

	Vector3()
	{
		this->x = 0;
		this->y = 0;
		this->z = 0;
	}

	Vector3(float x, float y, float z)
	{
		this->x = x;
		this->y = y;
		this->z = z;
	}

	const Vector3 operator*(const float x)
	{
		return Vector3(this->x * x, this->y * x, this->z * x);
	}

	const Vector3 operator/(const float x) const
	{
		return Vector3(this->x / x, this->y / x, this->z / x);
	}

	const Vector3 operator+(const Vector3 x) const
	{
		return Vector3(this->x + x.x, this->y + x.y, this->z + x.z);
	}
};

struct Vector4
{
    float x;
    float y;
	float z;
	float w;

	Vector4()
	{
		this->x = 0;
		this->y = 0;
		this->z = 0;
		this->w = 0;
	}

    Vector4(float x, float y, float z, float w)
    {
        this->x = x;
        this->y = y;
        this->z = z;
        this->w = w;
    }

	const Vector4 operator-()
	{
		return Vector4(-x, -y, -z, -w);
	}
};

struct Rectangle
{
	float x;
	float y;
	float width;
	float height;
	Rectangle()
	{
		this->x = 0;
		this->y = 0;
		this->width = 0;
		this->height = 0;
	}

	Rectangle(float x, float y, float width, float height)
	{
		this->x = x;
		this->y = y;
		this->width = width;
		this->height = height;
	}
};

struct Matrix4x4
{
	union
	{
		float x[16];
		struct
		{
			Vector4 v1;
			Vector4 v2;
			Vector4 v3;
			Vector4 v4;
		};
	};

	const float& operator[](int index) const
	{
		return x[index];
	}

	float& operator[](int index)
	{
		return x[index];
	}

	const Matrix4x4 operator *(const Matrix4x4 m)
	{
		Matrix4x4 result = {0};
		result[0] = x[0] * m[0] + x[4] * m[1] + x[8] * m[2] + x[12] * m[3];
		result[1] = x[1] * m[0] + x[5] * m[1] + x[9] * m[2] + x[13] * m[3];
		result[2] = x[2] * m[0] + x[6] * m[1] + x[10] * m[2] + x[14] * m[3];
		result[3] = x[3] * m[0] + x[7] * m[1] + x[11] * m[2] + x[15] * m[3];
		
		result[4] = x[0] * m[4] + x[4] * m[5] + x[8] * m[6] + x[12] * m[7];
		result[5] = x[1] * m[4] + x[5] * m[5] + x[9] * m[6] + x[13] * m[7];
		result[6] = x[2] * m[4] + x[6] * m[5] + x[10] * m[6] + x[14] * m[7];
		result[7] = x[3] * m[4] + x[7] * m[5] + x[11] * m[6] + x[15] * m[7];

		result[8] = x[0] * m[8] + x[4] * m[9] + x[8] * m[10] + x[12] * m[11];
		result[9] = x[1] * m[8] + x[5] * m[9] + x[9] * m[10] + x[13] * m[11];
		result[10] = x[2] * m[8] + x[6] * m[9] + x[10] * m[10] + x[14] * m[11];
		result[11] = x[3] * m[8] + x[7] * m[9] + x[11] * m[10] + x[15] * m[11];

		result[12] = x[0] * m[12] + x[4] * m[13] + x[8] * m[14] + x[12] * m[15];
		result[13] = x[1] * m[12] + x[5] * m[13] + x[9] * m[14] + x[13] * m[15];
		result[14] = x[2] * m[12] + x[6] * m[13] + x[10] * m[14] + x[14] * m[15];
		result[15] = x[3] * m[12] + x[7] * m[13] + x[11] * m[14] + x[15] * m[15];
		return result;
	}

	const Matrix4x4 operator *(const Matrix4x4 m) const
	{
		Matrix4x4 result = {0};
		result[0] = x[0] * m[0] + x[4] * m[1] + x[8] * m[2] + x[12] * m[3];
		result[1] = x[1] * m[0] + x[5] * m[1] + x[9] * m[2] + x[13] * m[3];
		result[2] = x[2] * m[0] + x[6] * m[1] + x[10] * m[2] + x[14] * m[3];
		result[3] = x[3] * m[0] + x[7] * m[1] + x[11] * m[2] + x[15] * m[3];
		
		result[4] = x[0] * m[4] + x[4] * m[5] + x[8] * m[6] + x[12] * m[7];
		result[5] = x[1] * m[4] + x[5] * m[5] + x[9] * m[6] + x[13] * m[7];
		result[6] = x[2] * m[4] + x[6] * m[5] + x[10] * m[6] + x[14] * m[7];
		result[7] = x[3] * m[4] + x[7] * m[5] + x[11] * m[6] + x[15] * m[7];

		result[8] = x[0] * m[8] + x[4] * m[9] + x[8] * m[10] + x[12] * m[11];
		result[9] = x[1] * m[8] + x[5] * m[9] + x[9] * m[10] + x[13] * m[11];
		result[10] = x[2] * m[8] + x[6] * m[9] + x[10] * m[10] + x[14] * m[11];
		result[11] = x[3] * m[8] + x[7] * m[9] + x[11] * m[10] + x[15] * m[11];

		result[12] = x[0] * m[12] + x[4] * m[13] + x[8] * m[14] + x[12] * m[15];
		result[13] = x[1] * m[12] + x[5] * m[13] + x[9] * m[14] + x[13] * m[15];
		result[14] = x[2] * m[12] + x[6] * m[13] + x[10] * m[14] + x[14] * m[15];
		result[15] = x[3] * m[12] + x[7] * m[13] + x[11] * m[14] + x[15] * m[15];
		return result;
	}
};

namespace Math
{
	const float Sin(const float t);
	const float Cos(const float t);
	float Fmod(float x, float d);
	float Abs(float x);
	int16 Abs(int16 x);
	int32 Abs(int32 x);
	float Floor(float x);
	float Sqrt(float x);
	float Min(float a, float b);
	float Max(float a, float b);
	float Pow(float base, float exponent);
	float Clamp(float x, float lower, float higher);
	int32 Clamp(int32 x, int32 lower, int32 higher);
	float Smoothstep(float edge1, float edge2, float x);

	float Length(Vector2 v);
	float Dot(Vector2 v1, Vector2 v2);
	Vector2 Normalize(Vector2 v);
	float Distance(Vector2 v1, Vector2 v2);

	float AngularDistance(float alpha, float beta);

	float ExponentialFunction(float amplitude, float coef, float t);
	
	const Matrix4x4 GetOrthographicsProjectionRH(const float left, const float right,
										 const float bottom, const float top,
										 const float near, const float far);
	const Matrix4x4 Transpose(const Matrix4x4& m);
	const Matrix4x4 GetTranslation(const float dx, const float dy, const float dz);
	const Matrix4x4 GetRotation(const float angle, Vector3 axis);
	const Matrix4x4 GetIdentity();
	const Matrix4x4 GetReflectionY();
	const Matrix4x4 GetScale(const float sx, const float sy, const float sz);
}


