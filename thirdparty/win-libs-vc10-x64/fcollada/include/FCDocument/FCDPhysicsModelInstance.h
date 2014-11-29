/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

/**
	@file FCDPhysicsModelInstance.h
	This file contains the FCDPhysicsModelInstance class.
*/

#ifndef _FCD_PHYSICS_MODEL_ENTITY_H_
#define _FCD_PHYSICS_MODEL_ENTITY_H_

#ifndef _FCD_ENTITY_INSTANCE_H_
#include "FCDocument/FCDEntityInstance.h"
#endif // _FCD_ENTITY_INSTANCE_H_

class FCDocument;
class FCDForceField;
class FCDPhysicsForceFieldInstance;
class FCDPhysicsRigidBody;
class FCDPhysicsRigidBodyInstance;
class FCDPhysicsRigidConstraint;
class FCDPhysicsRigidConstraintInstance;

/**
	A COLLADA physics model instance.

	@ingroup FCDocument
*/
class FCOLLADA_EXPORT FCDPhysicsModelInstance : public FCDEntityInstance
{
private:
	DeclareObjectType(FCDEntityInstance);
	
	DeclareParameterContainer(FCDEntityInstance, instances, FC("Inner Instances"));

public:
	/** Constructor: do not use directly. Create new physics models using the 
		FCDPhysicsModel::AddPhysicsModelInstance function.
		@param document The COLLADA document that contains this physics model
			instance. */
	FCDPhysicsModelInstance(FCDocument* document);

	/** Destructor. */
	virtual ~FCDPhysicsModelInstance();

	/** Retrieves the entity type for this class. This function is part of the 
		FCDEntity interface.
		@return The entity type: PHYSICS_MODEL. */
	virtual Type GetType() const { return FCDEntityInstance::PHYSICS_MODEL; }

	/** Retrieves all the instances from this physics model instance.
		@return The instances. */
	DEPRECATED(3.05A, GetInstanceCount and GetInstance(index)) void GetInstances() const {}

	/** Retrieves the number of instances from this physics model instance.
		@return The number of instances. */
	size_t GetInstanceCount() const { return instances.size(); }

	/** Retrieves a specified instance from this physics model instance by 
		index.
		@param index The index.
		@return The instance. */
	FCDEntityInstance* GetInstance(size_t index) { FUAssert(index < GetInstanceCount(), return NULL); return instances.at(index); }
	const FCDEntityInstance* GetInstance(size_t index) const { FUAssert(index < GetInstanceCount(), return NULL); return instances.at(index); } /**< See above. */

	/** Adds an instantiated rigid body to this physics model instance.
		@param rigidBody The rigid body to be instanced in this physics model
			instance. If this value is NULL, then there is no associated 
			rigid body for the instance yet. It should be filled in.
		@return The new rigid body instance. */
	FCDPhysicsRigidBodyInstance* AddRigidBodyInstance(FCDPhysicsRigidBody* rigidBody = NULL);

	/** Adds an instantiated rigid constraint to this physics model instance.
		@param rigidConstraint The rigid constraint to be instanced in this 
			physics model instance. If this value is NULL, then there is no 
			associated rigid constraint for the instance yet. It should be 
			filled in.
		@return The new rigid constraint instance. */
	FCDPhysicsRigidConstraintInstance* AddRigidConstraintInstance(FCDPhysicsRigidConstraint* rigidConstraint = NULL);

	/** Adds an instantiated force field to this physics model instance.
		@param forceField The force field to be instanced in this physics model
			instance. If this value is NULL, then there is no associated 
			force field for the instance yet. It should be filled in.
		@return The new force field instance. */
	FCDPhysicsForceFieldInstance* AddForceFieldInstance(FCDForceField* forceField = NULL);

	/** Removes and deletes the instance from the model.
		@param instance The instance to remove. 
		@return True if the instance was removed. */
	DEPRECATED(3.05A, instance->Release() or SAFE_RELEASE(instance)) bool RemoveInstance(FCDEntityInstance* instance);

	/** Copies the physics model instance into a clone.
		@param clone The empty clone. If this pointer is NULL, a new physics 
			model instance will be created and you will need to release the 
			returned pointer manually.
		@return The clone. */
	virtual FCDEntityInstance* Clone(FCDEntityInstance* clone = NULL) const;

	/** [INTERNAL] Cleans up the sub identifiers.
		The sub identifiers must be unique with respect to its parent. This method corrects the sub ids if there are conflicts.
		@param parentStringMap The string map from the parent of this instance in which the sub ids must be unique. */
	virtual void CleanSubId(FUSUniqueStringMap* parentStringMap);
};

#endif // _FCD_PHYSICS_MODEL_ENTITY_H_
