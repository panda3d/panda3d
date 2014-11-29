/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

/**
	@file FCDPhysicsRigidBody.h
	This file contains the FCDPhysicsRigidBody class.
*/

#ifndef _FCD_PHYSICSRIGIDBODY_H_
#define _FCD_PHYSICSRIGIDBODY_H_

#ifndef _FCD_ENTITY_H_
#include "FCDocument/FCDEntity.h"
#endif // _FCD_ENTITY_H_

class FCDocument;
class FCDPhysicsRigidBodyParameters;

/**
	A COLLADA rigid body.

	A rigid body contains all the information needed for collision detection.

	@ingroup FCDocument
*/
class FCOLLADA_EXPORT FCDPhysicsRigidBody : public FCDEntity
{
private:
	DeclareObjectType(FCDEntity);

	DeclareParameterRef(FCDPhysicsRigidBodyParameters, parameters, FC("Parameters"));

public:
	/** Constructor: do not use directly. Create new rigid bodies using the 
		FCDPhysicsModel::AddRigidBody function.
		@param document The COLLADA document that contains this rigid body. */
	FCDPhysicsRigidBody(FCDocument* document);

	/** Destructor. */
	virtual ~FCDPhysicsRigidBody();

	/** Retrieves the entity type for this class. This function is part of the 
		FCDEntity interface.
		@return The entity type: PHYSICS_RIGID_BODY. */
	virtual Type GetType() const { return FCDEntity::PHYSICS_RIGID_BODY; }

	/** Retrieves the sub id (sid) of this rigid body.
		@return The sub id. */
	inline const fm::string& GetSubId() const { return Parent::GetDaeId(); }

	/** Sets the sub id (sid) of this rigid body.
		@param sid The new sub id. */
	inline void SetSubId(const char* sid) { Parent::SetDaeId(sid); }
	inline void SetSubId(const fm::string& sid) { Parent::SetDaeId(sid); } /**< See above. */

	/** Retrieves the parameters of tihs rigid body.
		@return The parameters. */
	FCDPhysicsRigidBodyParameters* GetParameters() { return parameters; }
	const FCDPhysicsRigidBodyParameters* GetParameters() const { return parameters; } /**< See above. */

	/** Retrieves the factor to multiple by to ensure shape masses are correct.
		According to the COLLADA spec in 1.4.1, if the total mass of the rigid
		body is 6 and there are 2 shapes with mass of 1 and 2, then it will be
		interpreted as 2 shapes with mass of 2 and 4 respectively. This method
		gives the factor to convert from the shape's mass to the one that 
		should be interpreted as.
		@return The factor for shape mass. */
	float GetShapeMassFactor() const;

	/** Copies the rigid body into a clone.
		@param clone The empty clone. If this pointer is NULL, a new rigid
			body will be created and you will need to release the returned 
			pointer manually.
		@param cloneChildren Whether to recursively clone this entity's 
			children.
		@return The clone. */
	virtual FCDEntity* Clone(FCDEntity* clone = NULL, bool cloneChildren = false) const;
};

#endif // _FCD_PHYSICSRIGIDBODY_H_
