/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

/**
	@file FCDEntityReference.h
	This file contains the FCDEntityReference class.
*/

#ifndef _FCD_ENTITY_REFERENCE_H_
#define _FCD_ENTITY_REFERENCE_H_

#ifndef __FCD_OBJECT_H_
#include "FCDocument/FCDObject.h"
#endif // __FCD_OBJECT_H_
#ifndef _FU_URI_H_
#include "FUtils/FUUri.h"
#endif // _FU_URI_H_

class FCDPlaceHolder;
class FCDObjectWithId;

/**
	A COLLADA external reference for an entity instance.
	FCollada only exposes external references for entity instances.
	Other types of external references: geometry sources, morph targets, etc.
	are not supported.

	The entity instance for an external reference cannot be modified
	and tracks it, so that if is it released manually, the reference is also released.

	The placeholder and the document referenced by the entity instance can
	be modified manually or by the entity instance.

	Also allowing external links by more than just entity instances.  Controllers can
	reference external geometry targets. 

	Renaming this class to FCDEntityReference, as it used for tracking ALL entities.
	This lightens the Code load on all classes that track entities.

	TODO: The following are non-compliant on the XRefs:
		Animations (instance_animation) <-- TODO QUICK
		Morphers (base_geom && morph_targets)
		Emitters (force links)
		Emitters (object_mesh) <-- TODO LAST
		
	@ingroup FCDocument
*/
class FCOLLADA_EXPORT FCDEntityReference : public FCDObject, FUTracker
{
private:
	DeclareObjectType(FCDObject);

	FCDEntity* entity;
	FCDPlaceHolder* placeHolder;
	fm::string entityId;

	// Pointer back to the nearest object with id that references
	// us down the hierarchy.  This is because the references from the
	// actual object with the link are irrelevant in terms of the hierarchy,
	// it is only objects with id's (ie FCDEntities) that actually matter in terms
	// of object durability etc.
	FCDObjectWithId* baseObject;

public:
	/** Constructor.
		@param document The FCollada document that owns the reference.
		@param baseObject The parent object. */
	FCDEntityReference(FCDocument* document, FCDObjectWithId* parent);

	/** Destructor. */
	virtual ~FCDEntityReference();

	/** Retrieves the placeholder if this references an entity
		from an external COLLADA document.
		@return The COLLADA document placeholder. */
	FCDPlaceHolder* GetPlaceHolder() { return placeHolder; }
	const FCDPlaceHolder* GetPlaceHolder() const { return placeHolder; } /**< See above. */

	/** Retrieves the COLLADA id of the entity that is externally referenced.
		@return The COLLADA id of the referenced entity. */
	const fm::string& GetEntityId() const { return entityId; }

	/** Retrieves whether this entity reference is an external entity reference.
		This function intentionally hides the FCDObject::IsExternal function.
		@return Whether the entity reference is an external entity reference. */
	inline bool IsExternal() const { return placeHolder != NULL; }

	/** Retrieves whether this entity reference is a local entity reference.
		This function intentionally hides the FCDObject::IsLocal function.
		@return Whether the entity reference is a local entity reference. */
	inline bool IsLocal() const { return placeHolder == NULL; }

	/** Sets the COLLADA id of the referenced entity.
		@param id The COLLADA id of the referenced entity. */
	void SetEntityId(const fm::string& id) { entityId = id; SetDirtyFlag(); }

	/** Retrieves the full URI of the external reference.
		This points to the COLLADA document and the id of the referenced entity.
		@return The referenced entity URI. */
	FUUri GetUri() const;

	/** Sets the URI of the external reference.
		This points to the COLLADA document and the id of the referenced entity.
		@param uri The referenced entity URL. */
	void SetUri(const FUUri& uri);
	
	/** Get the entity this external reference points to.  This may cause the entity to
		load if it is not present already!
		See the FCollada::GetDereferenceFlag() for more information.
		@return the entity */
	FCDEntity* GetEntity() { return const_cast<FCDEntity*>(const_cast<const FCDEntityReference*>(this)->GetEntity()); }
	const FCDEntity* GetEntity() const; /**< See above. */

	/** Get a pointer to the entity that either directly or indirectly 
		exclusively contains this reference.  This is not necessarily the class
		that contains the actual pointer, but it is the class that is responsible for
		managing the link to the entity referenced (ie, not an FCDEntityInstance, but the
		containing FCDEntity).
		@return A pointer the object that uses this class to reference an entity */
	inline const FCDObjectWithId* GetClosestObjectWithId() const { return baseObject; }

	/** Set the pointer to the closest entity upstream that contains this reference.
		@param obj An object with Id that either directly or indirectly exclusively
				   contains this reference */
	inline void SetClosestObjectWithId(FCDObjectWithId* obj) { FUAssert(baseObject == NULL,); baseObject = obj; }

	/** Set the entity we are referencing.  If this is from an external document, it will
		create the appropriate FCDPlaceholder etc.
		@entity The new entity to reference */
	void SetEntity(FCDEntity* entity);

private:
	// Attempt to find the entity based on the set URL etc. 
	// This can cause an external document to load if the reference is external.
	void LoadEntity();

	//	Sets the COLLADA document referenced by the entity instance.
	// 	@param document The COLLADA document referenced by the entity instance.
	void SetEntityDocument(FCDocument* document);

	// Sets the COLLADA document place holder that replaces the
	// external COLLADA document for this reference.
	// @param placeHolder The COLLADA document place holder.
	void SetPlaceHolder(FCDPlaceHolder* placeHolder);

	virtual void OnObjectReleased(FUTrackable* object);
};

#endif // _FCD_ENTITY_REFERENCE_H_
