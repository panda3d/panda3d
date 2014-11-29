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
	@file FCDAnimation.h
	This file contains the FCDAnimation class.
*/

#ifndef _FCD_ANIMATION_H_
#define _FCD_ANIMATION_H_

#ifndef _FCD_ENTITY_H_
#include "FCDocument/FCDEntity.h"
#endif // _FCD_ENTITY_H_

class FCDocument;
class FCDAnimated;
class FCDAnimation;
class FCDAnimationChannel;
class FCDAnimationCurve;

typedef fm::pvector<FCDAnimationChannel> FCDAnimationChannelList; /**< A dynamically-sized array of animation channels. */
typedef fm::pvector<FCDAnimationCurve> FCDAnimationCurveList; /**< A dynamically-sized array of animation curves. */

/**
	A COLLADA animation entity.
	An animation entity contains a list of child animation entities,
	in order to form a tree of animation entities.
	It also hold a list of animation channels, which hold the information
	to generate animation curves.

	In other words, the animation entity is a structural class
	used to group animation channels hierarchically.

	@ingroup FCDocument
*/
class FCOLLADA_EXPORT FCDAnimation : public FCDEntity
{
private:
	DeclareObjectType(FCDEntity);

	// Animation hierarchy
	FCDAnimation* parent;
	DeclareParameterContainer(FCDAnimation, children, FC("Children"));

	// Animation sources and channels
	DeclareParameterContainer(FCDAnimationChannel, channels, FC("Channels"));

public:
	/** Constructor. Do not use directly.
		Instead, use the FCDLibrary::AddEntity function
		or the AddChild function, depending on the
		hierarchical level of the animation entity.
		@param document The FCollada document that owns the animation entity.
		@param parent The parent animation entity. This pointer will be NULL for root animation entities. */
	FCDAnimation(FCDocument* document, FCDAnimation* parent = NULL);

	/** Destructor .*/
	virtual ~FCDAnimation();

	/** Retrieves the entity class type.
		This function is a part of the FCDEntity interface.
		@return The entity class type: ANIMATION. */
	virtual Type GetType() const { return ANIMATION; }

	/** Retrieves the parent of the animation structure.
		@return The animation parent. This pointer will be NULL
			to indicate a root-level animation structure that is
			contained within the animation library. */
	inline FCDAnimation* GetParent() { return parent; }
	inline const FCDAnimation* GetParent() const { return parent; } /**< See above. */
    
	/** Copies the animation tree into a clone.
		The clone may reside in another document.
		@param clone The empty clone. If this pointer is NULL, a new animation tree
			will be created and you will need to release the returned pointer manually.
		@param cloneChildren Whether to recursively clone this entity's children.
		@return The clone. */
	virtual FCDEntity* Clone(FCDEntity* clone = NULL, bool cloneChildren = false) const;

	/** Retrieves the entity with the given COLLADA id.
		This function will look through the local sub-tree of animations
		for the given COLLADA id.
		@param daeId A COLLADA id.
		@return The animation entity that matches the COLLADA id. This pointer
			will be NULL if there are no animation entities that matches the COLLADA id. */
	virtual FCDEntity* FindDaeId(const fm::string& daeId) { return const_cast<FCDEntity*>(const_cast<const FCDAnimation*>(this)->FindDaeId(daeId)); }
	virtual const FCDEntity* FindDaeId(const fm::string& daeId) const; /**< See above. */

	/** Retrieves the number of animation entity sub-trees contained
		by this animation entity tree.
		@return The number of animation entity sub-trees. */
	inline size_t GetChildrenCount() const { return children.size(); }

	/** Retrieves an animation entity sub-tree contained by this
		animation entity tree.
		@param index The index of the sub-tree.
		@return The animation entity sub-tree at the given index. This pointer will
			be NULL if the index is out-of-bounds. */
	inline FCDAnimation* GetChild(size_t index) { FUAssert(index < children.size(), return NULL); return children.at(index); }
	inline const FCDAnimation* GetChild(size_t index) const { FUAssert(index < children.size(), return NULL); return children.at(index); } /**< See above. */

	/** Creates a new animation entity sub-tree contained within this animation entity tree.
		@return The new animation sub-tree. */
	FCDAnimation* AddChild();

	/** Retrieves the asset information structures that affect
		this entity in its hierarchy.
		@param assets A list of asset information structures to fill in. */
	inline void GetHierarchicalAssets(FCDAssetList& assets) { GetHierarchicalAssets(*(FCDAssetConstList*) &assets); }
	virtual void GetHierarchicalAssets(FCDAssetConstList& assets) const; /**< See above. */

	/** Retrieves the animation channels that target the given COLLADA target pointer.
		@param pointer A COLLADA target pointer.
		@param targetChannels A list of animation channels to fill in.
			This list is not cleared. */
	void FindAnimationChannels(const fm::string& pointer, FCDAnimationChannelList& targetChannels);

	/** Retrieves the number of animation channels at this level within the animation tree.
		@return The number of animation channels. */
	size_t GetChannelCount() const { return channels.size(); }

	/** Retrieves an animation channel contained by this animation entity.
		@param index The index of the channel.
		@return The channel at the given index. This pointer will be NULL
			if the index is out-of-bounds. */
	FCDAnimationChannel* GetChannel(size_t index) { FUAssert(index < GetChannelCount(), return NULL); return channels.at(index); }
	const FCDAnimationChannel* GetChannel(size_t index) const { FUAssert(index < GetChannelCount(), return NULL); return channels.at(index); } /**< See above. */

	/** [INTERNAL] Retrieves the channels' list
        @deprecated
		@return The list of channels */
	DEPRECATED(3.05A, GetChannelCount and GetChannel(index))
    void GetChannels() const {}

	/** Adds a new animation channel to this animation entity.
		@return The new animation channel. */
	FCDAnimationChannel* AddChannel();

	/** Retrieves all the curves created in the subtree of this animation element.
		@param curves A list of animation curves to fill in.
			This list is not cleared. */
	void GetCurves(FCDAnimationCurveList& curves);
};

#endif // _FCD_ANIMATION_H_

