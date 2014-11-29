/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

/**
	@file FCDPhysicsRigidConstraint.h
	This file contains the FCDPhysicsRigidConstraint class.
*/

#ifndef _FCD_PHYSICS_RIGID_CONSTRAINT_H_
#define _FCD_PHYSICS_RIGID_CONSTRAINT_H_

#ifndef _FCD_ENTITY_H_
#include "FCDocument/FCDEntity.h"
#endif // _FCD_ENTITY_H_
#ifndef _FCD_TRANSFORM_H_
#include "FCDocument/FCDTransform.h" /** @todo Remove this include by moving the FCDTransform::Type enum to FUDaeEnum.h. */
#endif // _FCD_TRANSFORM_H_
#ifndef _FU_DAE_ENUM_H_
#include "FUtils/FUDaeEnum.h"
#endif // _FU_DAE_ENUM_H_

class FCDocument;
class FCDTransform;
class FCDPhysicsModel;
class FCDPhysicsRigidBody;
class FCDSceneNode;

typedef FUObjectContainer<FCDTransform> FCDTransformContainer; /**< A dynamically-sized containment array for transforms. */

/**
	A COLLADA rigid constraint.

	A rigid constraint attaches two rigid bodies together using springs, ball
	joints, or other limitations.

	@ingroup FCDocument
*/
class FCOLLADA_EXPORT FCDPhysicsRigidConstraint : public FCDEntity
{
private:
	DeclareObjectType(FCDEntity);
	FCDPhysicsModel* parent;

	fm::string sid;

	DeclareParameterAnimatable(float, FUParameterQualifiers::SIMPLE, enabled, FC("Enabled"));
	DeclareParameterAnimatable(float, FUParameterQualifiers::SIMPLE, interpenetrate, FC("Inter-penetrate"));

	FUTrackedPtr<FCDPhysicsRigidBody> referenceRigidBody;
	FUTrackedPtr<FCDSceneNode> referenceNode;
	FUTrackedPtr<FCDPhysicsRigidBody> targetRigidBody;
	FUTrackedPtr<FCDSceneNode> targetNode;

	DeclareParameter(FMVector3, FUParameterQualifiers::VECTOR, limitsLinearMin, FC("Minimum Linear Limit"));
	DeclareParameter(FMVector3, FUParameterQualifiers::VECTOR, limitsLinearMax, FC("Maximum Linear Limit"));
	DeclareParameter(FMVector3, FUParameterQualifiers::VECTOR, limitsSCTMin, FC("Minimum SCT? Limit"));
	DeclareParameter(FMVector3, FUParameterQualifiers::VECTOR, limitsSCTMax, FC("Maximum SCT? Limit"));

	DeclareParameter(float, FUParameterQualifiers::SIMPLE, springLinearStiffness, FC("Spring Linear Stiffness"));
	DeclareParameter(float, FUParameterQualifiers::SIMPLE, springLinearDamping, FC("Spring Linear Damping"));
	DeclareParameter(float, FUParameterQualifiers::SIMPLE, springLinearTargetValue, FC("Spring Linear Target"));

	DeclareParameter(float, FUParameterQualifiers::SIMPLE, springAngularStiffness, FC("Spring Angular Stiffness"));
	DeclareParameter(float, FUParameterQualifiers::SIMPLE, springAngularDamping, FC("Spring Angular Damping"));
	DeclareParameter(float, FUParameterQualifiers::SIMPLE, springAngularTargetValue, FC("Spring Angular Target"));

	FCDTransformContainer transformsRef;
	FCDTransformContainer transformsTar;

public:
	/** Constructor: do not use directly. Create new rigid constraints using 
		the FCDPhysicsModel::AddRigidConstraint function.
		@param document The COLLADA document that contains this rigid 
			constraint. 
		@param _parent The physics model that contains this rigid constraint. 
	*/
	FCDPhysicsRigidConstraint(FCDocument* document, FCDPhysicsModel* _parent);

	/** Destructor. */
	virtual ~FCDPhysicsRigidConstraint();

	/** Retrieves the entity type for this class. This function is part of the 
		FCDEntity interface.
		@return The entity type: PHYSICS_RIGID_CONSTRAINT. */
	virtual Type GetType() const { return PHYSICS_RIGID_CONSTRAINT; }

	/** Retrieves the physics model that contraints this rigid constraint.
		@return The physics model. */
	FCDPhysicsModel* GetParent() { return parent; }
	const FCDPhysicsModel* GetParent() const { return parent; } /**< See above. */

