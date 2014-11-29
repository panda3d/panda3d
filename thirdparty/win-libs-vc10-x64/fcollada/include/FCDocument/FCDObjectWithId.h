/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

/**
	@file FCDObjectWithId.h
	This file contains the FCDObjectWithId class.
*/

#ifndef __FCD_OBJECT_WITH_ID_H_
#define __FCD_OBJECT_WITH_ID_H_

#ifndef __FCD_OBJECT_H_
#include "FCDocument/FCDObject.h"
#endif // __FCD_OBJECT_H_
#ifndef _FU_PARAMETER_H_
#include "FUtils/FUParameter.h"
#endif // _FU_PARAMETER_H_

/**
	A basic COLLADA object which has a unique COLLADA id.
	
	Many COLLADA structures such as entities and sources need a unique COLLADA id.
	The COLLADA document contains a map of all the COLLADA ids known in its scope.
	The interface of the FCDObjectWithId class allows for the retrieval and the modification
	of the unique COLLADA id attached to these objects.

	A unique COLLADA id is built, if none are provided, using the 'baseId' field of the constructor.
	A unique COLLADA id is generated only on demand.

	@ingroup FCDocument
*/
class FCOLLADA_EXPORT FCDObjectWithId : public FCDObject
{
private:
	DeclareObjectType(FCDObject);

	DeclareParameter(fm::string, FUParameterQualifiers::SIMPLE, daeId, FC("Unique Id"));

private:
	DeclareFlag(UniqueId, 0); /**< Whether the object's current id is considered unique. */
public:
	DeclareFlag(DaeIdChanged, 1);
	DeclareFlagCount(2);

public:
	/** Constructor: sets the prefix COLLADA id to be used if no COLLADA id is provided.
		@param document The COLLADA document which owns this object.
		@param baseId The prefix COLLADA id to be used if no COLLADA id is provided. */
	FCDObjectWithId(FCDocument* document, const char* baseId = "ObjectWithID");

	/** Destructor. */
	virtual ~FCDObjectWithId();

	/** Retrieves the unique COLLADA id for this object.
		If no unique COLLADA id has been previously generated or provided, this function
		has the side-effect of generating a unique COLLADA id.
		@return The unique COLLADA id. */
	const fm::string& GetDaeId() const;

	/** Sets the COLLADA id for this object.
		There is no guarantee that the given COLLADA id will be used, as it may not be unique.
		You can call the GetDaeId function after this call to retrieve the final, unique COLLADA id.
		@param id The wanted COLLADA id for this object. This COLLADA id does not need to be unique.
			If the COLLADA id is not unique, a new unique COLLADA id will be generated. */
	void SetDaeId(const fm::string& id);

	/** Sets the COLLADA id for this object.
		There is no guarantee that the given COLLADA id will be used, as it may not be unique.
		@param id The wanted COLLADA id for this object. This COLLADA id does not need to be unique.
			If the COLLADA id is not unique, a new unique COLLADA id will be generated and
			this formal variable will be modified to contain the new COLLADA id. */
	void SetDaeId(fm::string& id);

	/** [INTERNAL] Release the unique COLLADA id of an object.
		Use this function wisely, as it leaves the object id-less and without a way to automatically
		generate a COLLADA id. */
	void RemoveDaeId();

	/** [INTERNAL] Clones the object. The unique COLLADA id will be copied over to the clone object.
		Use carefully: when a cloned object with an id is released, it
		does remove the unique COLLADA id from the unique name map.
		@param clone The object clone. */
	void Clone(FCDObjectWithId* clone) const;

	/** Cleans up a given name into a valid COLLADA id.
		This function does no check for uniqueness.
		@param id A name.
		@return A valid COLLADA id. The returned value is a static variable reference.
			If you want to keep this information, copy it to a local value. */
	static fm::string CleanId(const char* id);
	inline static const fm::string CleanId(const fm::string& id) { return CleanId(id.c_str()); } /**< See above. */

	/** Cleans up a given name into a valid COLLADA sub-id.
		This function does no check for uniqueness.
		Sub-ids support even less characters than ids.
		@param sid A sub-id.
		@return A valid COLLADA id. The returned value is a static variable reference.
			If you want to keep this information, copy it to a local value. */
	static fm::string CleanSubId(const char* sid);
	inline static const fm::string CleanSubId(const fm::string& sid) { return CleanSubId(sid.c_str()); } /**< See above. */
};

#endif // __FCD_OBJECT_WITH_ID_H_
