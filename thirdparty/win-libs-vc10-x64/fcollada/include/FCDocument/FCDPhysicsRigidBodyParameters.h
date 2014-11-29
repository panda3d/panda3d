/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

/**
	@file FCDPhysicsRigidBodyParameters.h
	This file contains the FCDPhysicsRigidBodyParameters class.
*/

#ifndef _FCD_PHYSICS_RIGID_BODY_PARAMETERS_H_
#define _FCD_PHYSICS_RIGID_BODY_PARAMETERS_H_

#ifndef __FCD_OBJECT_H_
#include "FCDocument/FCDObject.h"
#endif // __FCD_OBJECT_H_
#ifndef _FCD_PARAMETER_ANIMATABLE_H_
#include "FCDocument/FCDParameterAnimatable.h"
#endif // _FCD_PARAMETER_ANIMATABLE_H_

class FCDPhysicsMaterial;
class FCDPhysicsShape;
class FCDPhysicsRigidBody;
class FCDPhysicsRigidBodyInstance;
class FCDEntityInstance;

/**
	A structure to hold the parameters for rigid body and rigid body instance.

	Because many of the parameters found in the rigid body can be overwritten
	by the rigid body instance, it is useful to keep it in one single place.
	This class is responsible for loading, storing, and writing these 
	parameters.

	@ingroup FCDocument
*/
class FCOLLADA_EXPORT FCDPhysicsRigidBodyParameters : public FCDObject
{
private:
	DeclareObjectType(FCDObject);

	bool ownsPhysicsMaterial; /**< This thing is stopping me from parameterizing the physics material. */
	FUTrackedPtr<FCDPhysicsMaterial> physicsMaterial;
	FUTrackedPtr<FCDEntityInstance> instanceMaterialRef;

	FCDObject* parent;

	DeclareParameterContainer(FCDPhysicsShape, physicsShape, FC("Shapes"));
	DeclareParameterAnimatable(float, FUParameterQualifiers::SIMPLE, dynamic, FC("IsDynamic"));
	DeclareParameterAnimatable(float, FUParameterQualifiers::SIMPLE, mass, FC("Mass"));
	DeclareParameter(float, FUParameterQualifiers::SIMPLE, density, FC("Density"));
	DeclareParameterAnimatable(FMVector3, FUParameterQualifiers::VECTOR, inertia, FC("Inertia"));
	DeclareParameterAnimatable(FMVector3, FUParameterQualifiers::VECTOR, massFrameTranslate, FC("Center of Mass Position"));
	DeclareParameterAnimatable(FMAngleAxis, FUParameterQualifiers::SIMPLE, massFrameOrientation, FC("Center of Mass Orientation"));

	FUTrackedPtr<FCDPhysicsRigidBody> entityOwner;
	FUTrackedPtr<FCDPhysicsRigidBodyInstance> instanceOwner;

	DeclareParameter(bool, FUParameterQualifiers::SIMPLE, isDensityMoreAccurate, FC("Use Density"));
	DeclareParameter(bool, FUParameterQualifiers::SIMPLE, isInertiaAccurate, FC("Use Inertia"));

public:
	/** Constructor. 
		@param owner The owner of this parameters holder. Its SetDirty will be
			called whenever this class is modified. It cannot be NULL. */
	FCDPhysicsRigidBodyParameters(FCDocument* document, FCDPhysicsRigidBody* owner);
	FCDPhysicsRigidBodyParameters(FCDocument* document, FCDPhysicsRigidBodyInstance* owner); /**< See above. */

	/** Destructor. */
	virtual ~FCDPhysicsRigidBodyParameters();

	/** Retrieves whether the owner is dynamic. If it is dynamic, forces like 
		gravity affect it.
		@return True if dynamic. */
	FCDParameterAnimatableFloat& GetDynamic() { return dynamic; }
	const FCDParameterAnimatableFloat& GetDynamic() const { return dynamic; } /**< See above. */
	bool IsDynamic() { return dynamic >= 0.5f; } /**< See above. */

