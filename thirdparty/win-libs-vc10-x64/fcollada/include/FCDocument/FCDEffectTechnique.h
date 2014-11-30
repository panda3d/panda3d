/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

/**
	@file FCDEffectTechnique.h
	This file declares the FCDEffectTechnique class.
*/

#ifndef _FCD_EFFECT_TECHNIQUE_H_
#define _FCD_EFFECT_TECHNIQUE_H_

#ifndef __FCD_OBJECT_H_
#include "FCDocument/FCDObject.h"
#endif // __FCD_OBJECT_H_
#ifndef _FU_PARAMETER_H_
#include "FUtils/FUParameter.h"
#endif // _FU_PARAMETER_H_

class FCDEffectCode;
class FCDEffectPass;
class FCDEffectParameter;
class FCDEffectProfileFX;

typedef fm::pvector<FCDEffectPass> FCDEffectPassList; /**< A dynamically-sized array of effect passes. */
typedef fm::pvector<FCDEffectCode> FCDEffectCodeList; /**< A dynamically-sized array of effect code inclusions. */

/**
	A COLLADA effect technique.

	The COLLADA effect technique contains the passes to be used in the rendering of
	polygon sets.
	
	It also contains a list of effect parameters: both generators and overrides
	and it is the lowest level of abstraction in which you can access effect parameters. For 
	flattened materials, this means that all the effect parameters will be accessible at this level.

	It also contains a list of effect code inclusions.

	@ingroup FCDEffect
*/
class FCOLLADA_EXPORT FCDEffectTechnique : public FCDObject
{
private:
	DeclareObjectType(FCDObject);
	FCDEffectProfileFX* parent;

	DeclareParameter(fstring, FUParameterQualifiers::SIMPLE, name, FC("Name"));
	DeclareParameterContainer(FCDEffectCode, codes, FC("Code Inclusions"));
	DeclareParameterContainer(FCDEffectPass, passes, FC("Passes"))
	DeclareParameterContainer(FCDEffectParameter, parameters, FC("Effect Parameters"));

public:
	/** Constructor: do not use directly.
		Instead, use the FCDEffectProfileFX::AddTechnique function. 
		@param document The FCollada document that owns this technique.
		@param parent The effect profile which contains the technique. */
	FCDEffectTechnique(FCDocument* document, FCDEffectProfileFX* parent);

	/** Destructor. */
	virtual ~FCDEffectTechnique();

	/** Retrieves the effect profile that contains this technique.
		@return The parent effect profile. */
	inline FCDEffectProfileFX* GetParent() { return parent; }
	inline const FCDEffectProfileFX* GetParent() const { return parent; } /**< See above. */

	/** Retrieves the COLLADA id of the parent effect.
		This function is mostly useful as a shortcut for debugging and reporting.
		@return The COLLADA id of the parent effect. */
	DEPRECATED(3.05A, GetParent()->GetParent()->GetDaeId) const fm::string& GetDaeId() const { return emptyString; }

	/** Retrieves the sub-id of the technique.
		@return The sub-id of the technique. */
	const fstring& GetName() const { return name; }

	/** Sets the sub-id of the technique.
		The effect technique must have a valid sub-id that is unique
		within its scope. Otherwise, one will be provided on XML export.
		@param _name A valid sub-id. */
	void SetName(const fstring& _name) { name = _name; SetDirtyFlag(); }

	/** Retrieves the list of passes.
		@return The list of passes. */
	DEPRECATED(3.05A, GetPassCount and GetPass(index)) void GetPassList() {}

	/** Retrieves the number of passes contained within this effect technique.
		@return The number of passes. */
	size_t GetPassCount() const { return passes.size(); }

	/** Retrieves a specific pass contained within this effect technique.
		@param index The index of the pass.
		@return The pass. This pointer will be NULL if the index is out-of-bounds. */
	FCDEffectPass* GetPass(size_t index) { FUAssert(index < GetPassCount(), return NULL); return passes.at(index); }
	const FCDEffectPass* GetPass(size_t index) const { FUAssert(index < GetPassCount(), return NULL); return passes.at(index); } /**< See above. */

	/** Adds a new pass to this effect technique.
		@return The new pass. */
	FCDEffectPass* AddPass();

	/** Retrieves the list of code inclusions.
		@return The list of code inclusions. */		
	DEPRECATED(3.05A, GetCodeCount and GetCode(index) or FindCode) void GetCodeList() {}

	/** Retrieves the number of code inclusions contained within the effect profile.
		@return The number of code inclusions. */
	size_t GetCodeCount() const { return codes.size(); }

	/** Retrieves a code inclusion contained within the effect profile.
		@param index The index of the code inclusion.
		@return The code inclusion. This pointer will be NULL if the index is out-of-bounds. */
	FCDEffectCode* GetCode(size_t index) { FUAssert(index < GetCodeCount(), return NULL); return codes.at(index); }
	const FCDEffectCode* GetCode(size_t index) const { FUAssert(index < GetCodeCount(), return NULL); return codes.at(index); } /**< See above. */

	/** Retrieves the code inclusion with the given sub-id.
		@param sid A COLLADA sub-id.
		@return The code inclusion with the given sub-id. This pointer will be NULL,
			if there are no code inclusions that match the given sub-id. */
	inline FCDEffectCode* FindCode(const char* sid) { return const_cast<FCDEffectCode*>(const_cast<const FCDEffectTechnique*>(this)->FindCode(sid)); }
	const FCDEffectCode* FindCode(const char* sid) const; /**< See above. */

	/** Adds a new code inclusion to this effect profile.
		@return The new code inclusion. */
	FCDEffectCode* AddCode();
	
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

	/** [INTERNAL] Clones the full effect technique.
		@param clone The cloned technique.
			If this pointer is NULL, a new technique is created and
			you will need to release this new techniquetechnique.
		@return The cloned technique. This pointer will never be NULL. */
	FCDEffectTechnique* Clone(FCDEffectTechnique* clone = NULL) const;

	/** [INTERNAL] Flattens this effect technique.
		Merges the parameter overrides into the parameter generators. */
	DEPRECATED(3.05A, not recommended) void Flatten() {}
};

#endif
