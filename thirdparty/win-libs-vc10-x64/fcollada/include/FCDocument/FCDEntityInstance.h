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
	@file FCDEntityInstance.h
	This file contains the FCDEntityInstance class.
*/

#ifndef _FCD_ENTITY_INSTANCE_H_
#define _FCD_ENTITY_INSTANCE_H_

#ifndef _FCD_ENTITY_H_
#include "FCDocument/FCDEntity.h"
#endif // _FCD_ENTITY_H_

class FCDocument;
class FCDENode;
class FCDSceneNode;
class FCDEntityInstanceFactory;
class FCDEntityReference;
class FCDEntityInstance;
class FUUri;

template <class T> class FUUniqueStringMapT;
typedef FUUniqueStringMapT<char> FUSUniqueStringMap; /**< A set of unique strings. */

/**
	A COLLADA entity instance.
	COLLADA allows for quite a bit of per-instance settings
	for entities. This information is held by the up-classes of this class.
	This base class is simply meant to hold the entity that is instantiated.

	The entity instance tracks the entity, so that when an entity is released,
	all its instances are released.

	@ingroup FCDocument
*/
class FCOLLADA_EXPORT FCDEntityInstance : public FCDObject, FUTracker
{
public:
	/** The class type of the entity instance class.
		Used this information to up-cast an entity instance. */
	enum Type
	{
		SIMPLE, /**< A simple entity instance that has no per-instance information.
					This is used for lights, cameras, physics materials and force fields: there is no up-class. */
		GEOMETRY, /**< A geometry entity(FCDGeometryInstance). */
		CONTROLLER, /**< A controller entity(FCDControllerInstance). */
		MATERIAL, /**< A material entity(FCDMaterialInstance). */
		PHYSICS_MODEL, /**< A physics model(FCDPhysicsModelInstance). */
		PHYSICS_RIGID_BODY, /**< A physics rigid body(FCDPhysicsRigidBodyInstance). */
		PHYSICS_RIGID_CONSTRAINT, /**< A physics rigid constraint(FCDPhysicsRigidConstraintInstance). */
		PHYSICS_FORCE_FIELD, /**< A physics force field (FCDPhysicsForceFieldInstance). */
		TYPE_COUNT
	};

private:
	DeclareObjectType(FCDObject);
	friend class FCDEntityInstanceFactory;

	FCDSceneNode* parent; // May be NULL for non-scene graph instances.
	FCDEntity::Type entityType;
	DeclareParameterPtr(FCDEntityReference, entityReference, FC("Entity Reference"));

	// common attributes for instances
	fstring name;
	DeclareParameter(fm::string, FUParameterQualifiers::SIMPLE, wantedSubId, FC("Instance Sub-id"));
	
	// Extra information for the entity instance.
	DeclareParameterRef(FCDExtra, extra, FC("Extra Tree"));

protected:
	/** Constructor: do not use directly.
		Instead, use the appropriate allocation function.
		For scene node instance: FCDSceneNode::AddInstance.
		@param document The COLLADA document that owns the entity instance.
		@param parent The visual scene node that contains the entity instance. This pointer will be NULL for
			instances that are not directly under a visual scene node.
		@param type The type of entity to instantiate. */
	FCDEntityInstance(FCDocument* document, FCDSceneNode* parent, FCDEntity::Type type);

public:
	/** Destructor. */
	virtual ~FCDEntityInstance();

	/** Retrieves the entity instance class type.
		This is used to determine the up-class for the entity instance object.
		@deprecated Instead use: FCDEntityInstance::HasType().
		@return The class type: SIMPLE for entity instances with no up-class. */
	virtual Type GetType() const { return SIMPLE; }

	/** Retrieves the instantiated entity type.
		The instantiated entity type will never change.
		@return The instantiated entity type. */
	inline FCDEntity::Type GetEntityType() const { return entityType; }

	/** Retrieves the instantiated entity.  If the entity is an external reference,
		this may load the external document and retrieve the entity.
		@return The instantiated entity. */
	FCDEntity* GetEntity();

	/** Retrieves the instantiated entity.  If the entity is an external reference,
		this function will load the entity.  Be careful when using this function
		since it will change the object.
		@return The instantiated entity, if loaded. */
	inline const FCDEntity* GetEntity() const { return ((FCDEntityInstance*)(this))->GetEntity(); }

	/** Retrieves the Uri to the skin target.
		This can be an internal or external link
		@return The uri to the target */
	const FUUri GetEntityUri() const;

	/** Sets the URI of the target mesh.
		@param uri The Uri to a local or external controller or geometry */
	void SetEntityUri(const FUUri& uri);

