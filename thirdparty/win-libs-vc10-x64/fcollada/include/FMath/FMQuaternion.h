/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

/**
	@file FMQuaternion.h
	The file containing the class for quaternions.
*/

#ifndef _FM_QUATERNION_H_
#define _FM_QUATERNION_H_

/**
	A quaternion.
	Used to represent rotations: quaternions have the sole advantage of reduced
	floating-point error when concatenating rotations.
	
	@ingroup FMath
*/
class FCOLLADA_EXPORT FMQuaternion
{
public:
	float x;	/**< The i component. */
	float y;	/**< The j component. */
	float z;	/**< The k component. */
	float w;	/**< The scalar component. */

#ifndef _DEBUG
	/**	Creates an empty FMQuaternion.
		The default values are non deterministic. */
	FMQuaternion() {}
#else
	FMQuaternion() { x = 123456789.0f; y = 123456789.0f; z = 123456789.0f; w = 123456789.0f; }
#endif 

	/**	Creates the quaternion with the given component values.
		@param _x The i component.
		@param _y The j component.
		@param _z The k component.
		@param _w The scalar component. */
	FMQuaternion(float _x, float _y, float _z, float _w) { x = _x; y = _y; z = _z; w = _w; }

	/**	Creates the quaternion with the given component values.
		@param values A static float array containing at least four elements. */
	FMQuaternion(const float* values);
	FMQuaternion(const double* values); /**< See above. */

	/**	Creates the quaternion from a given axis and angle of rotation.
		@param axis The axis of rotation.
		@param angle The angle of rotation in radians. */
	FMQuaternion(const FMVector3& axis, float angle);

	/** Retrieves this quaternion as an array of \c floats.
		@return The \c float array. */
	inline operator float*() { return &x; }
	inline operator const float*() const { return &x; } /**< See above. */

	/** Assign this FMQuaternion to the given \c float array.
		Assigns each coordinate of this FMQuaternion to the elements in the 
		\c float array. The first element to the i component, the second to the
		j, the third to the k, and the forth to the scalar.
		@param v The \c float array to assign with.
		@return This quaternion. */
	inline FMQuaternion& operator =(const float* v) { x = *v; y = *(v + 1); z = *(v + 2); w = *(v + 3); return *this; }

	/**	Retrieves the squared length of the vector.
		@return The squared length of this vector. */
	inline float LengthSquared() const { return x * x + y * y + z * z + w * w; }

	/**	Retrieves the length of the vector.
		@return The length of this vector. */
	inline float Length() const { return sqrtf(x * x + y * y + z * z + w * w); }

	/** Normalizes this vector. */
	inline void NormalizeIt() { float l = Length(); if (l > 0.0f) { x /= l; y /= l; z /= l; w /= l; }}

	/** Returns a quaternion with a unit length at the same space as this quaternion
		@return A FMVector3 with length 1 and same direction as this vector. */
	inline FMQuaternion Normalize() const { float l = Length(); return FMQuaternion(x / l, y / l, z / l, w / l); }

	/** Returns the concatonation of this quaternion rotation and the given
		quaternion rotation.
		@param q A second quaternion rotation.
		@return The result of the two quaternion rotation. */
	FMQuaternion operator*(const FMQuaternion& q) const;

	/** Returns the concatonation of this quaternion rotation applied
		on the given vector. This concatonation works great on
		vector but will give strange results for points.
		@param v A 3D vector to rotate.
		@return The rotated 3D vector. */
	FMVector3 operator*(const FMVector3& v) const;

	/** Returns the conjugate of this quaternion.
		@return The conjugate. */
	inline FMQuaternion operator~() const { return FMQuaternion(-x, -y, -z, w); }

	/** Applies quaternion multiplication of the given quaternion with this
		quaternion and returns the value.
		@param q The quaternion multiplied with.
		@return The resulting quaternion. */
	inline FMQuaternion& operator*=(const FMQuaternion& q) { return (*this) = (*this) * q; }

	/** Copy constructor.
		Clones the given quaternion into this quaternion.
		@param q A quaternion to clone.
		@return This quaternion. */
	inline FMQuaternion& operator=(const FMQuaternion& q) { x = q.x; y = q.y; z = q.z; w = q.w; return (*this); }

	/** Returns the slerp of this quaternion to other at time time
		@param other The Quaternion to interpolate to
		@param time The percentage (0 < time < 1) to slerp
		@return The Quaternion formed by the slerp */
	FMQuaternion slerp(const FMQuaternion& other, float time) const;

	/** Converts a quaternion to its Euler rotation angles.
		@param previousAngles To support animated quaternions conversion,
			you need to pass in the previous quaternion's converted angles.
			The closest angles to the previous angles will be returned for a smooth animation.
			If this parameter is NULL, one valid set of angles will be returned.
		@return A 3D vector containing the Euler rotation angles. */
	FMVector3 ToEuler(FMVector3* previousAngles = NULL) const;

	/** Converts a quaternion to a angle-axis rotation.
		@param axis The returned axis for the rotation.
		@param angle The returned angle for the rotation, in radians. */
	void ToAngleAxis(FMVector3& axis, float& angle) const;

	/** Converts a quaternion to a transformation matrix.
		@return The transformation matrix for this quaternion. */
	FMMatrix44 ToMatrix() const;

	/** Sets the Quaternions transform onto the specified matrix.
		This will overwrite any existing rotations, but not positions
		@param m  The matrix to set our transform onto */
	void SetToMatrix(FMMatrix44& m) const;

	/** Get the FMQuaternion representation of the Euler rotation angles.
		@param x The rotation about the x-axis (roll), in radians.
		@param y The rotation about the y-axis (pitch), in radians.
		@param z The rotation about the z-axis (yaw), in radians.
	 */
	static FMQuaternion EulerRotationQuaternion(float x, float y, float z);

	/** Get the FMQuaternion that represents the FMMatrix44 rotation
		@param mat The matrix whose rotation we will represent */
	static FMQuaternion MatrixRotationQuaternion(const FMMatrix44& mat);

public:
	static const FMQuaternion Zero; /**< The zero quaternion. */

	/** The identity quaternion.
		Transforming a vector or a point with this quaternion
		returns the same vector or point. */
	static const FMQuaternion Identity;
};

/** Retrieves whether two quaternions are equivalent.
	@param a A first quaternion.
	@param b A second quaternion.
	@return Whether the two quaternions are equivalent. */
inline bool IsEquivalent(const FMQuaternion& a, const FMQuaternion& b)
{
	return IsEquivalent(a.x, b.x) && IsEquivalent(a.y, b.y) && IsEquivalent(a.z, b.z) && IsEquivalent(a.w, b.w);
}

#endif // _FM_QUATERNION_H_
