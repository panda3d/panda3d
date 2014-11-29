/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

/**
	@file FCDPhysicsRigidConstraintInstance.h
	This file contains the FCDPhysicsRigidConstraintInstance class.
*/

#ifndef _FCD_PHYSICS_RIGID_CONSTRAINT_INSTANCE_H_
#define _FCD_PHYSICS_RIGID_CONSTRAINT_INSTANCE_H_

#ifndef _FCD_ENTITY_INSTANCE_H_
#include "FCDocument/FCDEntityInstance.h"
#endif // _FCD_ENTITY_INSTANCE_H_

class FCDocument;
class FCDEntity;
class FCDSceneNode;
class FCDPhysicsModel;
class FCDPhysicsModelInstance;
class FCDPhysicsRigidConstraint;

/**
	A COLLADA rigid constraint instance.

	They are instantiated in the instance_physics_model node. The only 
	documentation on them is that it allows overriding of properties.

	@ingroup FCDocument
*/
class FCOLLADA_EXPORT FCDPhysicsRigidConstraintInstance : public FCDEntityInstance
{
private:
	DeclareObjectType(FCDEntityInstance);
	FCDPhysicsModelInstance* parent;

public:
	/** Constructor: do not use directly. Create new rigid constraints 
		instances using the FCDPhysicsModel::AddRigidConstraintInstance
		function.
		@param document The COLLADA document that contains this rigid 
			constraint instance. 
		@param parent The physics model instance that contains this rigid 
			constraint instance. 
		@param constraint The rigid constraint to instance. This should not be
			NULL unless it is expected to be filled in by LoadFromXML. */
	FCDPhysicsRigidConstraintInstance(FCDocument* document, FCDPhysicsModelInstance* parent, FCDPhysicsRigidConstraint* constraint);

	/** Destructor. */
	virtual ~FCDPhysicsRigidConstraintInstance();

	/** Retrieves the entity type for this class. This function is part of the 
		FCDEntity interface.
		@return The entity type: PHYSICS_RIGID_CONSTRAINT. */
	virtual Type GetType() const { return PHYSICS_RIGID_CONSTRAINT; }

	/** Retrieves the physics model instance that contraints this rigid 
		constraint.
		@return The physics model instance. */
	FCDPhysicsModelInstance* GetParent() { return parent; }
	const FCDPhysicsModelInstance* GetParent() const { return parent; } /**< See above. */

	/** Retrieves the rigid constraint for this rigid constraint instance.
		@return The rigid constraint. */
	inline FCDPhysicsRigidConstraint* GetRigidConstraint() { return (FCDPhysicsRigidConstraint*) GetEntity(); }

	/** Clones the rigid constraint instance.
		@param clone The rigid constraint instance to become the clone.
			If this pointer is NULL, a new rigid constraint instance will be 
			created and you will need to release it.
		@return The clone. */
	virtual FCDEntityInstance* Clone(FCDEntityInstance* clone = NULL) const;

	/** [INTERNAL] Retrieve it's parent model instance.
		@return The parent of the intance.
	*/
	FCDPhysicsModelInstance* GetModelParentInstance(){ return parent; }

	/** [INTERNAL] Sets the rigid body for this rigid body instance. Default values are
		taken from the rigid body.
		@param body The rigid body. */
	void SetRigidConstraint(FCDPhysicsRigidConstraint* constraint);
};

#endif // _FCD_PHYSICS_RIGID_CONSTRAINT_INSTANCE_H_
