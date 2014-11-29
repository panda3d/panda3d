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

#ifndef _FCD_EXTRA_H_
#include "FCDocument/FCDExtra.h"
#endif // _FCD_EXTRA_H_
#ifndef _FCD_ASSET_H_
#include "FCDocument/FCDAsset.h"
#endif // _FCD_ASSET_H_
#ifndef _FCD_ENTITY_H_
#include "FCDocument/FCDEntity.h"
#endif // _FCD_ENITTY_H_

template <class T>
FCDLibrary<T>::FCDLibrary(FCDocument* document)
:	FCDObject(document)
,	InitializeParameterNoArg(entities)
,	InitializeParameterNoArg(extra)
,	InitializeParameterNoArg(asset)
{
	extra = new FCDExtra(document, this);
}

template <class T>
FCDLibrary<T>::~FCDLibrary()
{
	SAFE_RELEASE(extra);
	SAFE_RELEASE(asset);
}

// Create the asset if it isn't already there.
template <class T>
FCDAsset* FCDLibrary<T>::GetAsset(bool create)
{
	if (create && asset == NULL) asset = new FCDAsset(GetDocument());
	return asset;
}

// Search for the entity in this library with a given COLLADA id.
template <class T>
const T* FCDLibrary<T>::FindDaeId(const fm::string& daeId) const
{
#ifdef _DEBUG
	// [staylor] June 12 2007 - !!Code change verification check!!
	// When fixing up the FCPlugin archive merge, removed SkipPound
	// here (Should be obsolete).  If you see this code much past this 
	// date, feel free to remove it.
	FUAssert (daeId.empty() || daeId[0] != '#',);
#endif

	size_t entityCount = entities.size();
	for (size_t i = 0; i < entityCount; ++i)
	{
		const FCDEntity* found = entities[i]->FindDaeId(daeId);
		if (found != NULL && found->GetObjectType() == T::GetClassType())
		{
			return (T*) found;
		}
	}
	return NULL;
}

// Search for the entity in this library with a given COLLADA id.
template <class T>
T* FCDLibrary<T>::AddEntity()
{
	T* entity = new T(GetDocument());
	entities.push_back(entity);
	SetNewChildFlag();
	return entity;
}


template <class T>
void FCDLibrary<T>::AddEntity(T* entity) 
{ 
	entities.push_back(entity); SetNewChildFlag(); 
}
