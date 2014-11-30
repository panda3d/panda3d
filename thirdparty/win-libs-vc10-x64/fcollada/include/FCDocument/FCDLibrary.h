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
	@file FCDLibrary.h
	This file contains the FCDLibrary template class.
	See the FCDLibrary.hpp file for the template implementation.
*/

#ifndef _FCD_LIBRARY_
#define _FCD_LIBRARY_

#ifndef __FCD_OBJECT_H_
#include "FCDocument/FCDObject.h"
#endif // __FCD_OBJECT_H_
#ifndef _FU_PARAMETER_H_
#include "FUtils/FUParameter.h"
#endif // _FU_PARAMETER_H_

class FCDocument;
class FCDAsset;
class FCDEntity;
class FCDExtra;

/**
	A COLLADA library.

	A COLLADA library holds a list of entities. There are libraries for the following entities:
	animations (FCDAnimation), animation clips (FCDAnimationClip), meshes and splines (FCDGeometry),
	materials (FCDMaterial), effects (FCDEffect), images (FCDImage), skins and morphers (FCDController),
	cameras (FCDCamera), lights (FCDLight), physics models (FCDPhysicsModel), physics materials
	(FCDPhysicsMaterial), physics scenes (FCDPhysicsScene) and visual scenes (FCDSceneNode).

	The COLLADA libraries are contained within the FCDocument object.

	@ingroup FCDocument
*/	
template <class T>
class FCOLLADA_EXPORT FCDLibrary : public FCDObject
{
private:
	DeclareObjectType(FCDObject);

	/** Entities list. This list should contain all the root entities of the correct type.
		Note that the following entity types are tree-based, rather than list-based: FCDAnimation,
		FCDSceneNode and FCDPhysicsScene. */
	DeclareParameterContainer(T, entities, FC("Entities"));

	// Extra information for the entity.
	DeclareParameterRef(FCDExtra, extra, FC("Extra Tree"));

	// Asset information for the entity.
	DeclareParameterRef(FCDAsset, asset, FC("Asset Tag"));

public:
	/** Constructor: do not use directly.
		All the necessary libraries are created by the FCDocument object during its creation.
		@param document The parent document. */
	FCDLibrary(FCDocument* document);

	/** Destructor. */
	virtual ~FCDLibrary();

	/** Creates a new entity within this library.
		@return The newly created entity. */
	T* AddEntity();

	/** Inserts a new entity to this library.
		This function is useful if you are adding cloned entites
		back inside the library.
		@param entity The entity to insert in the library.
		@return The entity to insert. */
	void AddEntity(T* entity);

	/** Retrieves the library entity with the given COLLADA id.
		@param daeId The COLLADA id of the entity.
		@return The library entity which matches the COLLADA id.
			This pointer will be NULL if no matching entity was found. */
	T* FindDaeId(const fm::string& daeId) { return const_cast<T*>(const_cast<const FCDLibrary*>(this)->FindDaeId(daeId)); }
	const T* FindDaeId(const fm::string& daeId) const; /**< See above. */

	/** Returns whether the library contains no entities.
		@return Whether the library is empty. */
	inline bool IsEmpty() const { return entities.empty(); }

	/** Retrieves the number of entities within the library.
		@return the number of entities contained within the library. */
	inline size_t GetEntityCount() const { return entities.size(); }

	/** Retrieves an indexed entity from the library.
		@param index The index of the entity to retrieve.
			Should be within the range [0, GetEntityCount()[.
		@return The indexed entity. */
	inline T* GetEntity(size_t index) { FUAssert(index < GetEntityCount(), return NULL); return entities.at(index); }
	inline const T* GetEntity(size_t index) const { FUAssert(index < GetEntityCount(), return NULL); return entities.at(index); } /**< See above. */

	/** Retrieves the asset information for the library.
		The non-const version of this function can create
		an empty asset structure for this library.
		@param create Whether to create the asset tag if it is missing. Defaults to true.
		@return The asset tag for the library. */
	FCDAsset* GetAsset(bool create = true);
	inline const FCDAsset* GetAsset() const { return asset; } /** See above. */

	/** Retrieves the asset information for the library.
		@return The asset tag for the library. */
	inline FCDExtra* GetExtra() { return extra; }
	inline const FCDExtra* GetExtra() const { return extra; } /** See above. */
};

#ifdef __APPLE__
#include "FCDocument/FCDLibrary.hpp"
#endif // __APPLE__

#endif // _FCD_LIBRARY_
