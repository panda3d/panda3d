/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

/**
	@file FCDLightTools.h
	This file defines the FCDLightTools namespace.
*/

#ifndef _FCD_LIGHT_TOOLS_H_
#define _FCD_LIGHT_TOOLS_H_

#ifndef _FCD_PARAMETER_ANIMATABLE_H_
#include "FCDocument/FCDParameterAnimatable.h"
#endif // _FCD_PARAMETER_ANIMATABLE_H_

class FCDLight;

/** Holds commonly-used transformation functions for lights. */
namespace FCDLightTools
{
	/** Loads the penumbra into the light.
		The FCDLight does not contain penumbra information directly, but it 
		can be represented with appropriate values for the outerAngle and the
		fallOffAngle. If the penumbra is positive, then the outerAngle is the
		sum of the penumbra value and the fallOffAngle. If the penumbra is
		negative, then the outerAngle is the fallOffAngle and the fallOffAngle
		is the outerAngle minus the penumbra. This method sets the fallOffAngle
		and outerAngle to correspond to the penumbra given. It takes care of
		the animations as well.
		@param light The light to process.
		@param penumbraValue The value of the penumbra.
		@param penumbraAnimated The animated helper for the loaded penumbra. 
		@param createAnimationChannel True to create animation channel for the 
				falloff angle if needed. False will attach the animation to the
				outer angle's channel if needed. */
	void FCOLLADA_EXPORT LoadPenumbra(FCDLight* light, float penumbraValue, FCDAnimated* penumbraAnimated, bool createAnimationChannel=true);
}

#endif // _FCD_LIGHT_TOOLS_H_
