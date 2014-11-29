/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

/**
	@file FCDAnimationClipTools.h
	This file contains the FCDAnimationClipTools namespace.
*/

#ifndef _FCD_ANIMATION_CLIP_TOOLS_H_
#define _FCD_ANIMATION_CLIP_TOOLS_H_

class FCDocument;

namespace FCDAnimationClipTools
{
	/** Resets the times for the animation clips to a given value.
		@param document A COLLADA document that contains animation clips. */
	FCOLLADA_EXPORT void ResetAnimationClipTimes(FCDocument* document, float startValue);
};

#endif // _FCD_ANIMATION_CLIP_TOOLS_H_
