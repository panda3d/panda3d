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
	@file FCDTargetedEntity.h
	This file contains the FCDTargetedEntity class.
*/

#ifndef _FCD_TARGETED_ENTITY_H_
#define _FCD_TARGETED_ENTITY_H_

#ifndef _FCD_ENTITY_H_
#include "FCDocument/FCDEntity.h"
#endif // _FCD_ENTITY_H_

class FCDSceneNode;

/**
	A targeted entity.

	COLLADA doesn't have targeted entity.
	Therefore: the behavior of a targeted entity is application-defined.
	
	This class is used to support 3dsMax's targeted cameras and lights
	and we also plan to support Maya's targeted cameras and lights.

	@see FCDCamera FCDLight
	@ingroup FCDEntity
*/
class FCOLLADA_EXPORT FCDTargetedEntity : public FCDEntity
{
private:
	DeclareObjectType(FCDEntity);

	// Target
	DeclareParameterPtr(FCDSceneNode, targetNode, FC("Target Node"));

public:
	/** Constructor: do not use directly.
		Instead, create objects of the up-classes.
		@param document The COLLADA document that owns the targeted entity.
		@param baseId The prefix COLLADA id to be used if no COLLADA id is provided. */
	FCDTargetedEntity(FCDocument* document, const char* baseId);

	/** Destructor. */
	virtual ~FCDTargetedEntity();

	/** Retrieves whether a target is defined for this entity.
		@return Whether a target is defined for this entity. */
	inline bool HasTarget() const { return targetNode != NULL; }

	/** Retrieves the target visual scene node for this entity.
		@return The target visual scene node. */
	inline FCDSceneNode* GetTargetNode() { return targetNode; }
	inline const FCDSceneNode* GetTargetNode() const { return targetNode; } /**< See above. */

	/** Sets the target visual scene node for this entity.
		@param target The new target node. */
	void SetTargetNode(FCDSceneNode* target);

	/** Copies the entity target information into a entity clone.
		All the overwriting functions of this function should call this function
		to copy the target information.
		The cloned entity may reside in another document.
		@param clone The empty clone. If this pointer is NULL, a new entity
			will be created and you will need to release the returned pointer manually.
		@param cloneChildren Whether to recursively clone this entity's children.
		@return The clone. */
	virtual FCDEntity* Clone(FCDEntity* clone = NULL, bool cloneChildren = false) const;

};

#endif // _FCD_TARGETED_ENTITY_H_

