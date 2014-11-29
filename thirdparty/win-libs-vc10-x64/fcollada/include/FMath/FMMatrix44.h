/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

/**
	@file FMMatrix44.h
	The file containing the class and global functions for 4x4 matrices.
*/

#ifndef _FM_MATRIX44_H_
#define _FM_MATRIX44_H_

/**
	A 4x4 Row Major matrix: use to represent 3D transformations.

	@ingroup FMath
*/
class FCOLLADA_EXPORT
ALIGN_STRUCT(16)
FMMatrix44
{
public:

	float m[4][4];	/**< The matrix elements stored in a 2D array. */

	/** Creates a FMMatrix44 from the \c float array.
		The float array stores the elements in the following order: m[0][0], 
		m[1][0], m[2][0], m[3][0], m[0][1], m[1][1], m[2][1], m[3][1], m[0][2],
		m[1][2], m[2][2], m[3][2], m[0][3], m[1][3], m[2][3], m[3][3].
		@param _m The \c float array to create the matrix from. */
	FMMatrix44(const float* _m);
	FMMatrix44(const double* _m); /**< See above. */

	/** Creates an empty FMMatrix44.
		The default values are left un-initialized.
		To get an identity matrix:
		FMMatrix44 identity(FMMatrix44::Identity); */
	#ifndef _DEBUG
	inline FMMatrix44() {}
	#else
	inline FMMatrix44() { memset(m, 55, 16 * sizeof(float)); }
	#endif 

	/** Get this FMMatrix44 as an array of \c floats.
		The array contains the elements in the following order: m[0][0], 
		m[0][1], m[0][2], m[0][3], m[1][0], m[1][1], m[1][2], m[1][3], m[2][0],
		m[2][1], m[2][2], m[0][3], m[3][0], m[3][1], m[3][2], m[3][3].
		@return The \c float array. */
	inline operator float*() { return &m[0][0]; }
	inline operator const float*() const { return &m[0][0]; } /**< See above. */

	/** Get a specified row of FMMatrix44 as an array of \c floats.
		@param a The row index, starting at 0, of the row to get.
		@return The \c float array of the elements in the specified row. */
	template <class Integer> float* operator[](Integer a) { return m[a]; }
	template <class Integer> const float* operator[](Integer a) const { return m[a]; } /**< See above. */

	/** Assign this FMMatrix44's elements to be the same as that of the given 
		matrix.
		@param copy The FMMatrix to copy elements from.
		@return This FMMatrix. */
	FMMatrix44& operator=(const FMMatrix44& copy);

	/** Sets a FMMatrix44 from the \c float array.
		The float array stores the elements in the following order: m[0][0],
		m[1][0], m[2][0], m[3][0], m[0][1], m[1][1], m[2][1], m[3][1], m[0][2], 
		m[1][2], m[2][2], m[3][2], m[0][3], m[1][3], m[2][3], m[3][3].
		@param _m The \c float array to create the matrix from. */
	void Set(const float* _m);
	void Set(const double* _m);	/**< See above. */

	/** Gets the transposed of this FMMatrix44.
		@return The transposed of this FMMatrix. */
	FMMatrix44 Transposed() const;

	/** Gets the inverse of this matrix.
		@return The inverse of this matrix. */
	FMMatrix44 Inverted() const;

	/** Gets the determinant of this matrix.
		@return The determinant of this matrix. */
	float Determinant() const;
	
	/** Decompose this matrix into its scale, rotation, and translation
		components; it also tells whether it is inverted. To get back the 
		original matrix, perform the following multiplication:
		translation * axis-rotation [z*y*x order] * scale - or use
		the Recompose function. If invert is negative, 
		then to get back the original matrix, negate scale and perform the 
		above multiplication.
		@param scale The FMVector to place the scale components to.
		@param rotation The FMVector to place the rotation components to.
		@param translation The FMVector to place the translation components to.
		@param inverted -1.0 if inverted, 1.0 if not inverted. */
	void Decompose(FMVector3& scale, FMVector3& rotation, FMVector3& translation, float& inverted) const;

	/** Recompose this matrix from its scale, rotation, and translation
		components.; it also tells whether it is inverted. To get back the 
		original matrix, perform the following multiplication:
		translation * rotation [x*y*z order] * scale. If invert is negative, 
		then to get back the original matrix, negate scale and perform the 
		above multiplication.
		@param scale The FMVector to place the scale components to.
		@param rotation The FMVector to place the rotation components to.
		@param translation The FMVector to place the translation components to.
		@param inverted -1.0 if inverted, 1.0 if not inverted. */
	void Recompose(const FMVector3& scale, const FMVector3& rotation, const FMVector3& translation, float inverted = 1.0f);

	/** Transforms the given point by this matrix.
		@param coordinate The point to transform.
		@return The vector representation of the transformed point. */
	FMVector3 TransformCoordinate(const FMVector3& coordinate) const;
	FMVector4 TransformCoordinate(const FMVector4& coordinate) const; /**< See above. */

	/** Transforms the given vector by this FMMatrix44.
		@param v The vector to transform.
		@return The FMVector3 representation of the transformed vector. */
	FMVector3 TransformVector(const FMVector3& v) const;

	/** Gets the translation component of this matrix.
		@return A Reference to the FMVector3 representation of the translation. */
	inline const FMVector3& GetTranslation() const { return GetAxis(FMath::TRANS); }
	inline FMVector3& GetTranslation() { return GetAxis(FMath::TRANS); } /**< See Above */

	/** Sets the translation component of this matrix.
		@param translation The new translation component. */
	inline void SetTranslation(const FMVector3& translation) { GetTranslation() = translation; }

	/** Gets a reference to an axis of this matrix
		@param axis The index of the axis to get (X = 0, Y = 1, Z = 2)
		@return A reference to the axis */
	inline const FMVector3& GetAxis(FMath::AXIS axis) const { return *(FMVector3*)(size_t)(m[axis]); }
	inline FMVector3& GetAxis(FMath::AXIS axis) { return *(FMVector3*)(size_t)(m[axis]); } /**< See above */

public:
	static FMMatrix44 Identity;	/**< The identity matrix. */

	/** Gets the FMMatrix44 representation of a 3D translation.
		The translation in the x, y and z directions correspond to the \a x, 
		\a y, and \a z components of the FMVector3.
		@param translation The FMVector3 to get the translation components from.
		@return The translation FMMatrix44.  */
	static FMMatrix44 TranslationMatrix(const FMVector3& translation);

	/** Gets the FMMatrix44 representation of a 3D rotation about a given axis
		by an angle.
		@param axis The axis of rotation.
		@param angle The angle of rotation in radians.
		@return The rotation FMMatrix44. */
	static FMMatrix44 AxisRotationMatrix(const FMVector3& axis, float angle);

	/** Creates an FMMatrix44 rotation matrix of angle degrees around the X axis
		@param angle The rotation angle in radians
		@returns The rotation FMMatrix44 */
	static FMMatrix44 XAxisRotationMatrix(float angle);

	/** Creates an FMMatrix44 rotation matrix of angle degrees around the X axis
		@param angle The rotation angle in radians
		@returns The rotation FMMatrix44 */
	static FMMatrix44 YAxisRotationMatrix(float angle);

	/** Creates an FMMatrix44 rotation matrix of angle degrees around the X axis
		@param angle The rotation angle in radians
		@returns The rotation FMMatrix44 */
	static FMMatrix44 ZAxisRotationMatrix(float angle);

	/** Gets the matrix representation of a 3D euler rotation.
		The angles are considered in the order: x, y, z,
		which represents heading, banking and roll.
		@param rotation The euler rotation angles.
		@return The rotation matrix. */
	static FMMatrix44 EulerRotationMatrix(const FMVector3& rotation);

	/** Gets the FMMatrix44 representation of a 3D axis-bound scale.
		@param scale The scaling components.
		@return The scale transform. */
	static FMMatrix44 ScaleMatrix(const FMVector3& scale);

	/** Gets the FMMatrix44 represention of a look-at transformation.
		@param eye The eye position.
		@param target The target position.
		@param up The up direction.
		@return The look-at transformation matrix. */
	static FMMatrix44 LookAtMatrix(const FMVector3& eye, const FMVector3& target, const FMVector3& up);
};

