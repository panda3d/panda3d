/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

/**
	@file FCDAnimationClip.h
	This file contains the FCDAnimationClip class.
*/

#ifndef _FCD_ANIMATION_CLIP_H_
#define _FCD_ANIMATION_CLIP_H_

class FCDocument;
class FCDAnimation;
class FCDAnimationCurve;

class FCDEntityInstance;

typedef FUTrackedList<FCDAnimationCurve> FCDAnimationCurveTrackList; /**< A dynamically-sized tracking array of animation curves. */

#ifndef _FCD_ENTITY_H_
#include "FCDocument/FCDEntity.h"
#endif // _FCD_ENTITY_H_

#ifndef _FCD_ENTITY_INSTANCE_H_
#include "FCDocument/FCDEntityInstance.h"
#endif // _FCD_ENTITY__INSTANCE_H_

/**
	A COLLADA animation clip.

	Animation clips are used to group together animation segments.
	Animation clips are typically used to form complex animation sequences
	where all the curves should only be used simultaneously.

	@ingroup FCDocument
*/
class FCOLLADA_EXPORT FCDAnimationClip : public FCDEntity
{
private:
	DeclareObjectType(FCDEntity);
	FCDAnimationCurveTrackList curves;
	
	DeclareParameter(float, FUParameterQualifiers::SIMPLE, start, FC("Start Time"));
	DeclareParameter(float, FUParameterQualifiers::SIMPLE, end, FC("End Time"));
	DeclareParameterContainer(FCDEntityInstance, animations, FC("Animation Instances"));
	StringList names; // names are supported on animation_instances

public:
	/** Constructor.
		@param document The COLLADA document that holds this animation clip. */
	FCDAnimationClip(FCDocument* document);

	/** Destructor. */
	virtual ~FCDAnimationClip();

	/** Copies the animation clip entity into a clone.
		The clone may reside in another document.
		@param clone The empty clone. If this pointer is NULL, a new animation clip
			will be created and you will need to release the returned pointer manually.
		@param cloneChildren Whether to recursively clone this entity's children.
		@return The clone. */
	virtual FCDEntity* Clone(FCDEntity* clone = NULL, bool cloneChildren = false) const;

	/** Retrieves the entity type for this class. This function is part
		of the FCDEntity class interface.
		@return The entity type: IMAGE. */
	virtual Type GetType() const { return ANIMATION_CLIP; }

	/** Retrieves the list of curves that are used by this animation clip.
		@return The list of curves for the clip. */
	FCDAnimationCurveTrackList& GetClipCurves() { return curves; }
	const FCDAnimationCurveTrackList& GetClipCurves() const { return curves; } /**< See above. */

	/** Inserts an existing curve within this animation clip.
		@param curve An animation curve to be used within this clip. */
	void AddClipCurve(FCDAnimationCurve* curve);

	/** Retrieves the start time marker position for this animation clip.
		When using the animation clip, all the animation curves will need
		to be synchronized in order for the animation to start at the start time.
		@return The start time marker position, in seconds. */
	float GetStart() const { return start; }

	/** Sets the start time marker position for this animation clip.
		@param _start The new start time marker position. */
	void SetStart(float _start) { start = _start; SetDirtyFlag(); } 

	/** Retrieves the end time marker position for this animation clip.
		When using the animation clip, all the animation curves will need
		to be synchronized in order for the animation to complete at the end time.
		@return The end time marker position, in seconds. */
	float GetEnd() const { return end; }

	/** Sets the end time marker position for this animation clip.
		@param _end The end time marker position. */
	void SetEnd(float _end) { end = _end; SetDirtyFlag(); }

	/** Retrieves the number of instanced animations within this animation clip.
        @return The number of instanced animations. */
	inline size_t GetAnimationCount() const { return animations.size(); }

    /** Retrieves a given animation instanced by this clip.
        @param index The index of the animation to retrieve.
        @return The animation object at the given index. */
	inline FCDAnimation* GetAnimation(size_t index) const { FUAssert(GetAnimationCount() > index, return NULL); return (FCDAnimation*) animations[index]->GetEntity(); };

    /** Sets the name of the animation at a given index.
        @param name The name to give the animation at the given index.
        @param index The index of the animation that will get the new name. */
	inline void SetAnimationName(const fm::string& name, size_t index) { if (names.size() <= index) names.resize(index + 1); names[index] = name; }
    
    /** Retrieves the name of the animation at a given index.
        @param index The index of the animation.
        @return The name of the animation. */
	inline fm::string GetAnimationName(size_t index) const { return names[index]; }

    /** [INTERNAL] Adds an animation instance.
        @return The empty animation instance. */
	FCDEntityInstance* AddInstanceAnimation();
    
    /** [INTERNAL] Adds an animation instance.
        @param animation The animation to instance.
        @return The animation instance. */
	FCDEntityInstance* AddInstanceAnimation(FCDAnimation* animation);
};

#endif // _FCD_ANIMATION_CLIP_H_

