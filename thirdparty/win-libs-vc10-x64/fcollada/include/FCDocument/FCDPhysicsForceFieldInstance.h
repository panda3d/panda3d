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
	@file FCDPhysicsForceFieldInstance.h
	This file contains the FCDPhysicsForceFieldInstance class.
*/
#ifndef _FCD_PHYSICS_FORCE_FIELD_INSTANCE_H_
#define _FCD_PHYSICS_FORCE_FIELD_INSTANCE_H_

#ifndef _FCD_ENTITY_INSTANCE_H_
#include "FCDocument/FCDEntityInstance.h"
#endif // _FCD_ENTITY_INSTANCE_H_

class FCDocument;

/**
	A COLLADA force field instance.
*/
class FCOLLADA_EXPORT FCDPhysicsForceFieldInstance : public FCDEntityInstance
{
private:
	DeclareObjectType(FCDEntityInstance);

	friend class FCDEntityInstanceFactory;

protected:
	/** Constructor: do not use directly. Create new physics force fields using
		the FCDEntityInstanceFactory::CreateInstance function.
		@param document The COLLADA document that contains this physics model
			instance. 
		@param parent The parent node for this force field. At the time of
			writing, this should be NULL since only parents are physics model
			instances and physics scene.
		@param entityType The entity type: FORCE_FIELD. */
	FCDPhysicsForceFieldInstance(FCDocument* document, FCDSceneNode* parent, FCDEntity::Type entityType = FCDEntity::FORCE_FIELD);

public:
	/** Destructor. */
	virtual ~FCDPhysicsForceFieldInstance();

	/** Retrieves the entity instance class type.
		This is used to determine the up-class for the entity instance object.
		@deprecated Instead use: FCDEntityInstance::HasType(
			FCDGeometryInstance::GetClassType())
		@return The class type: PHYSICS_FORCE_FIELD. */
	virtual Type GetType() const { return PHYSICS_FORCE_FIELD; }

	/** Clones the physics force field instance.
		@param clone The physics force field instance to become the clone.
			If this pointer is NULL, a new physics force field instance will be
			created and you will need to release it.
		@return The clone. */
	virtual FCDEntityInstance* Clone(FCDEntityInstance* clone = NULL) const;
};

#endif // _FCD_PHYSICS_FORCE_FIELD_INSTANCE_H_