	/** Retrieves the wanted sub-id for this transform.
		A wanted sub-id will always be exported, even if the transform is not animated.
		But the wanted sub-id may be modified if it isn't unique within the scope.
		@return The sub-id. */
	inline const fm::string& GetSubId() const { return sid; }

	/** Sets the wanted sub-id for this transform.
		A wanted sub-id will always be exported, even if the transform is not animated.
		But the wanted sub-id may be modified if it isn't unique within the scope.
		@param subId The wanted sub-id. */
	inline void SetSubId(const fm::string& subId) { sid = CleanSubId(subId); SetDirtyFlag(); }
	inline void SetSubId(const char* subId) { sid = CleanSubId(subId); SetDirtyFlag(); } /**< See above. */

	/** Retrieves the attached rigid body which is the reference.
		@return The reference rigid body. */
	FCDPhysicsRigidBody* GetReferenceRigidBody() { return referenceRigidBody; }
	const FCDPhysicsRigidBody* GetReferenceRigidBody() const { return referenceRigidBody; } /**< See above. */

	/** Sets the attached rigid body which is the reference.
		@param _referenceRigidBody The reference rigid body. */
	void SetReferenceRigidBody(FCDPhysicsRigidBody* _referenceRigidBody) { referenceRigidBody = _referenceRigidBody; referenceNode = NULL; SetNewChildFlag(); }

	/** Retrieves the attached rigid body which is not the reference.
		@return The non reference rigid body. */
	FCDPhysicsRigidBody* GetTargetRigidBody() { return targetRigidBody; }
	const FCDPhysicsRigidBody* GetTargetRigidBody() const { return targetRigidBody; } /**< See above. */

	/** Sets the attached rigid body which is not the reference.
		@param _targetRigidBody The non reference rigid body. */
	void SetTargetRigidBody(FCDPhysicsRigidBody* _targetRigidBody) { targetRigidBody = _targetRigidBody; targetNode = NULL; SetNewChildFlag(); }

	/** Retrieves the attached node which is the reference. This method should
		be avoided as the specification says the attachment should be to a 
		rigid body. This value is only used if GetReferenceRigidBody is NULL.
		@return The attached reference node. */
	FCDSceneNode* GetReferenceNode() { return referenceNode; }
	const FCDSceneNode* GetReferenceNode() const { return referenceNode; } /**< See above. */

	/** Sets the attached node which is the reference. This method should be 
		avoided as the specification says the attachment should be to a rigid 
		body. This value is only used if GetReferenceRigidBody is NULL.
		@param _referenceNode The attached reference node. */
	void SetReferenceNode(FCDSceneNode* _referenceNode) { referenceNode = _referenceNode; referenceRigidBody = NULL; SetNewChildFlag(); }

	/** Retrieves the attached node which is not the reference. This method 
		should be avoided as the specification says the attachment should be to
		a rigid body. This value is only used if GetTargetRigidBody is NULL.
		@return The attached non reference node. */
	FCDSceneNode* GetTargetNode() { return targetNode; }
	const FCDSceneNode* GetTargetNode() const { return targetNode; } /**< See above. */
	
	/** Sets the attached node which is not the reference. This method should 
		be avoided as the specification says the attachment should be to a 
		rigid body. This value is only used if GetTargetRigidBody is NULL.
		@param _targetNode The attached non reference node. */
	void SetTargetNode(FCDSceneNode* _targetNode) { targetNode = _targetNode; targetRigidBody = NULL; SetNewChildFlag(); }

	/** Retrieves the transforms for the attached rigid body which is the 
		reference. 
		@return The transforms. */
	FCDTransformContainer& GetTransformsRef() { return transformsRef; }
	const FCDTransformContainer& GetTransformsRef() const { return transformsRef; } /**< See above. */

	/** Retrieves the transforms for the attached rigid body which is not the
		reference.
		@return The transforms. */
	FCDTransformContainer& GetTransformsTar() { return transformsTar; }
	const FCDTransformContainer& GetTransformsTar() const { return transformsTar; } /**< See above. */

	/** Adds a transform for the attached rigid body which is the reference. 
		@param type The type of transform.
		@param index The position to add the transform to. */
	FCDTransform* AddTransformRef(FCDTransform::Type type, size_t index = (size_t)-1);

	/** Adds a transform for the attached rigid body which is not the 
		reference. 
		@param type The type of transform.
		@param index The position to add the transform to. */
	FCDTransform* AddTransformTar(FCDTransform::Type type, size_t index = (size_t)-1);