	/** Sets whether the owner is dynamic. If it is dynamic, forces like 
		gravity affect it.
		@param dynamic True if dynamic. */
	void SetDynamic(bool dynamic);

	/** Retrieves whether density is more accurate. Because we are using an approximating algorithm for volume, density will be more accurate when
		dealing with non-analytical shapes. Density is calculated as the average density of the shapes. Shapes defining a mass will have density
		of 0.0f. A rigid body containing both shapes with only density and shapes with only mass will have both GetMass and GetDensity 
		approximated.
		@return True if density is more accurate. */
	bool IsDensityMoreAccurate() { return isDensityMoreAccurate; }

	/** [INTERNAL] Set 'DensityMoreAccurate'.
		@parame value Value to be set.
	*/
	void SetDensityMoreAccurate(bool value){ isDensityMoreAccurate = value; }

	/** Retrieves the density of the owner. The client should call IsDensityMoreAccurate to make sure this is what we want instead of mass.
		@return The density. */
	FUParameterFloat& GetDensity() { return density; }
	const FUParameterFloat& GetDensity() const { return density; } /**< See above. */

	/** [INTERNAL] Set the densisty.
		@param dens The density to set.
	*/
	void SetDensity(float dens) { density = dens; }

	/** Retrieves the mass of the owner.
		@return The mass. */
	FCDParameterAnimatableFloat& GetMass() { return mass; }
	const FCDParameterAnimatableFloat& GetMass() const { return mass; } /**< See above. */

	/** Sets the mass of the owner. 
		@param _mass The mass. */
	inline void SetMass(float _mass) { mass = _mass; parent->SetDirtyFlag(); }

	/** Retrieves the inertia of the owner.
		@return The inertia. */
	FCDParameterAnimatableVector3& GetInertia() { return inertia; }
	const FCDParameterAnimatableVector3& GetInertia() const { return inertia; } /**< See above. */

	/** Inertia is accurate only if it comes directly from the COLLADA file.
		We are using a simple sphere approximation for the inertia if there is 
		none specified. 
		@return True if the inertia is accurate. */
	bool IsInertiaAccurate() { return isInertiaAccurate; }

	/** [INTERNAL] Sets the inertia accuracy flag.
		@param value The value to set. */
	void SetInertiaAccurate(bool value){ isInertiaAccurate = value; }

	/** Sets the inertia of the owner.
		@param _inertia The inertia. */
	inline void SetInertia(const FMVector3& _inertia) 
			{ inertia = _inertia; isInertiaAccurate = true; parent->SetDirtyFlag(); }

	/** Retrieves the center of mass of the owner.
		@return The center of mass. */
	FCDParameterAnimatableVector3& GetMassFrameTranslate() { return massFrameTranslate; }
	const FCDParameterAnimatableVector3& GetMassFrameTranslate() const { return massFrameTranslate; } /**< See above. */

	/** Sets the center of mass of the owner.
		@param position The center of mass. */
	inline void SetMassFrameTranslate(const FMVector3& position) { massFrameTranslate = position; parent->SetDirtyFlag(); }
	
	/** Retrieves the orientation of mass of the owner.
		@return The orientation of mass. */
	inline FCDParameterAnimatableAngleAxis& GetMassFrameOrientation() { return massFrameOrientation; }
	inline const FCDParameterAnimatableAngleAxis& GetMassFrameOrientation() const { return massFrameOrientation; } /**< See above. */

	/** Sets the orientation of mass of the owner.
		@param angleAxis The orientation of mass. */
	inline void SetMassFrameOrientation(const FMAngleAxis& angleAxis) { massFrameOrientation = angleAxis; }

	/** Retrieves the axis of orientation of mass of the owner.
		@return The axis of orientation of mass. */
	inline FMVector3& GetMassFrameRotateAxis() { return massFrameOrientation->axis; }
	inline const FMVector3& GetMassFrameRotateAxis() const { return massFrameOrientation->axis; } /**< See above. */

