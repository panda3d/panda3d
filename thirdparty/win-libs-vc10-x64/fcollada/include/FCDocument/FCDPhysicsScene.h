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
	@file FCDPhysicsScene.h
	This file contains the FCDPhysicsScene class.
*/

#ifndef _FCD_PHYSICS_SCENE_NODE_
#define _FCD_PHYSICS_SCENE_NODE_

#ifndef _FCD_ENTITY_H_
#include "FCDocument/FCDEntity.h"
#endif // _FCD_ENTITY_H_

class FCDocument;
class FCDEntityInstance;
class FCDExtra;
class FCDPhysicsModel;
class FCDPhysicsModelInstance;
class FCDForceField;
class FCDPhysicsForceFieldInstance;

typedef FUObjectContainer<FCDPhysicsModelInstance> FCDPhysicsModelInstanceContainer; /**< A dynamically-sized containment array for physics model instances. */
typedef FUObjectContainer<FCDPhysicsForceFieldInstance> FCDForceFieldInstanceContainer; /**< A dynamically-sized containment array for physics force field instances. */

/**
	A COLLADA physics scene.

	A physics scene contains physics model instances that can run.

	@ingroup FCDocument
*/
class FCOLLADA_EXPORT FCDPhysicsScene : public FCDEntity
{
private:
	DeclareObjectType(FCDEntity);
	FMVector3 gravity;
	float timestep;
	FCDPhysicsModelInstanceContainer physicsModelInstances;
	FCDForceFieldInstanceContainer forceFieldInstances;

public:
	/** Constructor: do not use directly. Create new physics scenes by using
		FCDocument::AddPhysicsScene function.
		@param document The COLLADA document that contains this physics scene.
	*/
	FCDPhysicsScene(FCDocument* document);

	/** Destructor. */
	virtual ~FCDPhysicsScene();

	/** Retrieves the entity type for this class. This function is part of the 
		FCDEntity interface.
		@return The entity type: PHYSICS_SCENE_NODE. */
	virtual Type GetType() const { return PHYSICS_SCENE_NODE; }
	
	/** Retrieves the physics model instances that are a part of this physics
		scene.
		@return The physics model instances. */
	FCDPhysicsModelInstanceContainer& GetPhysicsModelInstances() { return physicsModelInstances; }
	const FCDPhysicsModelInstanceContainer& GetPhysicsModelInstances() const { return physicsModelInstances; } /**< See above. */

	/** Retrieves the number of physics model instances that are a part of this
		physics scene.
		@return The number of physics model instances. */
	size_t GetPhysicsModelInstancesCount() const { return physicsModelInstances.size(); };

	/** Retrieves a specific physics model instance that is a part of tihs
		physics scene by index.
		@param index The index of the physics model instance.
		@return The physics model instance. */
	FCDPhysicsModelInstance* GetPhysicsModelInstance(size_t index) { FUAssert(index < GetPhysicsModelInstancesCount(), return NULL); return physicsModelInstances.at(index); }
	const FCDPhysicsModelInstance* GetPhysicsModelInstance(size_t index) const { FUAssert(index < GetPhysicsModelInstancesCount(), return NULL); return physicsModelInstances.at(index); } /**< See above. */

	/** Retrieves the physics force field instances that are a part of this 
		physics scene.
		@return The physics force fieldinstances. */
	FCDForceFieldInstanceContainer& GetForceFieldInstances() { return forceFieldInstances; }
	const FCDForceFieldInstanceContainer& GetForceFieldInstances() const { return forceFieldInstances; } /**< See above. */

	/** Retrieves the number of physics force field instances that are a part 
		of this physics scene.
		@return The number of physics force field instances. */
	size_t GetForceFieldInstancesCount() const { return forceFieldInstances.size(); };

	/** Retrieves a specific physics force field instance that is a part of 
		tihs physics scene by index.
		@param index The index of the physics force field instance.
		@return The physics force field instance. */
	FCDPhysicsForceFieldInstance* GetForceFieldInstance(size_t index) { FUAssert(index < GetForceFieldInstancesCount(), return NULL); return forceFieldInstances.at(index); }
	const FCDPhysicsForceFieldInstance* GetForceFieldInstance(size_t index) const { FUAssert(index < GetForceFieldInstancesCount(), return NULL); return forceFieldInstances.at(index); } /**< See above. */

	/** Adds an instantiated physics model to this physics model.
		@param model The physics model to be instanced in this physics model. 
			If this value is NULL, then there is no associated physics model
			for the instance yet. It should be filled in.
		@return The new physics model instance. */
	FCDPhysicsModelInstance* AddPhysicsModelInstance(FCDPhysicsModel* model = NULL);

	/** Adds an instantiated force field to this physics model.
		@param forceField The force field to be instanced in this physics 
			model.
		@return The new force field instance. */
	FCDPhysicsForceFieldInstance* AddForceFieldInstance(FCDForceField* forceField = NULL);

	/** Retrieves the gravity magnitude and direction as a 3 dimensional
		vector.
		@return The gravity. */
	const FMVector3& GetGravity() const { return gravity; }

	/** Set the gravity magnitude and direction as a 3 dimensional vector.
		@param _gravity The gravity. */
	void SetGravity(const FMVector3& _gravity) { gravity = _gravity; SetDirtyFlag(); }

	/** Retrieves the integration time step, in seconds.
		@return The time step. */
	const float& GetTimestep() const { return timestep; }

	/** Set the integration time step, in seconds.
		@param _timestep The time step. */
	void SetTimestep(float _timestep) { timestep = _timestep; SetDirtyFlag(); }

	/** Copies the physics scene into a clone.
		@param clone The empty clone. If this pointer is NULL, a new physics 
			scene will be created and you will need to release the returned 
			pointer manually.
		@param cloneChildren Whether to recursively clone this entity's 
			children.
		@return The clone. */
	virtual FCDEntity* Clone(FCDEntity* clone = NULL, bool cloneChildren = false) const;

	/** [INTERNAL] Cleans up the sub identifiers.
		The sub identifiers must be unique with respect to its parent.
		This method corrects the sub ids if there are conflicts. */
	virtual void CleanSubId();
};

#endif // _FCD_SCENE_NODE_