	/** Retrieves whether this rigid constraint is enabled.
		@return True if enabled. */
	FCDParameterAnimatableFloat& GetEnabled() { return enabled; }
	const FCDParameterAnimatableFloat& GetEnabled() const { return enabled; } /**< See above. */
	bool IsEnabled() const { return enabled > 0.5f; } /**< See above. */

	/** Sets whether this rigid constraint is enabled. 
		@param _enabled True of enabled. */
	void SetEnabled(bool _enabled) { enabled = _enabled ? 1.0f : 0.0f; SetDirtyFlag(); }

	/** Retrieves whether the connected rigid bodies can penetrate eachother.
		@return True of they can penetrate. */
	FCDParameterAnimatableFloat& GetInterpenetrate() { return interpenetrate; }
	const FCDParameterAnimatableFloat& GetInterpenetrate() const { return interpenetrate; } /**< See above. */
	bool IsInterpenetrate() const { return interpenetrate > 0.5f; } /**< See above. */

	/** Sets whether the connected rigid bodies can penetrate eachother.
		@param _interpenetrate True of they can penetrate. */
	void SetInterpenetrate(bool _interpenetrate) { interpenetrate = _interpenetrate ? 1.0f : 0.0f; SetDirtyFlag(); }

	/** Retrieves the linear min limit of the degrees of freedom.
		@return The linear min limit. */
	FUParameterVector3& GetLimitsLinearMin() { return limitsLinearMin; }
	const FUParameterVector3& GetLimitsLinearMin() const { return limitsLinearMin; }  /**< See above. */

	/** Sets the linear min limit of the degrees of freedom.
		@param _limitsLinearMin The linear min limit. */
	void SetLimitsLinearMin(const FMVector3& _limitsLinearMin) { limitsLinearMin = _limitsLinearMin; SetDirtyFlag(); }

	/** Retrieves the linear max limit of the degrees of freedom.
		@return The linear max limit. */
	FUParameterVector3& GetLimitsLinearMax() { return limitsLinearMax; }
	const FUParameterVector3& GetLimitsLinearMax() const { return limitsLinearMax; } /**< See above. */

	/** Sets the linear max limit of the degrees of freedom.
		@param _limitsLinearMax The linear max limit. */
	void SetLimitsLinearMax(const FMVector3& _limitsLinearMax) { limitsLinearMax = _limitsLinearMax; SetDirtyFlag(); }

	/** Retrieves the swing cone and twist min limit of the degrees of freedom.
		@return The swing cone and twist min limit. */
	FUParameterVector3& GetLimitsSCTMin() { return limitsSCTMin; }
	const FUParameterVector3& GetLimitsSCTMin() const { return limitsSCTMin; } /**< See above. */

	/** Sets the swing cone and twist min limit of the degrees of freedom.
		@param _limitsSCTMin The swing cone and twist min limit. */
	void SetLimitsSCTMin(const FMVector3& _limitsSCTMin) { limitsSCTMin = _limitsSCTMin; SetDirtyFlag(); }

	/** Retrieves the swing cone and twist max limit of the degrees of freedom.
		@return The swing cone and twist max limit. */
	FUParameterVector3& GetLimitsSCTMax() { return limitsSCTMax; }
	const FUParameterVector3& GetLimitsSCTMax() const { return limitsSCTMax; } /**< See above. */

	/** Sets the swing cone and twist max limit of the degrees of freedom.
		@param _limitsSCTMax The swing cone and twist max limit. */
	void SetLimitsSCTMax(const FMVector3& _limitsSCTMax) { limitsSCTMax = _limitsSCTMax; SetDirtyFlag(); }

	/** Retrieves the spring linear stiffness of the spring rigid constraint.
		This is set to 1.0 if there is no spring.
		@return The spring linear stiffness. */
	FUParameterFloat& GetSpringLinearStiffness() { return springLinearStiffness; }
	const FUParameterFloat& GetSpringLinearStiffness() const { return springLinearStiffness; } /**< See above. */

	/** Sets the spring linear stiffness of the spring rigid constraint. This 
		is set to 1.0 if there is no spring.
		@param _springLinearStiffness The spring linear stiffness. */
	void SetSpringLinearStiffness(float _springLinearStiffness) { springLinearStiffness = _springLinearStiffness; SetDirtyFlag(); }

	/** Retrieves the spring linear damping of the spring rigid constraint.
		This is set to 0.0 if there is no spring.
		@return The spring linear damping. */
	FUParameterFloat& GetSpringLinearDamping() { return springLinearDamping; }
	const FUParameterFloat& GetSpringLinearDamping() const { return springLinearDamping; } /**< See above. */

