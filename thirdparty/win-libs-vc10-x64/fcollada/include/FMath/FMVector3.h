/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

/**
	@file FMVector3.h The file containing the class and global functions for 3 dimensional vectors.
*/

#ifndef _FM_VECTOR3_H_
#define _FM_VECTOR3_H_

class FMVector4;

/**
	A 3 dimensional vector.

	Simple, non-optimized vector class: * is the dot-product, ^ is the 
	cross-product.
	
	@ingroup FMath
*/
class FCOLLADA_EXPORT
ALIGN_STRUCT(16)
FMVector3
{
public:
	float x;	/**< The first coordinate. */
	float y;	/**< The second coordinate. */
	float z;	/**< The third coordinate. */
private:
	float w;	// For alignment purposes.

public:
	/**
	 * Creates an empty FMVector3.
	 */
	#ifndef _DEBUG
	inline FMVector3() {}
	#else
	inline FMVector3() { x = 123456789.0f; y = 123456789.0f; z = 123456789.0f; }
	#endif 

	/** Creates the FMVector3 with the coordinates given.
		@param _x The first coordinate.
		@param _y The second coordinate.
		@param _z The third coordinate. */
	inline FMVector3(float _x, float _y, float _z) { x = _x; y = _y; z = _z; }

	/** Copy constuctor. 
		@param rhs A second 3D vector. */
	inline FMVector3(const FMVector3& rhs) { x = rhs.x; y = rhs.y; z = rhs.z; }

	/** Constructor.
		Reduces the 4D vector into 3D by removing the 4th dimension.
		@param vect4 A 4D vector. */
	FMVector3(const FMVector4& vect4);

	/** Creates the FMVector3 from a list of floating-point values.
		It takes the first three floating-point starting from and including \a startIndex
		(0 indexing) in the array as the 3 coordinates. The first as the first 
		coordinate, the second as the second, and the third as the third.
		@param source The floating-point value array.
		@param startIndex The index of the first element. */
	FMVector3(const float* source, uint32 startIndex = 0);
	FMVector3(const double* source, uint32 startIndex = 0);	/**< See above. */

	/**	Retrieves the squared length of the vector.
		@return The squared length of this vector. */
	inline float LengthSquared() const { return x * x + y * y + z * z; }

	/**	Retrieves the length of the vector.
		@return The length of this vector. */
	inline float Length() const { return sqrtf(x * x + y * y + z * z); }

	/** Normalizes this vector. */
	inline void NormalizeIt() { float l = Length(); if (l > 0.0f) { x /= l; y /= l; z /= l; } else { x = y = 0; z = 1; }}

	/** Get a normalized FMVector3 with the same direction as this vector.
		@return A FMVector3 with length 1 and same direction as this vector. */
	inline FMVector3 Normalize() const { float l = Length(); return (l > 0.0f) ? FMVector3(x / l, y / l, z / l) : FMVector3::XAxis; }

	/** Project this FMVector3 onto another FMVector3.
		@param unto The FMVector3 to project onto. */
	inline void Project(const FMVector3& unto) { (*this) = Projected(unto); }

	/** Get the projection of this FMVector3 onto another FMVector3.
		@param unto The FMVector3 to project onto.
		@return The projected FMVector3. */
	inline FMVector3 Projected(const FMVector3& unto) const;

	/** Get this FMVector3 as an array of \c floats.
		@return The \c float array. */
	inline operator float*() { return &x; }

    /** Get this FMVector3 as an array of \c floats.
		@return The \c float array. */
	inline operator const float*() const { return &x; }

	/** Set the values of this vector
		@param x The new X value
		@param y The new Y value
		@param z The new Z value */
	inline void Set(float x, float y, float z) { this->x = x; this->y = y, this->z = z; }

	/** Assign this FMVector3 to the given float array.
		Assigns each coordinate of this FMVector3 to the elements in the \c
		float array. The first element to the first coordinate, the second to
		the second, and the third to the third. It returns this FMVector3.
		@param v The \c float array to assign with.
		@return This vector. */
	inline FMVector3& operator =(const float* v) { x = *v; y = *(v + 1); z = *(v + 2); return *this; }

	/** Assigns the FMVector3 passed to outselves.
		Copies XYZ from the passed vector
		@param rhs The vector copy off */
	inline FMVector3& operator =(const FMVector3& rhs) { x=rhs.x; y=rhs.y; z=rhs.z; return *this; }

	/** Update each component of this FMVector to the minimum of two FMVector3s.
		Updates each of the three components to be the minimum of the current
		value and that of the corresponding value of the given FMVector3.
		@param min The FMVector to take values from. */
	inline void ComponentMinimum(const FMVector3& min) { if (x < min.x) x = min.x; if (y < min.y) y = min.y; if (z < min.z) z = min.z; }

	/** Retrieves the smallest component of the vector.
		@return The smallest component of the vector. */
	inline float ComponentMinimum() const { return min(fabsf(x), min(fabsf(y), fabsf(z))); }

	/** Update each component of this FMVector to the maximum of two FMVector3s.
		Updates each of the three components to be the maximum of the current
		value and that of the corresponding value of the given FMVector3.
		@param max The FMVector to take values from. */
	inline void ComponentMaximum(const FMVector3& max) { if (x > max.x) x = max.x; if (y > max.y) y = max.y; if (z > max.z) z = max.z; }

	/** Retrieves the largest component of the vector.
		@return The largest component of the vector. */
	inline float ComponentMaximum() const { return max(fabsf(x), max(fabsf(y), fabsf(z))); }

	/** Clamp each component of this FMVector by the corresponding components
		in the specified min and max FMVector3.
		Clamp refers to setting a value within a given range. If the value is
		lower than the minimum of the range, it is set to the minimum; same for
		the maximum.
		@param min The FMVector to take the minimum values from.
		@param max The FMVector to take the maximum values from. */
	inline void ComponentClamp(const FMVector3& min, const FMVector3& max) { ComponentMinimum(min); ComponentMaximum(max); }

	/** Retrieves the average of the three vector components.
		@return The component average. */
	inline float ComponentAverage() const { return (fabsf(x) + fabsf(y) + fabsf(z)) / 3.0f; }

public:
	static const FMVector3 XAxis; /**< The FMVector3 representing the x axis */
	static const FMVector3 YAxis; /**< The FMVector3 representing the y axis */
	static const FMVector3 ZAxis; /**< The FMVector3 representing the z axis */
	static const FMVector3 Origin;/**< The FMVector3 representing the origin */
	static const FMVector3 Zero;  /**< The FMVector3 containing all zeroes: (0,0,0). */
	static const FMVector3 One;	  /**< The FMVector3 containing all ones: (1,1,1). */
};