/** Matrix multiplications.
	Effectively concatenates matrix transformations.
	FCollada does left-multiplication of matrices.
	@param m1 A first matrix.
	@param m2 A second matrix.
	@return The concatenation of the two matrix transformations. */
FMMatrix44 FCOLLADA_EXPORT operator*(const FMMatrix44& m1, const FMMatrix44& m2);

/** Transforms a four-dimensional vector by a given matrix.
	If the 'w' value of the vector is 1.0, this is a coordinate transformation.
	If the 'w' value of the vector is 0.0, this is a vector transformation.
	@param m A matrix.
	@param v A vector.
	@return The FMVector4 representation of the resulting vector. */
FMVector4 FCOLLADA_EXPORT operator*(const FMMatrix44& m, const FMVector4& v);

/** Scalar multiplication with float and FMMatrix44.
	All the components of the matrix get uniformly multiplied by the scalar.
	EDITOR NOTE: Is this functionality even useful? I cannot think of a reason
	this function would be used.
	@param a A scalar.
	@param m A matrix.
	@return The resulting matrix. */
FMMatrix44 FCOLLADA_EXPORT operator*(float a, const FMMatrix44& m);

/** Matrix equality comparison function.
	@param m1 A first matrix.
	@param m2 A second matrix.
	@return Whether the given matrices are equal. */
bool FCOLLADA_EXPORT IsEquivalent(const FMMatrix44& m1, const FMMatrix44& m2);
inline bool operator==(const FMMatrix44& m1, const FMMatrix44& m2) { return IsEquivalent(m1, m2); } /**< See above. */

/** Left-multiplies a given matrix, in-place.
	This function effectively concatenates a transformation into
	a transform matrix.
	@param m1 The in-place multiplied matrix.
	@param m2 A matrix transformation.
	@return The in-place multiplied matrix. */
inline FMMatrix44& operator*=(FMMatrix44& m1, const FMMatrix44& m2) { return m1 = m1 * m2; }

/** A dynamically-sized array of 4x4 matrices. */
typedef fm::vector<FMMatrix44> FMMatrix44List;

#endif // _FM_MATRIX44_H_
