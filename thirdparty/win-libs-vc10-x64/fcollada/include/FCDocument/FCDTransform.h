/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/
/*
	Based on the FS Import classes:
	Copyright (C) 2005-2006 Feeling Software Inc
	Copyright (C) 2005-2006 Autodesk Media Entertainment
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

/**
	@file FCDTransform.h
	This file contains the FCDTransform class and its up-classes:
	FCDTTranslation, FCDTScale, FCDTRotation, FCDTMatrix, FCDTLookAt and FCDTSkew.
*/

#ifndef _FCD_TRANSFORM_H_
#define _FCD_TRANSFORM_H_

class FCDocument;
class FCDAnimated;
class FCDSceneNode;
class FCDTransform;

#ifndef __FCD_OBJECT_H_
#include "FCDocument/FCDObject.h"
#endif // __FCD_OBJECT_H_
#ifndef _FM_QUATERNION_H_
#include "FMath/FMQuaternion.h"
#endif // _FM_QUATERNION_H_
#ifndef _FCD_PARAMETER_ANIMATABLE_H_
#include "FCDocument/FCDParameterAnimatable.h"
#endif // _FCD_PARAMETER_ANIMATABLE_H_

/**
	A COLLADA transform.

	COLLADA supports six transformation types: translations(FCDTTranslation),
	rotations(FCDTRotation), scales(FCDTScale), matrices(FCDTMatrix),
	skews(FCDTSkew) and the 'look-at' transform(FCDTLookAt).

	@ingroup FCDocument
*/
class FCOLLADA_EXPORT FCDTransform : public FCDObject
{
public:
	/** The COLLADA transform types. */
	enum Type
	{
		TRANSLATION, /**< A translation(FCDTTranslation). */
		ROTATION, /**< A rotation(FCDTRotation). */
		SCALE, /**< A non-uniform scale(FCDTScale). */
		MATRIX, /**< A matrix multiplication(FCDTMatrix). */
		LOOKAT, /**< A targeted, 'look-at' transformation(FCDTLookAt). */
		SKEW, /**< A skew(FCDTSkew). */
		TYPE_COUNT
	};

private:
	DeclareObjectType(FCDObject);
	FCDSceneNode* parent;
	DeclareParameter(fm::string, FUParameterQualifiers::SIMPLE, sid, FC("Sub-id"));

public:
	/** Constructor: do not use directly.
		Instead, use the FCDSceneNode::AddTransform function.
		@param document The COLLADA document that owns the transform.
		@param parent The visual scene node that contains the transform.
			Set this pointer to NULL if this transform is not owned by a
			visual scene node. */
	FCDTransform(FCDocument* document, FCDSceneNode* parent);

	/** Destructor. */
	virtual ~FCDTransform();

	/** Retrieves the visual scene node that contains this transformation.
		@return The parent visual scene node. This pointer will be NULL
			if the transformation is not contained by a visual scene node. */
	FCDSceneNode* GetParent() { return parent; }
	const FCDSceneNode* GetParent() const { return parent; } /**< See above. */

	/** [DEPRECATED] Sets on a scene node parent, the transform dirty flag. */
	DEPRECATED(3.05, SetValueChange)
	void SetTransformsDirtyFlag();

	/** Creates a copy of a transformation.
		@param clone The transform that will be the clone.
		@return The cloned transformation. */
	virtual FCDTransform* Clone(FCDTransform* clone = NULL) const = 0;

	/** Retrieves the class type of the transformation.
		The class type should be used to up-case the transformation pointer.
		@return The class type. */
	virtual Type GetType() const = 0;

	/** Converts the transformation into a matrix.
		Useful for visual scene nodes with a weird transformation stack.
		@return A matrix equivalent of the transformation. */
	virtual FMMatrix44 ToMatrix() const = 0;

	/** Retrieves the wanted sub-id for this transform.
		A wanted sub-id will always be exported, even if the transform is not animated.
		But the wanted sub-id may be modified if it isn't unique within the scope.
		@return The sub-id. */
	inline FUParameterString& GetSubId() { return sid; }
	inline const FUParameterString& GetSubId() const { return sid; } /**< See above. */

	/** Sets the wanted sub-id for this transform.
		A wanted sub-id will always be exported, even if the transform is not animated.
		But the wanted sub-id may be modified if it isn't unique within the scope.
		@param subId The wanted sub-id. */
	void SetSubId(const fm::string& subId);
	
