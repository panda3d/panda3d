/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

/**
	@file FMVector2.h
	The file containing the class and global functions for 2 dimensional 
	vectors.
*/

#ifndef _FM_VECTOR2_H_
#define _FM_VECTOR2_H_

/**
	A 2 dimensional vector.
	Not used within FCollada.
	
	@ingroup FMath
*/
class FCOLLADA_EXPORT FMVector2
{
public:
	union
	{
		float u;	/**< The first coordinate. */
		float x;	/**< The first coordinate. */
	};
	union
	{
		float v;	/**< The second coordinate. */
		float y;	/**< The second coordinate. */
	};

public:
	/** Creates an empty FMVector2. */
	#ifndef _DEBUG
	FMVector2() {}
	#else
	FMVector2() { u = 123456789.0f; v = 123456789.0f; }
	#endif 

	/** Creates the vector with the coordinates given.
		@param _u The first coordinate.
		@param _v The second coordinate. */
	FMVector2(float _u, float _v) { u = _u; v = _v; }

	/**	Retrieves the squared length of the vector.
		@return The squared length of this vector. */
	inline float LengthSquared() const { return x * x + y * y; }

	/**	Retrieves the length of the vector.
		@return The length of this vector. */
	inline float Length() const { return sqrtf(x * x + y * y); }

	/** Normalizes this vector. */
	inline void NormalizeIt() { float l = Length(); if (l > 0.0f) { x /= l; y /= l; } else { x = 0; y = 1; }}

	/** Get a normalized vector with the same direction as this vector.
		@return A vector with length 1 and same direction as this vector. */
	inline FMVector2 Normalize() const { float l = Length(); return (l > 0.0f) ? FMVector2(x / l, y / l) : FMVector2::XAxis; }

	/** Get this vector as an array of \c floats.
		@return The \c float array. */
	inline operator float*() { return &u; }

	/** Adds two vector.
		Adds to this vector's coordinates the individual components of the
		given vector and returns this vector.
		@param a The vector to add with this one.
		@return This vector. */
	inline FMVector2& operator +=(const FMVector2& a) { u += a.u; v += a.v; return *this; }

	/** Multiplies this vector by a scaler.
		Multiplies each of this vector's coordinates with the scaler and
		returns this vector.
		@param a The scalar to multiply with.
		@return This vector. */
	inline FMVector2& operator *=(float a) { u *= a; v *= a; return *this; }
	
	/** Assign this vector to the given float array.
		Assigns each coordinate of this vector to the elements in the \c
		float array. The first element to the first coordinate and the second to
		the second. It returns this vector.
		@param f The \c float array to assign with.
		@return This vector. */
	inline FMVector2& operator =(const float* f) { u = *f; v = *(f + 1); return *this; }

public:
	static const FMVector2 Zero; /**< The zero vector. */
	static const FMVector2 Origin; /**< The zero vector. */
	static const FMVector2 XAxis; /**< The 2D X axis. */
	static const FMVector2 YAxis; /**< The 2D Y axis. */
};

/** Vector addition with two vectors.
	@param a The first vector.
	@param b The second vector.
	@return The FMVector2 representation of the resulting vector. */
inline FMVector2 operator + (const FMVector2& a, const FMVector2& b) { return FMVector2(a.u + b.u, a.v + b.v); }

/** Vector subtraction with two vectors.
	@param a The first vector.
	@param b The second vector.
	@return The FMVector2 representation of the resulting vector. */
inline FMVector2 operator -(const FMVector2& a, const FMVector2& b) { return FMVector2(a.u - b.u, a.v - b.v); }

/** Dot product of two vectors.
	@param a The first vector.
	@param b The second vector.
	@return The result of the dot product.
 */
inline float operator *(const FMVector2& a, const FMVector2& b) { return a.u * b.u + a.v * b.v; }

/** Scalar multiplication with a vector.
	@param a The vector.
	@param b The scalar.
	@return The FMVector2 representing the resulting the vector. */
inline FMVector2 operator *(const FMVector2& a, float b) { return FMVector2(a.u * b, a.v * b); }

/**	Scalar multiplication with a vector.
	@param a The scalar.
	@param b The vector.
	@return The vector representing the resulting the vector. */
inline FMVector2 operator *(float a, const FMVector2& b) { return FMVector2(a * b.u, a * b.v); }

/** Scalar divison with a vector.
	@param a The vector.
	@param b The scalar.
	@return The vector representing the resulting the vector. */
inline FMVector2 operator /(const FMVector2& a, float b) { return FMVector2(a.x / b, a.y / b); }

/** Retrieves whether two 2D vectors are equivalent.
	@param a A first vector.
	@param b A second vector.
	@return Whether the two given 2D vectors are equivalent. */
inline bool IsEquivalent(const FMVector2& a, const FMVector2& b) { return IsEquivalent(a.x, b.x) && IsEquivalent(a.y, b.y); }
inline bool operator==(const FMVector2& a, const FMVector2& b) { return IsEquivalent(a.x, b.x) && IsEquivalent(a.y, b.y); } /**< See above. */

/** A dynamically-sized array of 2D vectors or points. */
typedef fm::vector<FMVector2> FMVector2List;

#endif // _FM_VECTOR2_H_
