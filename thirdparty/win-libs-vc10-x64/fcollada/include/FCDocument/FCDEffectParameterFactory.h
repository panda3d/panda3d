/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

/**
	@file FCDEffectParameterFactory.h
	This file contains the FCDEffectParameterFactory class.
*/

#ifndef _FCD_EFFECT_PARAMETER_FACTORY_H_
#define _FCD_EFFECT_PARAMETER_FACTORY_H_

class FCDocument;
class FCDEffectParameter;

/**
	[INTERNAL] The factory for COLLADA effect parameters.

	Takes in a COLLADA XML tree and returns a new
	parameter that represent it, if one is possible.

	@ingroup FCDEffect
*/
class FCOLLADA_EXPORT FCDEffectParameterFactory
{
private:
	// Never instantiate: this is a static class
	FCDEffectParameterFactory() {}

public:
	/** [INTERNAL] Creates a new effect parameter, given a type.
		To create new effect parameters, use the appropriate AddEffectParameter function.
		@param document The COLLADA document that will own the effect parameter.
		@param type The type of effect to create.
			This value should reflect the FCDEffectParameter::Type enum. */
	static FCDEffectParameter* Create(FCDocument* document, uint32 type);
};

#endif // _FCD_EFFECT_PARAMETER_FACTORY_H_
