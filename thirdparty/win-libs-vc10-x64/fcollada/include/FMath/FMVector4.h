/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

/**
	@file FMVector4.h
	This file contains the class for 4 dimensional vectors.
*/

#ifndef _FM_VECTOR4_H_
#define _FM_VECTOR4_H_

class FMColor;

/**
	A 4 dimensional vector.
	Not used within FCollada.
	
	@ingroup FMath
*/
class FCOLLADA_EXPORT
ALIGN_STRUCT(16)
FMVector4
{
public:
	float x;	/**< The first coordinate. */
	float y;	/**< The second coordinate. */
	float z;	/**< The third coordinate. */
	float w;	/**< The fourth coordinate. */

	/** Creates an empty 4D vector. */
	#ifndef _DEBUG
	inline FMVector4() {}
	#else
	inline FMVector4() { x = 123456789.0f; y = 123456789.0f; z = 123456789.0f; w = 123456789.0f; }
	#endif 

	/** Creates the FMVector4 from a list of \c floats.
		It takes the first 4 \c floats starting from and including \a startIndex
		(0 indexing) in the array as the 4 coordinates. The first as the first 
		coordinate, the second as the second, and the third as the third, the fourth as w
		@param source The \c float array.
		@param startIndex The index of the first element. */
	inline FMVector4(const float* source, uint32 startIndex = 0) { source = source + startIndex; x = *source++; y = *source++; z = *source++; w = *source; }

	/** Creates the FMVector4 with the coordinates given.
		The first three coordinates are taken from the FMVector3, where the
		first one is the x value, the second is that y, and the third is the z.
		The forth value is the \c float specified.
		@param v The FMVector3 representing the first three coordinates.
		@param _w The final coordinate. */
	inline FMVector4(const FMVector3& v, float _w) { x = v.x; y = v.y; z = v.z; w = _w; }

	/** Creates the FMVector4 with the coordinates given.
		@param v1 The FMVector2 representing the first two coordinates.
		@param v2 The FMVector2 representing the last two coordinates. */
	inline FMVector4(const FMVector2& v1, const FMVector2& v2) { x = v1.x; y = v1.y; z = v2.x; w = v2.y; }

	/** Creates the FMVector4 with the coordinates given.
		@param _x The first coordinate.
		@param _y The second coordinate.
		@param _z The third coordinate.
		@param _w The forth coordinate. */
	inline FMVector4(float _x, float _y, float _z, float _w) { x = _x; y = _y; z = _z; w = _w; }

	/** Creates the 4D vector from a given color value.
		@param c The color value. */
	FMVector4(const FMColor& c);

	/** Creates the 4D vector from a HSV color value.
		HSV stands for hue, saturation and value and is
		a more humane way to express color values, as opposed to RGB.
		@param hue The color hue. In the range [0,1].
		@param saturation The color saturation. In the range [0,1].
		@param value The color value. In the range [0,1]. */
	static FMVector4 FromHSVColor(float hue, float saturation, float value);

	/** Retrieves the HSV color value for this RGBA color value.
		All the color components are expected to be in the [0,1] range:
		no clamping will be done.
		@return The equivalent HSV color value. All the components will be
			in the [0,1] range. */
	FMVector3 ToHSVColor();

public:
	/** Get this FMVector4 as an array of \c floats.
		@return The \c float array. */
	inline operator float*() { return &x; }

	/** Get this FMVector4 as an array of \c floats.
		@return The \c float array. */
	inline operator const float*() const { return &x; }

	/** Assign this FMVector4 to the given float array.
		Assigns each coordinate of this FMVector4 to the elements in the \c
		float array. The first element to the first coordinate, the second to
		the second, and the third to the third. It returns this FMVector4.
		@param v The \c float array to assign with.
		@return This FMVector4. */
	inline FMVector4& operator =(const float* v) { x = *v; y = *(v + 1); z = *(v + 2); w = *(v+3); return *this; }

	/** Update each component of this FMVector to the minimum of two FMVector4s.
		Updates each of the four components to be the minimum of the current
		value and that of the corresponding value of the given FMVector4.
		@param min The FMVector to take values from. */
	inline void ComponentMinimum(const FMVector4& min) { if (x < min.x) x = min.x; if (y < min.y) y = min.y; if (z < min.z) z = min.z; if (w < min.w) w = min.w; }

	/** Update each component of this FMVector to the maximum of two FMVector4s.
		Updates each of the four components to be the maximum of the current
		value and that of the corresponding value of the given FMVector4.
		@param max The FMVector to take values from. */
	inline void ComponentMaximum(const FMVector4& max) { if (x > max.x) x = max.x; if (y > max.y) y = max.y; if (z > max.z) z = max.z; if (w > max.w) w = max.w;}

public:
	static const FMVector4 Zero;	/**< The 4D vector containing all zeroes. */
	static const FMVector4 One;		/**< The 4D vector containing all ones. */
	static const FMVector4 AlphaOne;
};

