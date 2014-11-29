/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

/**
	@file FCDPhysicsMaterial.h
	This file contains the FCDPhysicsMaterial class.
*/

#ifndef _FCD_PHYSICSMATERIAL_H_
#define _FCD_PHYSICSMATERIAL_H_

#ifndef _FCD_ENTITY_H_
#include "FCDocument/FCDEntity.h"
#endif // _FCD_ENTITY_H_

class FCDocument;

/**
	A COLLADA physics material.

	A physics material contains physical properties for an object. Because
	there isn't anything interesting in the physics material instance, this 
	class is used to represent that as well.

	@ingroup FCDocument
*/
class FCOLLADA_EXPORT FCDPhysicsMaterial : public FCDEntity
{
private:
	DeclareObjectType(FCDEntity);
	float staticFriction;
	float dynamicFriction;
	float restitution;

public:
	/** Constructor.
		@param document The COLLADA document that contains this rigid body. */
	FCDPhysicsMaterial(FCDocument* document);

	/** Destructor. */
	virtual ~FCDPhysicsMaterial();

	/** Retrieves the entity type for this class. This function is part of the 
		FCDEntity interface.
		@return The entity type: PHYSICS_MATERIAL. */
	virtual Type GetType() const { return FCDEntity::PHYSICS_MATERIAL; }

	/** Retrieves the static friction for this physics material.
		@return The static friction. */
	float GetStaticFriction() const { return staticFriction; }

	/** Sets the static friction for this physics material.
		@param _staticFriction The static friction. */
	void  SetStaticFriction(float _staticFriction) { staticFriction = _staticFriction; SetDirtyFlag(); }

	/** Retrieves the dynamic friction for this physics material.
		@return The dynamic friction. */
	float GetDynamicFriction() const { return dynamicFriction; }

	/** Sets the dynamic friction for this physics material.
		@param _dynamicFriction The dynamic friction. */
	void  SetDynamicFriction(float _dynamicFriction) { dynamicFriction = _dynamicFriction; SetDirtyFlag(); }

	/** Retrieves the restitution for this physics material.
		@return The restitution. */
	float GetRestitution() const { return restitution; }

	/** Sets the restitution for this physics material.
		@param _restitution The restitution. */
	void  SetRestitution(float _restitution) { restitution = _restitution; SetDirtyFlag(); }

	/** Copies the physics material into a clone.
		@param clone The empty clone. If this pointer is NULL, a new physics
			material will be created and you will need to release the returned 
			pointer manually.
		@param cloneChildren Whether to recursively clone this entity's 
			children.
		@return The clone. */
	virtual FCDEntity* Clone(FCDEntity* clone = NULL, bool cloneChildren = false) const;
};

#endif // _FCD_MATERIAL_H_
