/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

/**
	@file FCDPhysicsModel.h
	This file contains the FCDPhysicsModel class.
*/

#ifndef _FCD_PHYSICSMODEL_H_
#define _FCD_PHYSICSMODEL_H_

#ifndef _FCD_ENTITY_H_
#include "FCDocument/FCDEntity.h"
#endif // _FCD_ENTITY_H_
#ifndef _FU_URI_H_
#include "FUtils/FUUri.h"
#endif // _FU_URI_H_

class FCDocument;
class FCDPhysicsRigidBody;
class FCDPhysicsRigidConstraint;
class FCDPhysicsModelInstance;
class FUUri;

typedef FUObjectContainer<FCDPhysicsModelInstance> FCDPhysicsModelInstanceContainer; /**< A dynamically-sized containment array for physics mode instances. */
typedef FUObjectContainer<FCDPhysicsRigidBody> FCDPhysicsRigidBodyContainer; /**< A dynamically-sized containment array for rigid bodies. */
typedef FUObjectContainer<FCDPhysicsRigidConstraint>  FCDPhysicsRigidConstraintContainer; /**< A dynamically-sized containment array for rigid constraints. */

/**
	A COLLADA physics model.

	A physics model can contain rigid bodies, rigid constraints, or instances 
	of previously defined physics models. For example, a house physics model
	could contain a number of instantiated physics models such as walls.

	@ingroup FCDocument
*/
class FCOLLADA_EXPORT FCDPhysicsModel : public FCDEntity
{
private:
	typedef fm::map<xmlNode*, FUUri> ModelInstanceNameNodeMap;

	DeclareObjectType(FCDEntity);
	FCDPhysicsModelInstanceContainer instances;
	FCDPhysicsRigidBodyContainer rigidBodies;
	FCDPhysicsRigidConstraintContainer rigidConstraints;
	ModelInstanceNameNodeMap modelInstancesMap;

public:
	/** Constructor: do not use directly. Create new physics models using the 
		FCDLibrary::AddEntity function.
		@param document The COLLADA document that contains this physics model.
	*/
	FCDPhysicsModel(FCDocument* document);

	/** Destructor. */
	virtual ~FCDPhysicsModel();

	/** Retrieves the entity type for this class. This function is part of the 
		FCDEntity interface.
		@return The entity type: PHYSICS_MODEL. */
	virtual Type GetType() const { return FCDEntity::PHYSICS_MODEL; }

	/** Retrieves the instantiated physics models that are part of this physics
		model.
		@return The physics model instances. */
	FCDPhysicsModelInstanceContainer& GetInstances() { return instances; }
	const FCDPhysicsModelInstanceContainer& GetInstances() const { return instances; } /**< See above. */

	/** Retrieves the number of instantiated physics models that are part of 
		this physics model.
		@return The number of instantiated physics models. */
	size_t GetInstanceCount() const { return instances.size(); }

	/** Retrieves a specific instantiated physics model that is a part of this 
		physics model.
		@param index The index of the instance.
		@return The physics model instance. */
	FCDPhysicsModelInstance* GetInstance(size_t index) { FUAssert(index < instances.size(), return NULL); return instances.at(index); }
	const FCDPhysicsModelInstance* GetInstance(size_t index) const { FUAssert(index < instances.size(), return NULL); return instances.at(index); } /**< See above. */

	/** Adds a instantiated physics model to this physics model.
		@param model The physics model to be instanced in this physics model. 
			If this value is NULL, then there is no associated physics model
			for the instance yet. It should be filled in.
		@return The new physics model instance. */
	FCDPhysicsModelInstance* AddPhysicsModelInstance(FCDPhysicsModel* model = NULL);

	/** Retrieves the rigid bodies that are a part of this physics model.
		@return The rigid bodies. */
	FCDPhysicsRigidBodyContainer& GetRigidBodies() { return rigidBodies; }
	const FCDPhysicsRigidBodyContainer& GetRigidBodies() const { return rigidBodies; } /**< See above. */

