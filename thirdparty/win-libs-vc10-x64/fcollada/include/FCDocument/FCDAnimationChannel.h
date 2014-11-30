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
	@file FCDAnimationChannel.h
	This file contains the FCDAnimationChannel class.
*/

#ifndef _FCD_ANIMATION_CHANNEL_H_
#define _FCD_ANIMATION_CHANNEL_H_

#ifndef __FCD_OBJECT_H_
#include "FCDocument/FCDObject.h"
#endif // __FCD_OBJECT_H_
#ifndef _FU_PARAMETER_H_
#include "FUtils/FUParameter.h"
#endif // _FU_PARAMETER_H_

class FCDAnimated;
class FCDAnimation;
class FCDAnimationCurve;

typedef fm::pvector<FCDAnimationCurve> FCDAnimationCurveList; /**< A dynamically-sized array of animation curves. */

/**
	A COLLADA animation channel.
	Each animation channel holds the animation curves for one animatable element,
	such as a single floating-point value, a 3D vector or a matrix.

	@see FCDAnimated
	@ingroup FCDocument
*/
class FCOLLADA_EXPORT FCDAnimationChannel : public FCDObject
{
private:
	DeclareObjectType(FCDObject);
	FCDAnimation* parent;

	DeclareParameterContainer(FCDAnimationCurve, curves, FC("Animation Curves"));

public:
	/** Constructor: do not use directly.
		Instead, call the FCDAnimation::AddChannel function.
		@param document The COLLADA document that owns the animation channel.
		@param parent The animation sub-tree that contains the animation channel. */
	FCDAnimationChannel(FCDocument* document, FCDAnimation* parent);

	/** Destructor. */
	virtual ~FCDAnimationChannel();
	
	/** Copies the animation channel into a clone.
		The clone may reside in another document.
		@param clone The empty clone. If this pointer is NULL, a new animation channel
			will be created and you will need to release the returned pointer manually.
		@return The clone. */
	FCDAnimationChannel* Clone(FCDAnimationChannel* clone = NULL) const;

	/** Retrieves the animation sub-tree that contains the animation channel.
		@return The parent animation sub-tree. */
	FCDAnimation* GetParent() { return parent; }
	const FCDAnimation* GetParent() const { return parent; } /**< See above. */

	/** Retrieves the list of animation curves contained within the channel.
        @deprecated
	    @return The list of animation curves. */
	DEPRECATED(3.05A, GetCurveCount and GetCurve(index))
    void GetCurves() const {}

	/** Retrieves the number of animation curves contained within the channel.
		@return The number of animation curves. */
	size_t GetCurveCount() const { return curves.size(); }

	/** Retrieves an animation curve contained within the channel.
		@param index The index of the animation curve.
		@return The animation curve at the given index. This pointer will be NULL
			if the index is out-of-bounds. */
	FCDAnimationCurve* GetCurve(size_t index) { FUAssert(index < GetCurveCount(), return NULL); return curves.at(index); }
	const FCDAnimationCurve* GetCurve(size_t index) const { FUAssert(index < GetCurveCount(), return NULL); return curves.at(index); } /**< See above. */

	/** Adds a new animation curve to this animation channel.
		@return The new animation curve. */
	FCDAnimationCurve* AddCurve();
};

#endif // _FCD_ANIMATION_CHANNEL_H_