/** Scalar multiplication with a FMVector4.
	@param a The vector.
	@param b The scalar.
	@return The FMVector4 representing the resulting vector. */
inline FMVector4 operator *(const FMVector4& a, float b) { return FMVector4(a.x * b, a.y * b, a.z * b, a.w * b); }

/** Scalar multiplication with a FMVector4.
	@param a The scalar.
	@param b The vector.
	@return The FMVector4 representing the resulting vector. */
inline FMVector4 operator *(float a, const FMVector4& b) { return FMVector4(a * b.x, a * b.y, a * b.z, a * b.w); }

/** Vector addition with two FMVector4.
	@param a The first vector.
	@param b The second vector.
	@return The FMVector4 representation of the resulting vector. */
inline FMVector4 operator +(const FMVector4& a, const FMVector4& b) { return FMVector4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w); }

/** Vector substraction with two FMVector4.
	@param a The first vector.
	@param b The second vector.
	@return The FMVector4 representation of the resulting vector. */
inline FMVector4 operator -(const FMVector4& a, const FMVector4& b) { return FMVector4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w); }

/** Assignment of the addition of two FMVector4.
	@param b The first vector, which will also be assigned to the result.
	@param a The second vector.
	@return The first vector, after it has been assigned new values. */
inline FMVector4& operator +=(FMVector4& b, const FMVector4& a) { b.x += a.x; b.y += a.y; b.z += a.z; b.w += a.w; return b; }

/** Assignment of the subtraction of two FMVector4.
	@param b The first vector, which will also be assigned to the result.
	@param a The second vector.
	@return The first vector, after it has been assigned new values. */
inline FMVector4& operator -=(FMVector4& b, const FMVector4& a) { b.x -= a.x; b.y -= a.y; b.z -= a.z; b.w -= a.w; return b; }

/** Assignment of the scalar multiplication of a FMVector4.
	@param b The vector, which will also be assigned to the result.
	@param a The scalar.
	@return The vector, after it has been assigned new values. */
inline FMVector4& operator *=(FMVector4& b, float a) { b.x *= a; b.y *= a; b.z *= a; b.w *= a; return b; }

/** Assignment of the scalar division of a FMVector4.
	@param b The vector, which will also be assigned to the result.
	@param a The scalar.
	@return The vector, after it has been assigned new values. */
inline FMVector4& operator /=(FMVector4& b, float a) { b.x /= a; b.y /= a; b.z /= a; b.w /= a; return b; }

/** Dot product of two FMVector4.
	@param a The first vector.
	@param b The second vector.
	@return The result of the dot product. */
inline float operator *(const FMVector4& a, const FMVector4& b) { return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w; }


/** Returns whether two 4D vectors are equivalent.
	@param p A first vector.
	@param q A second vector.
	@return Whether the vectors are equivalent. */
inline bool IsEquivalent(const FMVector4& p, const FMVector4& q) { return IsEquivalent(p.x, q.x) && IsEquivalent(p.y, q.y) && IsEquivalent(p.z, q.z) && IsEquivalent(p.w, q.w); }
inline bool operator==(const FMVector4& p, const FMVector4& q) { return IsEquivalent(p.x, q.x) && IsEquivalent(p.y, q.y) && IsEquivalent(p.z, q.z) && IsEquivalent(p.w, q.w); } /**< See above. */

/** A dynamically-sized array of 4D vectors or points. */
typedef fm::vector<FMVector4> FMVector4List;

#endif // _FM_VECTOR4_H_