	/** Retrieves whether this transformation has an animation tied to its values.
		@return Whether the transformation is animated. */
	virtual bool IsAnimated() const = 0;

	/** Retrieves the animated element for the transformation.
		@return The animated element. This pointer will be NULL if the transformation
			is not animated. */
	inline FCDAnimated* GetAnimated() { return const_cast<FCDAnimated*>(const_cast<const FCDTransform*>(this)->GetAnimated()); }
	virtual const FCDAnimated* GetAnimated() const = 0; /**< See above. */

	/** Retrieves whether a given transformation is the exact opposite of
		this transformation. Executing two opposite transformations, one after the
		other will not give any resulting transformation. This function is useful
		to detect pivots within the transform stack.
		@param transform A second transformation.
		@return Whether the two transformations are opposites. */
	virtual bool IsInverse(const FCDTransform* transform) const;

	/** Set Value changed flag.  When this happens, notify our parent */
	virtual void SetValueChange();
};

/**
	A COLLADA translation.
	A translation is a simple 3D displacement.

	@ingroup FCDocument
*/
class FCOLLADA_EXPORT FCDTTranslation : public FCDTransform
{
private:
	DeclareObjectType(FCDTransform);
	DeclareParameterAnimatable(FMVector3, FUParameterQualifiers::VECTOR, translation, FC("Translation"));

public:
	/** Constructor: do not use directly.
		Instead, use the FCDSceneNode::AddTransform function with
		the TRANSLATION transformation type.
		@param document The COLLADA document that owns the translation.
		@param parent The visual scene node that contains the translation.
			Set this pointer to NULL if the translation is not owned
			by a visual scene node. */
	FCDTTranslation(FCDocument* document, FCDSceneNode* parent);
	
	/** Destructor. */
	virtual ~FCDTTranslation();

	/** Retrieves the transformation class type for the translation.
		@return The transformation class type: TRANSLATION. */
	virtual Type GetType() const { return TRANSLATION; }

	/** Retrieves the translation 3D displacement vector.
		This displacement vector may be animated.
		@return The displacement vector. */
	inline FCDParameterAnimatableVector3& GetTranslation() { return translation; }
	inline const FCDParameterAnimatableVector3& GetTranslation() const { return translation; } /**< See above. */

	/** Sets the translation 3D displacement vector.
		@param _translation The displacement vector. */
	inline void SetTranslation(const FMVector3& _translation) { translation = _translation; SetValueChange(); }

	/** Sets the translation 3D displacement vector.
		@param x The x-component displacement.
		@param y The y-component displacement.
		@param z The z-component displacement. */
	inline void SetTranslation(float x, float y, float z) { translation = FMVector3(x, y, z); SetValueChange(); }

	/** Converts the translation into a matrix.
		@return A matrix equivalent of the translation. */
	virtual FMMatrix44 ToMatrix() const;

	/** Retrieves whether this translation is affected by an animation.
		@return Whether the translation is animated. */
	virtual bool IsAnimated() const;

	/** Retrieves the animated element for the translation.
		@return The animated element. This pointer will be NULL if the translation
			is not animated. */
	inline FCDAnimated* GetAnimated() { return Parent::GetAnimated(); }
	virtual const FCDAnimated* GetAnimated() const; /**< See above. */

	/** Retrieves whether a given transform is the exact opposite of
		this translation. The opposite of a translation has a displacement
		vector with all the components multiplied by -1.
		@param transform A second transformation.
		@return Whether the two transformations are opposites. */
	virtual bool IsInverse(const FCDTransform* transform) const;

	/** Creates a copy of a translation.
		@param clone The transform that will be the clone.
		@return The cloned transformation. */
	virtual FCDTransform* Clone(FCDTransform* clone = NULL) const;
};

/**
	A COLLADA non-uniform scale.
	A non-uniform scale contains three scale factors.
	@ingroup FCDocument
*/
class FCOLLADA_EXPORT FCDTScale : public FCDTransform
{
private:
	DeclareObjectType(FCDTransform);
	DeclareParameterAnimatable(FMVector3, FUParameterQualifiers::VECTOR, scale, FC("Scale"));

public:
	/** Constructor: do not use directly.
		Instead, use the FCDSceneNode::AddTransform function with
		the SCALE transformation type.
		@param document The COLLADA document that owns the non-uniform scale.
		@param parent The visual scene node that contains the non-uniform scale.
			Set this pointer to NULL if the non-uniform scale is not owned
			by a visual scene node. */
	FCDTScale(FCDocument* document, FCDSceneNode* parent);

