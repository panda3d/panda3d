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
	@file FCDEffectProfileFX.h
	This file declares the FCDEffectProfileFX class.
*/

#ifndef _FCD_EFFECT_PROFILE_FX_H_
#define _FCD_EFFECT_PROFILE_FX_H_

#ifndef _FU_DAE_ENUM_H_
#include "FUtils/FUDaeEnum.h"
#endif // _FU_DAE_ENUM_H_
#ifndef _FCD_EFFECT_PROFILE_H_
#include "FCDocument/FCDEffectProfile.h"
#endif // _FCD_EFFECT_PROFILE_H_

class FCDocument;
class FCDEffect;
class FCDEffectCode;
class FCDEffectParameter;
class FCDEffectParameterSurface;
class FCDEffectTechnique;

/**
	A general effect profile description.

	The general effect profile contains all the information necessary
	to implement the advanced effect profiles, such as CG, HLSL, GLSL and GLES.
	Since these effect profiles contains extremely similar information, they
	use the same description structure. For the COMMON profile,
	see the FCDEffectStandard class.

	You should use the GetType function to figure out which profile this structure
	addresses. You can then retrieve one or many of the FCDEffectTechnique objects
	that describe how to render for this profile. You may want to check the
	FCDEffectMaterialTechniqueHint objects at the FCDMaterial level, in order to
	determine which technique(s) to use for your platform. At the profile
	level of abstraction, parameters may be generated within the FCDEffectParamterList.

	@ingroup FCDEffect
*/
class FCOLLADA_EXPORT FCDEffectProfileFX : public FCDEffectProfile
{
private:
	DeclareObjectType(FCDEffectProfile);
	DeclareParameter(uint32, FUParameterQualifiers::SIMPLE, type, FC("Profile Language")); // FUDaeProfileType::Type
	DeclareParameter(fstring, FUParameterQualifiers::SIMPLE, platform, FC("Platform"));

	DeclareParameterContainer(FCDEffectCode, codes, FC("Code Inclusions"));
	DeclareParameterContainer(FCDEffectTechnique, techniques, FC("Effect Techniques"));

public:
	/** Constructor: do not use directly. Instead, use the FCDEffect::AddProfile function.
		@param parent The effect which contains this profile.
		@param type The type of profile. */
	FCDEffectProfileFX(FCDocument* document, FCDEffect* parent);

	/** Destructor. */
	virtual ~FCDEffectProfileFX();

	/** Retrieves the profile type for this effect.
		This function is a part of the FCDEffectProfile interface and allows you
		to up-cast an effect profile pointer safely to this class.
		@return The profile type. This should never be the value: 'COMMON',
			but all other profiles currently derive from this class. */
	virtual FUDaeProfileType::Type GetType() const { return (FUDaeProfileType::Type) *type; }

	/** Sets the profile type for this effect.
		Do not change the profile type of a completed effect. */
	void SetType(FUDaeProfileType::Type _type) { type = _type; }

	/** Retrieves the name of the platform in which to use the effect profile.
		This parameter is very optional.
		@return The platform name. */
	const fstring& GetPlatform() const { return platform; }

	/** Sets the name of the platform in which to use the effect profile.
		This parameter is very optional.
		@param _platform The platform name. */
	void SetPlatform(const fstring& _platform) { platform = _platform; SetDirtyFlag(); }

	/** Retrieves the list of techniques contained within this effect profile.
		You may want to check the FCDEffectMaterialTechniqueHint objects at the FCDMaterial level,
		in order to determine which technique(s) to use for your platform.
		@return The list of inner techniques. */
	DEPRECATED(3.05A, GetTechniqueCount and GetTechnique(index)) void GetTechniqueList() const {}

	/** Retrieves the number of techniques contained within this effect profile.
		@return The number of inner techniques. */
	size_t GetTechniqueCount() const { return techniques.size(); }

	/** Retrieves a technique contained within this effect profile.
		You may want to check the FCDEffectMaterialTechniqueHint objects at the FCDMaterial level,
		in order to determine which technique(s) to use for your platform.
		@param index The index of the technique.
		@return The inner technique. This pointer will be NULL if the index is out-of-bounds. */
	FCDEffectTechnique* GetTechnique(size_t index) { FUAssert(index < GetTechniqueCount(), return NULL); return techniques.at(index); }
	const FCDEffectTechnique* GetTechnique(size_t index) const { FUAssert(index < GetTechniqueCount(), return NULL); return techniques.at(index); } /**< See above. */

	/** Adds a new technique to this effect profile.
		@return The new technique object. */
	FCDEffectTechnique* AddTechnique();

	/** Retrieves the list of code inclusions.
		@return The list of code inclusions. */		
	DEPRECATED(3.05A, GetCodeCount and GetCode(index)) void GetCodeList() {}

	/** Retrieves the number of code inclusions contained within the effect profile.
		@return The number of code inclusions. */
	size_t GetCodeCount() const { return codes.size(); }

	/** Retrieves a code inclusion contained within the effect profile.
		@param index The index of the code inclusion.
		@return The code inclusion. This pointer will be NULL if the index is out-of-bounds. */
	inline FCDEffectCode* GetCode(size_t index) { FUAssert(index < GetCodeCount(), return NULL); return codes.at(index); }
	inline const FCDEffectCode* GetCode(size_t index) const { FUAssert(index < GetCodeCount(), return NULL); return codes.at(index); } /**< See above. */

	/** Retrieves the code inclusion with the given sub-id.
		@param sid A COLLADA sub-id.
		@return The code inclusion with the given sub-id. This pointer will be NULL,
			if there are no code inclusions that match the given sub-id. */
	inline FCDEffectCode* FindCode(const char* sid) { return const_cast<FCDEffectCode*>(const_cast<const FCDEffectProfileFX*>(this)->FindCode(sid)); }
	const FCDEffectCode* FindCode(const char* sid) const; /**< See above. */

	/** Adds a new code inclusion to this effect profile.
		@return The new code inclusion. */
	FCDEffectCode* AddCode();

	/** Clones the full effect profile.
		@param clone The cloned profile.
			If this pointer is NULL, a new profile is created and
			you will need to release this new profile.
		@return The cloned profile. This pointer will never be NULL. */
	virtual FCDEffectProfile* Clone(FCDEffectProfile* clone = NULL) const;

	/** [INTERNAL] Flattens this effect profile. Pushes all the effect parameter overrides
		into the effect parameter generators and moves all the parameters to the 
		effect technique level of abstraction. To flatten the material, use the
		FCDMaterialInstance::FlattenMaterial function. */
	DEPRECATED(3.05A, not recommended) void Flatten() {}
};

#endif // _FCD_EFFECT_PROFILE_H_
