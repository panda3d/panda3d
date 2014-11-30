/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

/**
	@file FCDPhysicsRigidBodyInstance.h
	This file contains the FCDPhysicsRigidBodyInstance class.
*/

#ifndef _FCD_PHYSICS_RIGID_BODY_ENTITY_H_
#define _FCD_PHYSICS_RIGID_BODY_ENTITY_H_

#ifndef _FCD_ENTITY_INSTANCE_H_
#include "FCDocument/FCDEntityInstance.h"
#endif // _FCD_ENTITY_INSTANCE_H_
#ifndef _FCD_PARAMETER_ANIMATABLE_H_
#include "FCDocument/FCDParameterAnimatable.h"
#endif // _FCD_PARAMETER_ANIMATABLE_H_

class FCDocument;
class FCDSceneNode;
class FCDPhysicsRigidBody;
class FCDPhysicsRigidBodyParameters;
class FCDPhysicsMaterial;
class FCDPhysicsModelInstance;
template <class Arg1, class Arg2> class FUEvent2;


/**
	An instance of a physics rigid body.
	Allows you to overwrite the material of a rigid body
	and attach the instance to a visual scene node.

	@ingroup FCDPhysics
*/
class FCOLLADA_EXPORT FCDPhysicsRigidBodyInstance : public FCDEntityInstance
{
private:
	DeclareObjectType(FCDEntityInstance);

	FCDPhysicsModelInstance* parent;
	typedef FUEvent2<FCDPhysicsRigidBodyInstance*, const FMVector3&> CollisionEvent;
	CollisionEvent* onCollisionEvent; 

	DeclareParameterAnimatable(FMVector3, FUParameterQualifiers::VECTOR, velocity, FC("Initial Linear Velocity"));
	DeclareParameterAnimatable(FMVector3, FUParameterQualifiers::VECTOR, angularVelocity, FC("Initial Angular Velocity"));
	DeclareParameterRef(FCDPhysicsRigidBodyParameters, parameters, FC("Parameters"));
	DeclareParameterPtr(FCDSceneNode, targetNode, FC("Target Node"));


public:
	/** Constructor: do not use directly. Create new rigid bodies using the 
		FCDPhysicsModelInstance::AddRigidBodyInstance function.
		@param document The COLLADA document that contains this rigid body 
			instance. 
		@param _parent The physics model instance holding this rigid body 
			instance.
		@param body The rigid body to instance. Default values are taken from
			here. This should not be NULL unless it is expected to be filled in
			by LoadFromXML. */
	FCDPhysicsRigidBodyInstance(FCDocument* document, FCDPhysicsModelInstance* _parent, FCDPhysicsRigidBody* body);

	/** Destructor. */
	virtual ~FCDPhysicsRigidBodyInstance();

	/** Retrieves the entity type for this class. This function is part of the 
		FCDEntity interface.
		@return The entity type: PHYSICS_RIGID_BODY. */
	virtual Type GetType() const { return PHYSICS_RIGID_BODY; }

	/** [INTERNAL] Retrieve it's parent model instance.
		@return The parent of the intance.
	*/
	FCDPhysicsModelInstance* GetModelParentInstance(){ return parent; }

	/** Retrieves the parameters of tihs rigid body.
		@return The parameters. */
	FCDPhysicsRigidBodyParameters* GetParameters() { return parameters; }
	const FCDPhysicsRigidBodyParameters* GetParameters() const { return parameters; } /**< See above. */

	/** Retrieves the rigid body for this instance.
		@return The rigid body. */
	inline FCDPhysicsRigidBody* GetRigidBody() { return (FCDPhysicsRigidBody*) GetEntity(); }

	/** Retrieves the angular velocity of this rigid body instance.
		@return The angular velocity. */
	FCDParameterAnimatableVector3& GetAngularVelocity() { return angularVelocity; }
	const FCDParameterAnimatableVector3& GetAngularVelocity() const { return angularVelocity; } /**< See above. */

	/** Sets the angular velocity of this rigid body instance.
		@param _angularVelocity The angular velocity. */
	inline void SetAngularVelocity(const FMVector3& _angularVelocity) { angularVelocity = _angularVelocity; SetDirtyFlag(); }

	/** Retrives the linear velocity of this rigid body instance.
		@return The linear velocity. */
	FCDParameterAnimatableVector3& GetVelocity() { return velocity; }
	const FCDParameterAnimatableVector3& GetVelocity() const { return velocity; } /**< See above. */

	/** Sets the linear velocity of this rigid body instance.
		@param _velocity The linear velocity. */
	inline void SetVelocity(const FMVector3& _velocity) { velocity = _velocity; SetDirtyFlag(); }

	/** Retrieves the target node influenced by this rigid body instance.
		@return The target node. */
	FCDSceneNode* GetTargetNode() { return targetNode; }
	const FCDSceneNode* GetTargetNode() const { return targetNode; } /**< See above. */

	/** Sets the target node influenced by this rigid body instance.
		@param target The target node. */
	void SetTargetNode(FCDSceneNode* target) { targetNode = target;	SetDirtyFlag(); }

	/** Retrieves the collision event.
		This event may be used by a physics plugin to call an applications' callback when a collision occured on this
			rigid body. It should call it twice per collision, once for each object. The first parameter to the event
			is the rigid body instance that was collided with, and the second parameter is the world position of the 
			collision.
		@return The collision event. */
	CollisionEvent& GetCollisionEvent() { return *onCollisionEvent; }

	/** Clones the rigid body instance.
		@param clone The rigid body instance to become the clone.
			If this pointer is NULL, a new rigid body instance will be created
			and you will need to release it.
		@return The clone. */
	virtual FCDEntityInstance* Clone(FCDEntityInstance* clone = NULL) const;

	/** [INTERNAL] Sets the rigid body for this rigid body instance. Default values are
		taken from the rigid body.
		@param body The rigid body. */
	void SetRigidBody(FCDPhysicsRigidBody* body);
};

#endif // _FCD_PHYSICS_RIGID_BODY_ENTITY_H_