	/** Destructor. */
	virtual ~FCDTScale();

	/** Retrieves the transformation class type for the non-uniform scale.
		@return The class type: SCALE. */
	virtual Type GetType() const { return SCALE; }

	/** Retrieves the factors of the non-uniform scale.
		These factors may be animated.
		@return The scale factors. */
	inline FCDParameterAnimatableVector3& GetScale() { return scale; }
	inline const FCDParameterAnimatableVector3& GetScale() const { return scale; } /**< See above. */

	/** Sets the factors of the non-uniform scale.
		@param _scale The scale factors. */
	inline void SetScale(const FMVector3& _scale) { scale = _scale; SetValueChange(); }

	/** Sets the factors of the non-uniform scale.
		@param x The x-component scale factor.
		@param y The y-component scale factor.
		@param z The z-component scale factor. */
	inline void SetScale(float x, float y, float z) { scale = FMVector3(x, y, z); SetValueChange(); }

	/** Converts the non-uniform scale into a matrix.
		@return A matrix equivalent of the non-uniform scale. */
	virtual FMMatrix44 ToMatrix() const;

	/** Retrieves whether the factors of the non-uniform scale are animated.
		@return Whether the scale factors are animated. */
	virtual bool IsAnimated() const;

	/** Retrieves the animated element for the non-uniform scale factors.
		@return The animated element. This pointer will be NULL if the
			scale factors are not animated. */
	inline FCDAnimated* GetAnimated() { return Parent::GetAnimated(); }
	virtual const FCDAnimated* GetAnimated() const; /**< See above. */

	/** Creates a copy of a non-uniform scale.
		@param clone The transform that will be the clone.
		@return The cloned transformation. */
	virtual FCDTransform* Clone(FCDTransform* clone = NULL) const;
};

/**
	A COLLADA angle-axis rotation.
	This rotation defines an axis around which the 3D points
	are rotated by a given angle.
	@todo (clock-wise/counter-clock-wise?)
	@ingroup FCDocument
*/
class FCOLLADA_EXPORT FCDTRotation : public FCDTransform
{
private:
	DeclareObjectType(FCDTransform);
	DeclareParameterAnimatable(FMAngleAxis, FUParameterQualifiers::SIMPLE, angleAxis, FC("Angle-axis"));

public:
	/** Constructor: do not use directly.
		Instead, use the FCDSceneNode::AddTransform function with
		the transformation type: ROTATION.
		@param document The COLLADA document that owns the rotation.
		@param parent The visual scene node that contains the rotation.
			Set this pointer to NULL if the rotation is not owned
			by a visual scene node. */
	FCDTRotation(FCDocument* document, FCDSceneNode* parent);
	
	/** Destructor. */
	virtual ~FCDTRotation();

	/** Retrieves the transformation class type for the rotation.
		@return The class type: ROTATION. */
	virtual Type GetType() const { return ROTATION; }

	/** Retrieves the angle-axis value of the rotation.
		@return The angle-axis value. */
	inline FCDParameterAnimatableAngleAxis& GetAngleAxis() { return angleAxis; }
	inline const FCDParameterAnimatableAngleAxis& GetAngleAxis() const { return angleAxis; } /**< See above. */

	/** Sets the angle-axis value of the rotation.
		@param aa The new angle-axis value. */
	inline void SetAngleAxis(const FMAngleAxis& aa) { angleAxis = aa; SetValueChange(); }

	/** Retrieves the rotation axis.
		This 3D vector may be animated.
		@return The rotation axis. */
	inline FMVector3& GetAxis() { return angleAxis->axis; }
	inline const FMVector3& GetAxis() const { return angleAxis->axis; } /**< See above. */

	/** Sets the rotation axis.
		@param axis The rotation axis. */
	inline void SetAxis(const FMVector3& axis) { angleAxis->axis = axis; SetValueChange(); }

	/** Sets the rotation axis.
		@param x The x-component of the rotation axis.
		@param y The y-component of the rotation axis.
		@param z The z-component of the rotation axis. */
	inline void SetAxis(float x, float y, float z) { angleAxis->axis = FMVector3(x, y, z); SetValueChange(); }

	/** Retrieves the rotation angle.
		This angle may be animated.
		@return The rotation angle, in degrees. */
	inline float& GetAngle() { return angleAxis->angle; }
	inline const float& GetAngle() const { return angleAxis->angle; } /**< See above. */

