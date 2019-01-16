#pragma once

#include "Core.h"

#include "GSM.hpp"

//Used to specify a location in 2D space with floating point precision.
GS_CLASS Vector2
{
public:
	//X component of this vector.
	float X = 0.0f;

	//Y component of this vector.
	float Y = 0.0f;

	Vector2()
	{
	}

	Vector2(float X, float Y) : X(X), Y(Y)
	{
	}

	Vector2(const Vector2 & Other) : X(Other.X), Y(Other.Y)
	{
	}

	void Negate()
	{
		X = -X;
		Y = -Y;

		return;
	}

	void Normalize()
	{
		*this = GSM::Normalize(*this);

		return;
	}

	Vector2 operator+ (float Other) const
	{
		return { X + Other, Y + Other };
	}

	Vector2 operator+ (const Vector2 & Other) const
	{
		return { X + Other.X, Y + Other.Y };
	}

	void operator+= (float Other)
	{
		X += Other;
		Y += Other;

		return;
	}

	void operator+= (const Vector2 & Other)
	{
		X += Other.X;
		Y += Other.Y;

		return;
	}

	Vector2 operator- (float Other) const
	{
		return { X - Other, Y - Other };
	}

	Vector2 operator- (const Vector2 & Other) const
	{
		return { X - Other.X, Y - Other.Y };
	}

	void operator-= (float Other)
	{
		X -= Other;
		Y -= Other;

		return;
	}

	void operator-= (const Vector2 & Other)
	{
		X -= Other.X;
		Y -= Other.Y;

		return;
	}
	
	Vector2 operator* (float Other) const
	{
		return { X * Other, Y * Other };
	}

	void operator*= (float Other)
	{
		X *= Other;
		Y *= Other;

		return;
	}

	Vector2 operator/ (float Other) const
	{
		return { X / Other, Y / Other };
	}

	void operator/= (float Other)
	{
		X /= Other;
		Y /= Other;

		return;
	}
};