	/** Sets the instantiated entity.
		The type of the entity will be verified.
		@param entity The instantiated entity. */
	void SetEntity(FCDEntity* entity);

	/** Get the contained EntityReference object. */
	inline FCDEntityReference* GetEntityReference() { return entityReference; }
	inline const FCDEntityReference* GetEntityReference() const { return entityReference; } /**< See above */

	/** Retrieves the name of the entity instance. This value has no direct use
		in COLLADA but is useful to track the user-friendly name of an entity
		instance.
		@return The name. */
	inline const fstring& GetName() const { return name; }

	/** Sets the name of the entity instance. This value has no direct use in 
		COLLADA but is useful to track the user-friendly name of an entity
		instance.
		@param name The name. */
	void SetName(const fstring& name);

	/** Retrieves the optional sub id and is not garanteed to exist. 
		This id is the same as that given in SetSubId or from the COLLADA document using LoadFromXML unless it clashes with another id and 
		CleanSubId has been called.
		@return The set sub id of the node. */
	inline const fm::string& GetWantedSubId() const { return wantedSubId; }

	/** Sets the sub id for this object. 
		This id must be unique within the scope of the parent element. If it is not, it can be corrected by calling CleanSubId.
		@param _wantedSubId The new sub id of the object. */
	inline void SetWantedSubId(const fm::string& _wantedSubId) { wantedSubId = _wantedSubId; }

	/** Retrieves the extra information tree for this entity instance. The 
		prefered way to save extra information in FCollada is at the entity 
		level. Use this extra information tree to store any information you 
		want exported and imported back.
		@return The extra information tree. */
	FCDExtra* GetExtra();
	inline const FCDExtra* GetExtra() const { return const_cast<FCDEntityInstance*>(this)->GetExtra(); } /**< See above. */

	/** Retrieves whether this entity instance points to an external entity.
		@return Whether this is an external entity instantiation. */
	bool IsExternalReference() const;

	/** Retrieves the parent of the entity instance.
		@return the parent visual scene node. This pointer will be NULL
			when the instance is not created in the visual scene graph. */
	inline FCDSceneNode* GetParent() { return parent; }
	inline const FCDSceneNode* GetParent() const { return parent; } /**< See above. */

	/** Checks whether or not this instance is below the given scene node in
		the scene hierarchy.
		@param node The scene node.
		@return True if parent is above this instance in the hierarchy, false otherwise.*/
	bool HasForParent(FCDSceneNode* node) const;

	/** [INTERNAL] Cleans up the sub identifiers.
		The sub identifiers must be unique with respect to its parent. This method corrects the sub ids if there are conflicts.
		@param parentStringMap The string map from the parent of this instance in which the sub ids must be unique. */
	virtual void CleanSubId(FUSUniqueStringMap* parentStringMap = NULL);

	/** Clones the entity instance.
		@param clone The entity instance to become the clone.
		@return The cloned entity instance. */
	virtual FCDEntityInstance* Clone(FCDEntityInstance* clone = NULL) const;

protected:
	/** [INTERNAL] Retrieves the COLLADA name for the instantiation of a given entity type.
		Children can override this method to easily add more class types.
		@param type The entity class type.
		@return The COLLADA name to instantiate an entity of the given class type. */
	//virtual const char* GetInstanceClassType(FCDEntity::Type type) const;

	/** Callback when the instantiated entity is being released.
		@param object A tracked object. */
	virtual void OnObjectReleased(FUTrackable* object);
};

/**
	[INTERNAL] A factory for COLLADA Entity instances.
	Creates the correct instance object for a given entity type/XML tree node.
	To create new instances, use the FCDSceneNode::AddInstance function.
*/
class FCOLLADA_EXPORT FCDEntityInstanceFactory
{
private:
	FCDEntityInstanceFactory() {} // Static class: do not instantiate.

public:
	/** Creates a new COLLADA instance, given a entity type.
		@param document The COLLADA document that will own the new instance.
		@param parent The visual scene node that will contain the instance.
		@param type The type of instance object to create.
		@return The new COLLADA instance. This pointer will be NULL
			if the given type is invalid. */
	static FCDEntityInstance* CreateInstance(FCDocument* document, FCDSceneNode* parent, FCDEntity::Type type);

	/** Creates a new COLLADA instance of a given entity.
		@param document The COLLADA document that will own the new instance.
		@param parent The visual scene node that will contain the instance.
		@param entity The entity to create an instance of.
		@return The new COLLADA instance. This pointer will be NULL
			if the given type is invalid. */
	static FCDEntityInstance* CreateInstance(FCDocument* document, FCDSceneNode* parent, FCDEntity *entity);
};


#endif // _FCD_ENTITY_INSTANCE_H_