	/** Sets the rotation angle.
		@param a The rotation angle, in degrees. */
	inline void SetAngle(float a) { angleAxis->angle = a; SetValueChange(); }

	/** Sets the rotation components
		@param axis The rotation axis.
		@param angle The rotation angle, in degrees. */
	inline void SetRotation(const FMVector3& axis, float angle) { angleAxis = FMAngleAxis(axis, angle); SetValueChange(); }

	/** Retrieves the rotation orientation.
		@return The rotation orientation quaternion. */
	inline FMQuaternion GetOrientation() { return FMQuaternion(angleAxis->axis, FMath::DegToRad(angleAxis->angle)); }

	/** Sets the rotation orientation.
		@param q The rotation orientation quaternion. */
	inline void SetOrientation(const FMQuaternion& q) { q.ToAngleAxis(angleAxis->axis, angleAxis->angle); angleAxis->angle = FMath::RadToDeg(angleAxis->angle); SetValueChange(); }

	/** Converts the rotation into a matrix.
		@return A matrix equivalent of the rotation. */
	virtual FMMatrix44 ToMatrix() const;

	/** Retrieves whether the axis or the angle of the rotation are animated.
		@return Whether the rotation is animated. */
	virtual bool IsAnimated() const;

	/** Retrieves the animated element for the angle-axis rotation.
		@see FCDAnimatedAngleAxis
		@return The animated element. This pointer will be NULL if the
			rotation is not animated. */
	inline FCDAnimated* GetAnimated() { return Parent::GetAnimated(); }
	virtual const FCDAnimated* GetAnimated() const; /**< See above. */

	/** Retrieves whether a given transform is the exact opposite of
		this rotation. The opposite of an angle-axis rotation has the
		same axis as this rotation but the angle is multiplied by -1.
		@param transform A second transformation.
		@return Whether the two rotation are opposites. */
	virtual bool IsInverse(const FCDTransform* transform) const;

	/** Creates a copy of a rotation.
		@param clone The transform that will be the clone.
		@return The cloned transformation. */
	virtual FCDTransform* Clone(FCDTransform* clone = NULL) const;
};

/**
	A COLLADA matrix transformation.
	This transformation contains a matrix that should be
	multiplied to the local transformation matrix.
	@ingroup FCDocument
*/
class FCOLLADA_EXPORT FCDTMatrix : public FCDTransform
{
private:
	DeclareObjectType(FCDTransform);
	DeclareParameterAnimatable(FMMatrix44, FUParameterQualifiers::SIMPLE, transform, FC("Transform"));

public:
	/** Constructor: do not use directly.
		Instead, use the FCDSceneNode::AddTransform function with
		the transformation type: MATRIX.
		@param document The COLLADA document that owns the transformation.
		@param parent The visual scene node that contains the transformation. */
	FCDTMatrix(FCDocument* document, FCDSceneNode* parent);

	/** Destructor. */
	virtual ~FCDTMatrix();
	
	/** Retrieves the transformation class type for the transformation.
		@return The class type: MATRIX. */
	virtual Type GetType() const { return MATRIX; }

	/** Retrieves the matrix for the transformation.
		All 16 values of the matrix may be animated.
		@return The transformation matrix. */
	inline FCDParameterAnimatableMatrix44& GetTransform() { return transform; }
	inline const FCDParameterAnimatableMatrix44& GetTransform() const { return transform; } /**< See above. */

	/** Sets the matrix for the transformation.
		@param mx The transformation matrix. */
	inline void SetTransform(const FMMatrix44& mx) { transform = mx; SetValueChange(); }

	/** Converts the transformation into a matrix.
		For matrix transformations, that's simply the transformation matrix.
		@return The transformation matrix. */
	virtual FMMatrix44 ToMatrix() const { return transform; }

	/** Retrieves whether the transformation matrix is animated.
		@return Whether the transformation matrix is animated. */
	virtual bool IsAnimated() const;

	/** Retrieves the animated element for the transformation matrix.
		@see FCDAnimatedMatrix
		@return The animated element. This pointer will be NULL if the
			transformation matrix is not animated. */
	inline FCDAnimated* GetAnimated() { return Parent::GetAnimated(); }
	virtual const FCDAnimated* GetAnimated() const; /**< See above. */