	/** Sets the spring linear damping of the spring rigid constraint. This is 
		set to 0.0 if there is no spring.
		@param _springLinearDamping The spring linear damping. */
	void SetSpringLinearDamping(float _springLinearDamping) { springLinearDamping = _springLinearDamping; SetDirtyFlag(); }

	/** Retrieves the sping linear target value of the spring rigid constraint.
		This is set to 0.0 if there is no spring.
		@return The spring linear target value. */
	FUParameterFloat& GetSpringLinearTargetValue() { return springLinearTargetValue; }
	const FUParameterFloat& GetSpringLinearTargetValue() const { return springLinearTargetValue; } /**< See above. */

	/** Sets the sping linear target value of the spring rigid constraint. This
		is set to 0.0 if there is no spring.
		@param _springLinearTargetValue The spring linear target value. */
	void SetSpringLinearTargetValue(float _springLinearTargetValue) { springLinearTargetValue = _springLinearTargetValue; SetDirtyFlag(); }

	/** Retrieves the spring angular stiffness of the spring rigid constraint.
		This is set to 1.0 if there is no spring.
		@return The spring angular stiffness. */
	FUParameterFloat& GetSpringAngularStiffness() { return springAngularStiffness; }
	const FUParameterFloat& GetSpringAngularStiffness() const { return springAngularStiffness; } /**< See above. */

	/** Sets the spring angular stiffness of the spring rigid constraint. This 
		is set to 1.0 if there is no spring.
		@param _springAngularStiffness The spring angular stiffness. */
	void SetSpringAngularStiffness(float _springAngularStiffness) { springAngularStiffness = _springAngularStiffness; SetDirtyFlag(); }

	/** Retrieves the spring angular damping of the spring rigid constraint.
		This is set to 0.0 if there is no spring.
		@return The spring angular damping. */
	FUParameterFloat& GetSpringAngularDamping() { return springAngularDamping; }
	const FUParameterFloat& GetSpringAngularDamping() const { return springAngularDamping; } /**< See above. */

	/** Sets the spring angular damping of the spring rigid constraint. This is
		set to 0.0 if there is no spring.
		@param _springAngularDamping The spring angular damping. */
	void SetSpringAngularDamping(float _springAngularDamping) { springAngularDamping = _springAngularDamping; SetDirtyFlag(); }

	/** Retrieves the sping angular target value of the spring rigid 
		constraint. This is set to 0.0 if there is no spring.
		@return The spring angular target value. */
	FUParameterFloat& GetSpringAngularTargetValue() { return springAngularTargetValue; }
	const FUParameterFloat& GetSpringAngularTargetValue() const { return springAngularTargetValue; } /**< See above. */

	/** Sets the sping angular target value of the spring rigid constraint. 
		This is set to 0.0 if there is no spring.
		@param _springAngularTargetValue The spring angular target value. */
	void SetSpringAngularTargetValue(float _springAngularTargetValue) { springAngularTargetValue = _springAngularTargetValue; SetDirtyFlag(); }

	/** Retrieves the animated value for enabled.
		@returns The animated value, or NULL if enabled is not animated. */
	DEPRECATED(3.05A, GetEnabled().GetAnimated) FCDAnimated* GetAnimatedEnabled() { return GetEnabled().GetAnimated(); }
	DEPRECATED(3.05A, GetEnabled().GetAnimated) const FCDAnimated* GetAnimatedEnabled() const { return GetEnabled().GetAnimated(); } /**< See above. */

	/** Retrieves the animated value for interpenetrate.
		@returns The animated value, or NULL if interpenetrate is not animated. */
	DEPRECATED(3.05A, GetInterpenetrate().GetAnimated) FCDAnimated* GetAnimatedInterpenetrate() { return GetInterpenetrate().GetAnimated(); }
	DEPRECATED(3.05A, GetInterpenetrate().GetAnimated) const FCDAnimated* GetAnimatedInterpenetrate() const { return GetInterpenetrate().GetAnimated(); } /**< See above. */

	/** Copies the rigid constraint into a clone.
		@param clone The empty clone. If this pointer is NULL, a new rigid
			constraint will be created and you will need to release the 
			returned pointer manually.
		@param cloneChildren Whether to recursively clone this entity's children.
		@return The clone. */
	virtual FCDEntity* Clone(FCDEntity* clone = NULL, bool cloneChildren = false) const;
};

#endif // _FCD_PHYSICS_RIGID_CONSTRAINT_H_
