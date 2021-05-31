#pragma once
#include <stdint.h>
#include <math.h>
#include <random>

class vec2
{
public:

	vec2(float x_=0, float y_=0)
		: x(x_), y(y_) {}

	vec2 operator +(const vec2 &rhs) const
	{
		return vec2(x + rhs.x, y + rhs.y);
	}

	vec2 operator -(const vec2 &rhs) const
	{
		return vec2(x - rhs.x, y - rhs.y);
	}

	vec2 operator *(const float &rhs) const
	{
		return vec2(x * rhs, y * rhs);
	}

	vec2 operator /(const float &rhs) const
	{
		return vec2(x / rhs, y / rhs);
	}

	explicit operator bool() const
	{
		return (x || y);
	}

	vec2& operator +=(const vec2 &rhs)
	{
		x += rhs.x;
		y += rhs.y;
		return *this;
	}

	vec2& operator -=(const vec2 &rhs)
	{
		x -= rhs.x;
		y -= rhs.y;
		return *this;
	}

	vec2& operator *=(const float &rhs)
	{
		x *= rhs;
		y *= rhs;
		return *this ;
	}

	vec2& operator /=(const float &rhs)
	{
		x /= rhs;
		y /= rhs;
		return *this;
	}



	bool operator ==(const vec2 &rhs) const
	{
		return (x == rhs.x) && (y == rhs.y);
	}

	bool operator !=(const vec2 &rhs) const
	{
		return (x != rhs.x) || (y != rhs.y);
	}

	inline float Angle() const
	{
		return atan2f(y, x);
	}

	inline vec2 Perp() const
	{
		return vec2(y, -x);
	}

	inline vec2 Rotate(float Angle)
	{
		return vec2::FromAngle(this->Angle() + Angle);
	}

	inline vec2 Normalize() const
	{
		float Len = Length();
		if (Len == 0)
		{
			return *this;
		}

		float Inv = 1.f / Len;
		return vec2(x * Inv, y * Inv);
	}

	inline float SquaredLength() const
	{
		return x * x + y * y;
	}

	inline float Length() const
	{
		return sqrt(x * x + y * y);
	}

	inline float Dot(const vec2 rhs) const
	{
		vec2 t = Normalize();
		vec2 t2 = rhs.Normalize();
		return t.x * t2.x + t.y * t2.y;
	}



	static inline vec2 Rand()
	{
		vec2 Res(rand() - (RAND_MAX/2), rand() - (RAND_MAX / 2));
		return Res.Normalize();
	}

	static inline vec2 FromAngle(float Angle)
	{
		return vec2(cos(Angle), sin(Angle));
	}

	float x, y;
private:
	//float x, y;
};

class vec2i
{
public:
	vec2i()
		: x(0), y(0) {}

	vec2i(int32_t x_, int32_t y_)
		: x(x_), y(y_) {}

	vec2i(float x_, float y_)
		: x((int32_t)x), y((int32_t)y) {}

	vec2i(vec2 v)
		: x((int32_t)v.x), y((int32_t)v.y) {}


	vec2i operator +(const vec2i &rhs) const
	{
		return vec2i(x + rhs.x, y + rhs.y);
	}

	vec2i operator -(const vec2i &rhs) const
	{
		return vec2i(x - rhs.x, y - rhs.y);
	}

	vec2i& operator +=(const vec2i &rhs)
	{
		x += rhs.x;
		y += rhs.y;
		return *this;
	}

	bool operator ==(const vec2i &rhs) const
	{
		return (x == rhs.x) && (y == rhs.y);
	}

	bool operator !=(const vec2i &rhs) const
	{
		return (x != rhs.x) || (y != rhs.y);
	}


	int32_t x, y;
private:
};