	/** Retrieves the number of rigid bodies that are a part of this physics 
		model.
		@return The number of rigid bodies. */
	size_t GetRigidBodyCount() const { return rigidBodies.size(); }

	/** Retrieves a specific rigid body that is a part of this physics model
		by its index.
		@param index The index of the rigid body.
		@return The rigid body. */
	FCDPhysicsRigidBody* GetRigidBody(size_t index) { FUAssert(index < rigidBodies.size(), return NULL); return rigidBodies.at(index); }
	const FCDPhysicsRigidBody* GetRigidBody(size_t index) const { FUAssert(index < rigidBodies.size(), return NULL); return rigidBodies.at(index); } /**< See above. */

	/** Retrieves a specific rigid body that is a part of this physics model
		by its sid.
		@param sid The sid of the rigid body.
		@return The rigid body. */
	inline FCDPhysicsRigidBody* FindRigidBodyFromSid(const fm::string& sid) { return const_cast<FCDPhysicsRigidBody*>(const_cast<const FCDPhysicsModel*>(this)->FindRigidBodyFromSid(sid)); }
	const FCDPhysicsRigidBody* FindRigidBodyFromSid(const fm::string& sid) const; /**< See above. */

	/** Adds a rigid body to this physics model.
		@return The new rigid body. */
	FCDPhysicsRigidBody* AddRigidBody();

	/** Retrieves the rigid constraints that are a part of this physics model.
		@return The rigid constraints. */
	FCDPhysicsRigidConstraintContainer& GetRigidConstraints() { return rigidConstraints; }
	const FCDPhysicsRigidConstraintContainer& GetRigidConstraints() const { return rigidConstraints; } /**< See above. */

	/** Retrieves the number of rigid constraints that are a part of this
		physics model.
		@return The number of rigid constraints. */
	size_t GetRigidConstraintCount() const { return rigidConstraints.size(); }

	/** Retrieves a specific rigid constraint that is a part of tihs physics
		model by its index.
		@param index The index of the rigid constraint.
		@return The rigid constraint. */
	FCDPhysicsRigidConstraint* GetRigidConstraint(size_t index) { FUAssert(index < GetRigidConstraintCount(), return NULL); return rigidConstraints.at(index); }
	const FCDPhysicsRigidConstraint* GetRigidConstraint(size_t index) const { FUAssert(index < GetRigidConstraintCount(), return NULL); return rigidConstraints.at(index); } /**< See above. */

	/** Retrieves a specific rigid constraint that is a part of tihs physics
		model by its sid.
		@param sid The sid of the rigid constraint.
		@return The rigid constraint. */
	inline FCDPhysicsRigidConstraint* FindRigidConstraintFromSid(const fm::string& sid) { return const_cast<FCDPhysicsRigidConstraint*>(const_cast<const FCDPhysicsModel*>(this)->FindRigidConstraintFromSid(sid)); }
	const FCDPhysicsRigidConstraint* FindRigidConstraintFromSid(const fm::string& sid) const; /**< See above. */

	/** Adds a rigid constraint to this physics model.
		@return The new rigid constraint. */
	FCDPhysicsRigidConstraint* AddRigidConstraint();

	/** Copies the physics model into a clone.
		@param clone The empty clone. If this pointer is NULL, a new physics 
			model will be created and you will need to release the returned 
			pointer manually.
		@param cloneChildren Whether to recursively clone this entity's 
			children.
		@return The clone. */
	virtual FCDEntity* Clone(FCDEntity* clone = NULL, bool cloneChildren = false) const;

	/** [INTERNAL] Attaches the model instances.
		Because model instances can instance other models, the ordering is unknown so this cannot be done in the 
		LoadFromXML call. This method should be called after LoadFromXML is applied to all physics models. */
	bool AttachModelInstances();

	/** [INTERNAL] Cleans up the sub identifiers.
		The sub identifiers must be unique with respect to its parent. This method corrects the sub ids if there are conflicts. */
	virtual void CleanSubId();
};

#endif // _FCD_PHYSICSMODEL_H_
