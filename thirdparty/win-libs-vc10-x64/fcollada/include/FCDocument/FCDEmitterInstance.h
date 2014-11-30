/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

/**
	@file FCDEmitterInstance.h
	This file contains the FCDEmitterInstance class.

	@ingroup FCDEmitters
*/

#ifndef _FCD_EMITTER_INSTANCE_H_
#define _FCD_EMITTER_INSTANCE_H_

#ifndef _FCD_ENTITY_INSTANCE_H_
#include "FCDocument/FCDEntityInstance.h"
#endif // _FCD_ENTITY_INSTANCE_H_

class FCDGeometryPolygons;
class FCDMaterial;
class FCDMaterialInstance;
class FCDForceField;

/**
	A COLLADA emitter instance.

	The types of particles emitted are instance dependent.
	Instances also contain the FCDForceInstances pointers to forces applied to the particles.
*/
class FCOLLADA_EXPORT FCDEmitterInstance : public FCDEntityInstance
{
private:
    DeclareObjectType(FCDEntityInstance);
	friend class FCDEntityInstanceFactory;

protected:
	/** Constructor: do not use directly.
		Instead, use the FCDSceneNode::AddInstance function.
		@param document The COLLADA document that owns the emitter instance.
		@param parent The parent visual scene node.
		@param entityType The type of the entity to instantiate. Unless this class
			is overwritten, FCDEntity::EMITTER should be given. */
	FCDEmitterInstance(FCDocument* document, FCDSceneNode* parent, FCDEntity::Type entityType = FCDEntity::EMITTER);

public:
	/** Destructor. */
	virtual ~FCDEmitterInstance();

};

#endif // _FCD_EMITTER_INSTANCE_H_
