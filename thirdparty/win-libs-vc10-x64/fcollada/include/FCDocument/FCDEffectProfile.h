/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

/**
	@file FCDEffectProfile.h
	This file contains the FCDEffectProfile abstract class.
*/

#ifndef _FCD_EFFECT_PROFILE_H_
#define _FCD_EFFECT_PROFILE_H_

#ifndef _FU_DAE_ENUM_H_
#include "FUtils/FUDaeEnum.h"
#endif // _FU_DAE_ENUM_H_
#ifndef __FCD_OBJECT_H_
#include "FCDocument/FCDObject.h"
#endif // __FCD_OBJECT_H_

class FCDocument;
class FCDEffect;
class FCDEffectParameter;
class FCDExtra;

/**
	The base for a COLLADA effect profile.

	COLLADA has multiple effect profiles: CG, HLSL, GLSL, GLES and the COMMON profile.
	For each profile, there is a class which implements this abstract class.
	This abstract class solely holds the parent effect and allows access to the
	profile type.

	@see FCDEffectProfileFX FCDEffectStandard
	
	@ingroup FCDEffect
*/
class FCOLLADA_EXPORT FCDEffectProfile : public FCDObject
{
private:
	DeclareObjectType(FCDObject);
	FCDEffect* parent;
	DeclareParameterContainer(FCDEffectParameter, parameters, FC("Effect Parameters"));
	DeclareParameterRef(FCDExtra, extra, FC("Extra Tree"));

public:
	/** Constructor: do not use directly.
		Instead, use the FCDEffect::AddProfile function.
		@param document The FCollada document that owns this effect profile.
		@param parent The effect which contains this profile. */
	FCDEffectProfile(FCDocument* document, FCDEffect* parent);

	/** Destructor. */
	virtual ~FCDEffectProfile();

	/** Retrieves the profile type for this effect.
		This function allows you to up-cast the pointer safely to a more specific
		effect profile class.
		@return The profile type. */
	virtual FUDaeProfileType::Type GetType() const = 0;

	/** Retrieves the parent effect.
		This is the effect which contains this profile.
		@return The parent effect. This pointer will never be NULL. */
	FCDEffect* GetParent() { return parent; }
	const FCDEffect* GetParent() const { return parent; } /**< See above. */

	/** [INTERNAL] Retrieves the COLLADA id of the parent effect.
		This function is useful when reporting errors and warnings.
		@return The COLLADA id of the parent effect. */
	DEPRECATED(3.05A, GetParent()->GetDaeId) const fm::string& GetDaeId() const { return emptyString; }

	/** Retrieves the extra information tree for this effect profile.
		The prefered way to save extra information in FCollada is at
		the entity level: FCDEffect.
		@return The extra information tree. */
	inline FCDExtra* GetExtra() { return extra; }
	inline const FCDExtra* GetExtra() const { return extra; } /**< See above. */

	/** Clones the profile effect and its parameters.
		@param clone The cloned profile.
			If this pointer is NULL, a new profile is created and
			you will need to release this new profile.
		@return The cloned profile. This pointer will be NULL if the
			abstract class' cloning function is used without a given clone. */
	virtual FCDEffectProfile* Clone(FCDEffectProfile* clone = NULL) const;
	
	/** Retrieves the number of local effect parameters
		@return The number of local effect parameters. */
	inline size_t GetEffectParameterCount() const { return parameters.size(); }

	/** Retrieves a given local effect parameter.
		@param index An index.
		@return The local effect parameter at the given index. */
	inline FCDEffectParameter* GetEffectParameter(size_t index) { FUAssert(index < parameters.size(), return NULL); return parameters.at(index); }
	inline const FCDEffectParameter* GetEffectParameter(size_t index) const { FUAssert(index < parameters.size(), return NULL); return parameters.at(index); }

	/** Adds a local effect parameter to the local list.
		@see FCDEffectParameter::Type
		@param type The value type of the effect parameter to create.
		@return The new local effect parameter. */
	FCDEffectParameter* AddEffectParameter(uint32 type);

	/** [INTERNAL] Flattens this effect profile, pushing all the effect parameter overrides
		into the effect parameter generators and moving all the parameters to the 
		effect technique level of abstraction. To flatten the material, use the
		FCDMaterialInstance::FlattenMaterial function. */
	DEPRECATED(3.05A, not recommended) void Flatten() {}
};

#endif // _FCD_EFFECT_PROFILE_H_
