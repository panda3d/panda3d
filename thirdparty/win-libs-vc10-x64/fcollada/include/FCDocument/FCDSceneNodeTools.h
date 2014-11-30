/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

/**
	@file FCDSceneNodeTools.h
	This file contains the FCDSceneNodeTools namespace.
*/

#ifndef _FCD_SCENE_NODE_TOOLS_H_
#define _FCD_SCENE_NODE_TOOLS_H_

class FCDSceneNode;

/** A set of tools that operates or modifies visual scene nodes. */
namespace FCDSceneNodeTools
{
	/** Generate a list of matrices, with corresponding key times that
		represent an animation curve for the local transform of a scene node.
		This function will <b>permanently</b> modify the transforms of this visual scene node.
		<p>Because of DLL support, this function was split into four functions
		and only one animation curve is available at one time within this
		namespace. First, call GenerateSampledAnimation. Then, call
		GetSampledAnimationKeys and GetSampledAnimationMatrices to process
		or retrieve the resulting animation curve. Finally, optionally
		call ClearSampledAnimation in order to free up the internal memory buffers.
		Every call to GenerateSampledAnimation start by calling ClearSampledAnimation.
		@param sceneNode The scene node. */
	FCOLLADA_EXPORT void GenerateSampledAnimation(FCDSceneNode* sceneNode);

	/** Retrieves the generated sampled animation curve's keys.
		@see GenerateSampledAnimation.
		@return The generated sampled animation curve's keys. */
	FCOLLADA_EXPORT const FloatList& GetSampledAnimationKeys();

	/** Retrieves the generated sampled animation curve's values.
		@see GenerateSampledAnimation.
		@return The generated sampled animation curve's values. */
	FCOLLADA_EXPORT const FMMatrix44List& GetSampledAnimationMatrices();

	/** Frees up the memory used by any previous call to GenerateSampledAnimation.
		It is recommended, but not necessary, to call this function after retrieving
		and copying locally the keys and values for the generated sampled
		animation curve. */
	FCOLLADA_EXPORT void ClearSampledAnimation();
};

#endif // _FCD_SCENE_NODE_TOOLS_H_
