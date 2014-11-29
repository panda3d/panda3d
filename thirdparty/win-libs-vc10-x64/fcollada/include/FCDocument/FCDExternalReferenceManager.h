/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

/**
	@file FCDExternalReferenceManager.h
	This file contains the FCDExternalReferenceManager class.
*/

#ifndef _FCD_EXTERNAL_REFERENCE_MANAGER_H_
#define _FCD_EXTERNAL_REFERENCE_MANAGER_H_

#ifndef __FCD_OBJECT_H_
#include "FCDocument/FCDObject.h"
#endif // __FCD_OBJECT_H_

class FCDEntityInstance;
class FCDEntityReference;
class FCDPlaceHolder;

/**
	A FCollada document external reference manager.

	Each FCollada document has one and only one external reference manager.
	It keeps track of all the external document's, whether they are loaded or not.

	By default, all external references are handled automatically.
	You will have to access this structure only for informational purposes or
	if you have disabled the automatic de-referencing feature using the
	FCollada::SetDereferenceFlag function.

	@ingroup FCDocument
*/
class FCOLLADA_EXPORT FCDExternalReferenceManager : public FCDObject
{
private:
	DeclareObjectType(FCDObject);

	FUObjectContainer<FCDPlaceHolder> placeHolders;

public:
	/** Constructor.
		@param document The COLLADA document that owns the external reference manager. */
	FCDExternalReferenceManager(FCDocument* document);

	/** Destructor. */
	virtual ~FCDExternalReferenceManager();

	/** Adds a new FCollada document placeholder to this document.
		@param document The external FCollada document to track. */
	FCDPlaceHolder* AddPlaceHolder(FCDocument* document);

	/** Adds a new FCollada document placeholder to this document.
		@param fileUrl The URI of the external FCollada document to track. */
	FCDPlaceHolder* AddPlaceHolder(const fstring& fileUrl);

	/** Retrieves the number of FCollada document that are automatically
		tracked by this document.
		@return The number of tracked document. */
	inline size_t GetPlaceHolderCount() const { return placeHolders.size(); }

	/** Retrieves a FCollada document placeholder.
		@param index The index of the placeholder.
		@return The placeholder at the given index. */
	inline FCDPlaceHolder* GetPlaceHolder(size_t index) { FUAssert(index < placeHolders.size(), return NULL); return placeHolders.at(index); }
	inline const FCDPlaceHolder* GetPlaceHolder(size_t index) const { FUAssert(index < placeHolders.size(), return NULL); return placeHolders.at(index); } /**< See above. */

	/** Retrieves the placeholder that references the FCollada document at the given URI.
		@param fileUrl The URI of the FCollada document.
		@return The placeholder for the FCollada document. This pointer will be NULL
			if no local entity instances reference entities within the document
			at the given URI. */
	const FCDPlaceHolder* FindPlaceHolder(const fstring& fileUrl) const;
	inline FCDPlaceHolder* FindPlaceHolder(const fstring& fileUrl) { return const_cast<FCDPlaceHolder*>(const_cast<const FCDExternalReferenceManager*>(this)->FindPlaceHolder(fileUrl)); } /**< See above. */

	/** Retrieves the placeholder that references the given FCollada document.
		@param document A FCollada document.
		@return The placeholder for the given FCollada document. This pointer will be NULL
			if no local entity instances reference entities within the given document. */
	const FCDPlaceHolder* FindPlaceHolder(const FCDocument* document) const;
	inline FCDPlaceHolder* FindPlaceHolder(FCDocument* document)  { return const_cast<FCDPlaceHolder*>(const_cast<const FCDExternalReferenceManager*>(this)->FindPlaceHolder(document)); } /**< See above. */

	/** [INTERNAL] Registers a newly-loaded FCollada document
		with the other existing FCollada document. This callback is used
		to update all the entity instances that reference external entities.
		Any entity instance that references an entity within the newly-loaded
		FCollada document will be updated.
		@param document The newly-loaded FCollada document. */
	static void RegisterLoadedDocument(FCDocument* document);
};

#endif // _FCD_EXTERNAL_REFERENCE_MANAGER_H_
