/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

#ifndef _FCD_FORCE_FIELD_H_
#define _FCD_FORCE_FIELD_H_

class FCDocument;
class FCDExtra;
class FCDForce;

#ifndef _FCD_FORCE_TYPED_H_
#include "FCDocument/FCDForceTyped.h"
#endif // _FCD_FORCE_TYPED_H_
#ifndef _FCD_ENTITY_H_
#include "FCDocument/FCDEntity.h"
#endif // _FCD_ENTITY_H_

/**
	A COLLADA physics force field.

	This class does not have any parameters in the COMMON profile.
	You can use the custom extra tree to enter/retrieve your
	specific customized information.

	@ingroup FCDocument
*/

class FCOLLADA_EXPORT FCDForceField : public FCDEntity
{
private:
	DeclareObjectType(FCDEntity);
	DeclareParameterRef(FCDExtra, information, FC("Information"));

public:
	/** Constructor.
		@param document The COLLADA document that owns the force field. */
	FCDForceField(FCDocument* document);

	/** Destructor. */
	virtual ~FCDForceField();

	/** Retrieves the extra tree for all the force field information.
		@return The extra tree. */
	inline FCDExtra* GetInformation() { return const_cast<FCDExtra*>(const_cast<const FCDForceField*>(this)->GetInformation()); }
	const FCDExtra* GetInformation() const;

	/** [INTERNAL] Set the information.
		@param info The new information to set. */
	inline void SetInformation(FCDExtra* info){ information = info; }

	/** Retrieves the entity class type.
		@return The entity class type: FORCE_FIELD */
	virtual Type GetType() const { return FORCE_FIELD; }

	/** Clones the force field information.
		@param clone The cloned force field.
			If this pointer is NULL, a new force field is created and
			you will need to release it manually.
		@param cloneChildren Whether to recursively clone this entity's children.
		@return The clone. */
	virtual FCDEntity* Clone(FCDEntity* clone = NULL, bool cloneChildren = false) const;
};

#endif // _FCD_FORCE_FIELD_H_