	/** Creates a copy of a matrix transformation.
		@param clone The transform that will be the clone.
		@return The cloned transformation. */
	virtual FCDTransform* Clone(FCDTransform* clone = NULL) const;
};

/**
	A COLLADA 'look-at' transformation.
	This transformation type fully defines a position
	and an orientation with a 3D world by using three
	3D vectors: the viewer's position, the position
	that the viewer is looking at, and the up-vector
	for camera rolls. */
class FCOLLADA_EXPORT FCDTLookAt : public FCDTransform
{
private:
	DeclareObjectType(FCDTransform);
	DeclareParameterAnimatable(FMLookAt, FUParameterQualifiers::SIMPLE, lookAt, FC("LookAt"));

public:
	/** Constructor: do not use directly.
		Instead, use the FCDSceneNode::AddTransform function with
		the transformation type: LOOKAT.
		@param document The COLLADA document that owns the transformation.
		@param parent The visual scene node that contains the transformation. */
	FCDTLookAt(FCDocument* document, FCDSceneNode* parent);

	/** Destructor. */
	virtual ~FCDTLookAt();

	/** Retrieves the transformation class type for the transformation.
		@return The class type: LOOKAT. */
	virtual Type GetType() const { return LOOKAT; }

	/** Retrieves the look-at value for this transformation.
		@return The local look-at. */
	inline FCDParameterAnimatableLookAt& GetLookAt() { return lookAt; }
	inline const FCDParameterAnimatableLookAt& GetLookAt() const { return lookAt; } /**< See above. */

	/** Sets the look-at value for this transformation.
		@param _lookAt A look-at. */
	inline void SetLookAt(const FMLookAt& _lookAt) { lookAt = _lookAt; SetValueChange(); }

	/** Retrieves the viewer's position.
		@return The viewer's position. */
	FMVector3& GetPosition() { return lookAt->position; }
	const FMVector3& GetPosition() const { return lookAt->position; } /**< See above. */

	/** Sets the viewer's position.
		@param pos The viewer's position. */
	inline void SetPosition(const FMVector3& pos) { lookAt->position = pos; SetValueChange(); }

	/** Sets the viewer's position.
		@param x The x-component of the position.
		@param y The y-component of the position.
		@param z The z-component of the position. */
	inline void SetPosition(float x, float y, float z) { lookAt->position = FMVector3(x, y, z); SetValueChange(); }

	/** Retrieves the position that the viewer is looking at.
		@return The viewer's target. */
	FMVector3& GetTarget() { return lookAt->target; }
	const FMVector3& GetTarget() const { return lookAt->target; } /**< See above. */

	/** Sets the position that the viewer is looking at.
		@param target The target position. */
	inline void SetTarget(const FMVector3& target) { lookAt->target = target; SetValueChange(); }

	/** Sets the position that the viewer is looking at.
		@param x The x-component of the target position.
		@param y The y-component of the target position.
		@param z The z-component of the target position. */
	inline void SetTarget(float x, float y, float z) { lookAt->target = FMVector3(x, y, z); SetValueChange(); }

	/** Retrieves the viewer's up-vector.
		@return The up-vector. */
	FMVector3& GetUp() { return lookAt->up; }
	const FMVector3& GetUp() const { return lookAt->up; } /**< See above. */

	/** Sets the viewer's up-vector.
		@param up The up-vector. */
	inline void SetUp(const FMVector3& up) { lookAt->up = up; SetValueChange(); }

	/** Sets the viewer's up-vector.
		@param x The x-component of the up-vector.
		@param y The y-component of the up-vector.
		@param z The z-component of the up-vector. */
	inline void SetUp(float x, float y, float z) { lookAt->up = FMVector3(x, y, z); SetValueChange(); }

	/** Converts the transformation into a matrix.
		@return The transformation matrix. */
	virtual FMMatrix44 ToMatrix() const;

	/** Retrieves whether the transformation is animated.
		FCollada now supports animated look-at transforms, through the FCDAnimatedCustom.
		@return Whether the transform is animated. */
	virtual bool IsAnimated() const;

	/** Retrieves the animated element for the transformation matrix.
		FCollada now supports animated look-at transforms.
		@see FCDAnimatedCustom
		@return the animated element, containing 9 values. */
	inline FCDAnimated* GetAnimated() { return Parent::GetAnimated(); }
	virtual const FCDAnimated* GetAnimated() const; /**< See above. */