/** Vector addition with two FMVector3.
	@param a The first vector.
	@param b The second vector.
	@return The FMVector3 representation of the resulting vector. */
inline FMVector3 operator +(const FMVector3& a, const FMVector3& b) { return FMVector3(a.x + b.x, a.y + b.y, a.z + b.z); }

/** Vector subtraction with two FMVector3.
	@param a The first vector.
	@param b The second vector.
	@return The FMVector3 representation of the resulting vector. */
inline FMVector3 operator -(const FMVector3& a, const FMVector3& b) { return FMVector3(a.x - b.x, a.y - b.y, a.z - b.z); }

/** Positive operator of the given FMVector3.
	It applies the positive operator to each of the components of the FMVector3.
	@param a The vector to apply the positive operator to.
	@return The FMVector3 representation of the resulting vector. */
inline FMVector3 operator +(const FMVector3& a) { return FMVector3(+a.x, +a.y, +a.z); }

/** Negates the given FMVector3.
	It negates each of the components of the FMVector3.
	@param a The vector to negate.
	@return The FMVector3 representation of the resulting vector. */
inline FMVector3 operator -(const FMVector3& a) { return FMVector3(-a.x, -a.y, -a.z); }

/** Dot product of two FMVector3.
	@param a The first vector.
	@param b The second vector.
	@return The result of the dot product. */
inline float operator *(const FMVector3& a, const FMVector3& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

/** Scalar multiplication with a FMVector3.
	@param a The vector.
	@param b The scalar.
	@return The FMVector3 representing the resulting vector. */
inline FMVector3 operator *(const FMVector3& a, float b) { return FMVector3(a.x * b, a.y * b, a.z * b); }

/** Scalar multiplication with a FMVector3.
	@param a The scalar.
	@param b The vector.
	@return The FMVector3 representing the resulting vector. */
inline FMVector3 operator *(float a, const FMVector3& b) { return FMVector3(a * b.x, a * b.y, a * b.z); }

/** Scalar division with a FMVector3.
	@param a The vector.
	@param b The scalar.
	@return The FMVector3 representing the resulting vector. */
inline FMVector3 operator /(const FMVector3& a, float b) { return FMVector3(a.x / b, a.y / b, a.z / b); }

/** Cross product of two FMVector3.
	@param a The first vector.
	@param b The second vector.
	@return The result of the dot product. */
inline FMVector3 operator ^(const FMVector3& a, const FMVector3& b) { return FMVector3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x); }

/** Assignment of the addition of two FMVector3.
	@param b The first vector, which will also be assigned to the result.
	@param a The second vector.
	@return The first vector, after it has been assigned new values. */
inline FMVector3& operator +=(FMVector3& b, const FMVector3& a) { b.x += a.x; b.y += a.y; b.z += a.z; return b; }

/**	Assignment of the subtraction of two FMVector3.
	@param b The first vector, which will also be assigned to the result.
	@param a The second vector.
	@return The first vector, after it has been assigned new values. */
inline FMVector3& operator -=(FMVector3& b, const FMVector3& a) { b.x -= a.x; b.y -= a.y; b.z -= a.z; return b; }

/** Assignment of the scalar multiplication of a FMVector3.
	@param b The vector, which will also be assigned to the result.
	@param a The scalar.
	@return The vector, after it has been assigned new values. */
inline FMVector3& operator *=(FMVector3& b, float a) { b.x *= a; b.y *= a; b.z *= a; return b; }

/** Assignment of the scalar division of a FMVector3.
	@param b The vector, which will also be assigned to the result.
	@param a The scalar.
	@return The vector, after it has been assigned new values. */
inline FMVector3& operator /=(FMVector3& b, float a) { b.x /= a; b.y /= a; b.z /= a; return b; }

/** Returns whether two 3D vectors or points are equivalent.
	@param p A first vector.
	@param q A second vector.
	@return Whether the vectors are equivalent. */
inline bool IsEquivalent(const FMVector3& p, const FMVector3& q) { return IsEquivalent(p.x, q.x) && IsEquivalent(p.y, q.y) && IsEquivalent(p.z, q.z); }
inline bool operator == (const FMVector3& p, const FMVector3& q) { return IsEquivalent(p, q); } /**< See above. */

// Already documented above.
inline FMVector3 FMVector3::Projected(const FMVector3& unto) const { return ((*this) * unto) / unto.LengthSquared() * unto; }

/** A dynamically-sized array of 3D vectors or points. */
typedef fm::vector<FMVector3> FMVector3List;

#endif // _FM_VECTOR3_H_
