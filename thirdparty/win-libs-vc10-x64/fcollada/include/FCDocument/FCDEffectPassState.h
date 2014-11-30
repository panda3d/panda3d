/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

/**
	@file FCDEffectPassState.h
	This file contains the FCDEffectPassState class.
*/

#ifndef _FCD_EFFECT_PASS_STATE_H_
#define _FCD_EFFECT_PASS_STATE_H_

#ifndef __FCD_OBJECT_H_
#include "FCDocument/FCDObject.h"
#endif // __FCD_OBJECT_H_
#ifndef _FU_DAE_ENUM_H_
#include "FUtils/FUDaeEnum.h"
#endif // _FU_DAE_ENUM_H_
#ifndef _FU_PARAMETER_H_
#include "FUtils/FUParameter.h"
#endif // _FU_PARAMETER_H_

/**
	This class holds the information necessary to set or apply
	one render state for a given pass.
	To get more information about the render state and how to interpret the data
	they hold, see the FUDaePassState namespace and its enumerated type.
	@see FUDaePassState::State
		
	@ingroup FCDEffect
*/
class FCOLLADA_EXPORT FCDEffectPassState : public FCDObject
{
private:
	DeclareObjectType(FCDObject);
	DeclareParameter(uint32, FUParameterQualifiers::SIMPLE, type, FC("Type")); // FUDaePassState::State

	// Data remains state-specific, un-parameterizable and will
	// pretty much always require custom UI.
	uint8* data;
	size_t dataSize;

public:
	/** Constructor.
		Once built, the render state associated with this object should never change
		and the data pointer should be allocated to the correct size and never
		re-allocated.
		@param document The COLLADA document that owns this render state.
		@param renderState The render state type. */
	FCDEffectPassState(FCDocument* document, FUDaePassState::State renderState);

	/** Destructor. */
	virtual ~FCDEffectPassState();

	/** Retrieves the type of the pass render state.
		@return The render state type. */
	FUDaePassState::State GetType() const { return (FUDaePassState::State) *type; }

	/** Retrieves the data size of the pass render state.
		@return The size of the render state data. */
	size_t GetDataSize() const { return dataSize; }

	/** Retrieves the data pointer for the pass render state.
		@return The data pointer. */
	uint8* GetData() { return data; }
	const uint8* GetData() const { return data; } /**< See above. */

	/**	Use this method to reset the state back to its default value. This method
		is called in the constructor.*/
	void SetDefaultValue();

	/** Clones the effect pass and shaders.
		@param clone The cloned pass.
			If this pointer is NULL, a new pass is created and
			you will need to release this new pass.
		@return The cloned pass. */
	FCDEffectPassState* Clone(FCDEffectPassState* clone = NULL) const;
};

#endif // _FCD_EFFECT_PASS_STATE_H_