	/** Creates a copy of a look-at transformation.
		@param clone The transform that will be the clone.
		@return The cloned transformation. */
	virtual FCDTransform* Clone(FCDTransform* clone = NULL) const;
};

/**
	A COLLADA skew.
	In COLLADA, the skew transformation follows the Renderman convention.
	A skew is defined by two axis and one angle: the axis which is rotated, the axis around
	which the rotation is done and the angle of the rotation.
	@ingroup FCDocument
*/
class FCOLLADA_EXPORT FCDTSkew : public FCDTransform
{
private:
	DeclareObjectType(FCDTransform);
	DeclareParameterAnimatable(FMSkew, FUParameterQualifiers::SIMPLE, skew, FC("Skew"));

public:
	/** Constructor: do not use directly.
		Instead, use the FCDSceneNode::AddTransform function with
		the transformation type: SKEW.
		@param document The COLLADA document that owns the skew.
		@param parent The visual scene node that contains the skew. */
	FCDTSkew(FCDocument* document, FCDSceneNode* parent);

	/** Destructor. */
	virtual ~FCDTSkew();

	/** Retrieves the transformation class type for the transformation.
		@return The class type: SKEW. */
	virtual Type GetType() const { return SKEW; }
	
	/** Retrieves the skew value for this transformation.
		@return The skew value. */
	inline FCDParameterAnimatableSkew& GetSkew() { return skew; }
	inline const FCDParameterAnimatableSkew& GetSkew() const { return skew; }

	/** Sets the skew value for this transformation.
		@param _skew A skew value. */
	inline void SetSkew(const FMSkew& _skew) { skew = _skew; SetValueChange(); }

	/** Retrieves the axis which is rotated.
		@return The rotated axis. */
	const FMVector3& GetRotateAxis() const { return skew->rotateAxis; }
	FMVector3& GetRotateAxis() { return skew->rotateAxis; } /**< See above. */

	/** Sets the axis which is rotated.
		@param axis The rotated axis. */
	inline void SetRotateAxis(const FMVector3& axis) { skew->rotateAxis = axis; SetValueChange(); }

	/** Retrieves the axis around which the rotation is done.
		@return The rotation axis. */
	inline const FMVector3& GetAroundAxis() const { return skew->aroundAxis; }
	inline FMVector3& GetAroundAxis() { return skew->aroundAxis; } /**< See above. */

	/** Sets the axis around which the rotation is done.
		@param axis The rotation axis. */
	inline void SetAroundAxis(const FMVector3& axis) { skew->aroundAxis = axis; SetValueChange(); }

	/** Retrieves the rotation angle.
		@return The rotation angle. */
	inline const float& GetAngle() const { return skew->angle; }
	inline float& GetAngle() { return skew->angle; } /**< See above. */

	/** Sets the rotation angle.
		@param _angle The rotation angle. */
	inline void SetAngle(float angle) { skew->angle = angle; SetValueChange(); }

	/** Converts the skew into a matrix.
		@return The transformation matrix. */
	virtual FMMatrix44 ToMatrix() const;

	/** Retrieves whether the transformation is animated.
		@return FCollada doesn't support animated skews: false. */
	virtual bool IsAnimated() const;

	/** Retrieves the animated element for the skew.
		@return FCollada doesn't support animated skews: NULL. */
	inline FCDAnimated* GetAnimated() { return Parent::GetAnimated(); }
	virtual const FCDAnimated* GetAnimated() const; /**< See above. */

	/** Creates a copy of a skew transformation.
		@param clone The transform that will be the clone.
		@return The cloned transformation. */
	virtual FCDTransform* Clone(FCDTransform* clone = NULL) const;
};

/**
	[INTERNAL] A factory for COLLADA transforms.
	Creates the correct transform object for a given transform type/XML tree node.
	To create new transforms, use the FCDSceneNode::AddTransform function.
*/
class FCOLLADA_EXPORT FCDTFactory
{
private:
	FCDTFactory() {} // Static class: do not instantiate.

public:
	/** Creates a new COLLADA transform, given a transform type.
		@param document The COLLADA document that will own the new transform.
		@param parent The visual scene node that will contain the transform.
		@param type The type of transform object to create.
		@return The new COLLADA transform. This pointer will be NULL
			if the given type is invalid. */
	static FCDTransform* CreateTransform(FCDocument* document, FCDSceneNode* parent, FCDTransform::Type type);
};

#endif // _FR_TRANSFORM_H_
