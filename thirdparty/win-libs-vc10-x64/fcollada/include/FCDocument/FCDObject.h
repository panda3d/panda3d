/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

/**
	@file FCDObject.h
	This file contains the FCDObject class.
*/

#ifndef __FCD_OBJECT_H_
#define __FCD_OBJECT_H_

#ifndef _FU_PARAMETERIZABLE_H_
#include "FUtils/FUParameterizable.h"
#endif // _FU_PARAMETERIZABLE_H_

class FCDocument;

/**
	A basic COLLADA document object.
	
	All the objects owned by the COLLADA document derive from this class.
	The FCDocument object is accessible through this interface to all the object which it owns.

	Space for an handle which has no meaning to FCollada is available in this base class, for our users.
	You can therefore attach your own objects to most FCollada objects. If you assign memory buffers
	to the user-specified handle, be aware that FCollada will make no attempt to release it.

	32 flags are also available within this object. You can use the DeclareFlag
	and DeclareFlagCount macros to include flags within your custom classes. This object reverses
	the first four bits for its own flags: dirty, value changed, transient and new child.
	
	All FCollada objects should set the dirty flag when modifications are made
	to the objects, but FCollada will never reset it. This flag should be used by
	multi-tier applications. This flag defaults to 'true'.

	@ingroup FCDocument
*/
class FCOLLADA_EXPORT FCDObject : public FUParameterizable
{
private:
	DeclareObjectType(FUParameterizable);

	// The COLLADA document that owns this object
	FCDocument* document;

	// An handle which has no meaning to FCollada but is available for users to
	// attach objects to most FCollada objects.
	void* userHandle;

public:
	/** Declare the flags to set various states available on an FCDObject 
		Each Declare flag requires its local index.  It is required to 
		DeclareFlagCount, declaring the amount of flags specified locally */
	DeclareFlag(Transient, 0); /**< [EXPERIMENTAL] This object exists for the application to use.
							        This object should be not archived/saved. */
	DeclareFlag(NewChild, 1); /**< [EXPERIMENTAL] A new child has been assigned to this object.
							       Should be replaced by the StructureChanged flag in future versions. */
	DeclareFlagCount(2); /**< 5 flags are locally declared. */

public:
	/** Constructor: sets the COLLADA document object.
		@param document The COLLADA document which owns this object. */
	FCDObject(FCDocument* document);

	/** Destructor. */
	virtual ~FCDObject();

	/** Retrieves the COLLADA document which owns this object.
		@return The COLLADA document. */
	inline FCDocument* GetDocument() { return document; }
	inline const FCDocument* GetDocument() const { return document; } /**< See above. */

	/** Retrieves whether a given object is a local reference from this object.
		@param object A data object.
		@return Whether a reference from this object to the given object is local. */
	inline bool IsLocal(const FCDObject* object) const { return document == object->document; }

	/** Retrieves whether a given object is an external reference from this object.
		@param object A data object.
		@return Whether a reference from this object to the given object is external. */
	inline bool IsExternal(const FCDObject* object) const { return document != object->document; }

	/** Retrieves the object's user-specified handle.
		This handle is available for users and has no
		meaning to FCollada.
		@return The object user-specified handle. */
	inline void* GetUserHandle() const { return userHandle; }
	
	/** Sets the object's user-specified handle.
		This handle is available for users and has no
		meaning to FCollada.
		@param handle The user-specified handle. */
	inline void SetUserHandle(void* handle) { userHandle = handle; SetDirtyFlag(); }

	/** ValueChangedFlag override, this allows objects to react if necessary. */
	virtual void SetValueChange() { SetValueChangedFlag(); }
};

#endif // __FCD_OBJECT_H_