	/** Sets the axis of orientation of mass of the owner.
		@param axis The axis of orientation of mass. */
	inline void SetMassFrameRotateAxis(const FMVector3& axis) { massFrameOrientation->axis = axis; parent->SetDirtyFlag(); }

	/** Retrieves the angle of orientation of mass of the owner along the axis 
		retrieved from GetMassFrameRotateAxis.
		@return The angle of orientation of mass. */
	float& GetMassFrameRotateAngle() { return massFrameOrientation->angle; }
	const float& GetMassFrameRotateAngle() const { return massFrameOrientation->angle; } /**< See above. */

	/** Sets the angle of orientation of mass of the owner along the axis
		retrieved from GetMassFrameRotateAxis.
		@param angle The angle of orientation of mass. */
	inline void SetMassFrameRotateAngle(float angle) { massFrameOrientation->angle = angle; parent->SetDirtyFlag(); }

	/** Retrives the physics material of the owner.
		@return The physics material. */
	FCDPhysicsMaterial* GetPhysicsMaterial() { return physicsMaterial; }
	const FCDPhysicsMaterial* GetPhysicsMaterial() const { return physicsMaterial; } /**< See above. */

	/** Sets the physics material of the owner.
		@param physicsMaterial The physics material. */
	void SetPhysicsMaterial(FCDPhysicsMaterial* physicsMaterial);

	/** Adds a physics material for the owner. This parameter structuer is 
		responsible for releasing the physics material. 
		@return The new physics material. */
	FCDPhysicsMaterial* AddOwnPhysicsMaterial();

	/** Retrieves the physics shapes of the owner.
		@return The physics shapes. */
	DEPRECATED(3.05A, GetPhysicsShapeCount and GetPhysicsShape(index)) void GetPhysicsShapeList() const {}

	/** Retrieves the number of physics shapes of the owner.
		@return The number of physics shapes. */
	size_t GetPhysicsShapeCount() const { return physicsShape.size(); }

	/** Retrieves a speficied physics shape of the owner by index.
		@param index The index of the physics shape.
		@return The physics shape. */
	FCDPhysicsShape* GetPhysicsShape(size_t index) { FUAssert(index < physicsShape.size(), return NULL) return physicsShape.at(index); }
	const FCDPhysicsShape* GetPhysicsShape(size_t index) const { FUAssert(index < physicsShape.size(), return NULL) return physicsShape.at(index); } /**< See above. */

	/** Adds a physics shape to the owner.
		@return The new physics shape. */
	FCDPhysicsShape* AddPhysicsShape();

	/** Copies the rigid body parameters into this parameters structure.
		@param original The original rigid body parameters to get values from. */
	virtual void CopyFrom(const FCDPhysicsRigidBodyParameters& original);

	/** [INTERNAL] Retrieves the owner of the parameters.
		@return The owner of the parameters */
	DEPRECATED(3.05A, GetParent) FCDObject* GetOwner() { return parent; }
	FCDObject* GetParent() { return parent; } /**< See above. */
	const FCDObject* GetParent() const { return parent; } /**< See above. */

	/** [INTERNAL] Retrieve the entity owner.
		@return The entity owner. */
	FCDPhysicsRigidBody* GetEntityOwner(){ return entityOwner; }

	/** [INTERNAL] Retrieve the instance owner.
		@return The instance owner. */
	FCDPhysicsRigidBodyInstance* GetInstanceOwner(){ return instanceOwner; }

	/** [INTERNAL] Determine if the model owns the maaterial.
		@return true if the model owns the material. */
	bool OwnsPhysicsMaterial() { return ownsPhysicsMaterial; }

	/** [INTERNAL] Set the material instance.
		@instance The new material instance. */
	void SetInstanceMaterial(FCDEntityInstance* instance) { instanceMaterialRef = instance; }

	/** [INTERNAL] Retrieve the material instance.
		@return The material instance. */
	FCDEntityInstance* GetInstanceMaterial(){ return instanceMaterialRef; }
};

#endif // _FCD_PHYSICS_RIGID_BODY_PARAMETERS_H_
