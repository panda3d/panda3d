/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

/**
	@file FCDPlaceHolder.h
	This file contains the FCDPlaceHolder class.
*/

#ifndef _FCD_PLACEHOLDER_H_
#define _FCD_PLACEHOLDER_H_

#ifndef __FCD_OBJECT_H_
#include "FCDocument/FCDObject.h"
#endif // __FCD_OBJECT_H_

class FCDEntityReference;

/**
	A FCollada document placeholder.
	
	This class is used to correctly support deferred external references.
	It contains the information necessary to find and open referenced 
	FCollada documents and (re-)bind entity instances that use entities within
	the referenced FCollada document.

	@ingroup FCDocument
*/
class FCOLLADA_EXPORT FCDPlaceHolder : public FCDObject, FUTracker
{
private:
	DeclareObjectType(FCDPlaceHolder);

	FCDocument* target;
	FUTrackedList<FCDEntityReference> references;
	fstring fileUrl;

public:
	/** Constructor.
		@param document The FCollada document that owns the placeholder.
		@param target The FCollada document referenced by the placeholder. */
	FCDPlaceHolder(FCDocument* document, FCDocument* target = NULL);

	/** Destructor. */
	virtual ~FCDPlaceHolder();

	/** Retrieves the referenced FCollada document.
		@return The referenced FCollada document. The NULL pointer will
			be returned when the referenced FCollada document is not loaded. */
	inline FCDocument* GetTarget() const { return target; }

	/** Retrieves the referenced FCollada document.
		@param loadIfMissing Whether the referenced document should be loaded when
			it is not already loaded.
		@return The referenced FCollada document. The NULL pointer will
			be returned when the referenced FCollada document could not be opened. */
	FCDocument* GetTarget(bool loadIfMissing = true);

	/** [INTERNAL] Loads the referenced FCollada document.
		@param _target The FCollada document referenced by this placeholder.
			This pointer will be NULL to let the placeholder load the FCollada document it
			knows about. */
	void LoadTarget(FCDocument* _target = NULL);

	/** Unloads and releases the referenced FCollada document. */
	void UnloadTarget();

	/** Retrieves whether the FCollada document referenced by this placeholder
		is currently loaded and available.
		@return Whether the referenced document is available. */
	inline bool IsTargetLoaded() const { return target != NULL; }

	/** Retrieves the URL of the referenced FCollada document.
		@return The URL of the referenced FCollada document. */
	const fstring& GetFileUrl() const;

	/** Sets the URL of the referenced FCollada document.
		@param url The URL of the referenced FCollada document. */
	void SetFileUrl(const fstring& url);

	/** [INTERNAL] Registers an entity reference into out document.
		This external reference will be updated as the 
		referenced FCollada document is loaded and unloaded.
		@param reference An external reference. */
	inline void AddExternalReference(FCDEntityReference* reference) { references.push_back(reference); SetNewChildFlag(); }

	/** [INTERNAL] Removes an external reference from
		this placeholder's management list.
		@param reference An external reference currently managed by this placeholder. */
	inline void RemoveExternalReference(FCDEntityReference* reference) { references.erase(reference); SetNewChildFlag(); }

	/** Retrieves the number of external references that references entities
		contained within the referenced FCollada document.
		@return The number of external references. */
	inline size_t GetExternalReferenceCount() const { return references.size(); }

	/** Retrieves an indexed external reference.
		@param index The index of the external reference.
		@return The external reference at the given index. */
	const FCDEntityReference* GetExternalReference(size_t index) const { FUAssert(index < GetExternalReferenceCount(), return NULL); return references.at(index); }

protected:
	/** [INTERNAL] Callback for the manual release of a
		loaded referenced FCollada document.
		@param object The released object pointer. */
	void OnObjectReleased(FUTrackable* object);
};

#endif // _FCD_PLACEHOLDER_H_

