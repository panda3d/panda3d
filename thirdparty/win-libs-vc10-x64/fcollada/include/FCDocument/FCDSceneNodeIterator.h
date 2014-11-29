/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/
/**
	@file FCDSceneNodeIterator.h
	This file contains the FCDSceneNodeIteratorT template
	and its two definitions: FCDSceneNodeIterator and FCDSceneNodeConstIterator.
*/

#ifndef _FCD_SCENE_NODE_ITERATOR_H_
#define _FCD_SCENE_NODE_ITERATOR_H_

class FCDSceneNode;

/**
	This template is used to process a given scene node and its full sub-tree.
	We use a template here in order to easily support both const
	and non-const scene node data, with the same code.

	Do not use this template directly, instead use the
	FCDSceneNodeIterator and FCDSceneNodeConstIterator definitions.

	This template does not care whether multiple instances of the same node is processed.
*/
template <class _NODE>
class FCOLLADA_EXPORT FCDSceneNodeIteratorT
{
private:
	fm::pvector<_NODE> queue;
	size_t iterator;

public:
	/** The order in which the scene nodes should be iterated. */
	enum SearchType
	{
		/** Iterate over the scene nodes one level at a time. Starts from the root
			and terminates with the leaves. Each node level is processed fully
			before processing the next one. */
		BREADTH_FIRST, 
		
		/** Standard tree traversal. Processes the root, the first child of the root,
			then the child's first child... until a leaf is reached. Then, the leaf's siblings
			are iterated over before iterating over the leaf's parent's siblings. */
		DEPTH_FIRST_PREORDER,
		
		/** Iterate over the scene nodes one level at a time. Starts at the leaves 
			and terminates with the root. Each node level is processed fully
			before processing the next one. */
		DEPTH_FIRST_POSTORDER,
	};

	/** Constructor.
		@param root The scene root of the sub-tree to iterate over.
		@param searchType The search type determines the ordering of the scene nodes returned by Next.
		@param pureChildOnly Only process nodes that are direct children of the root. All node
			instances will be discarded. */
	FCDSceneNodeIteratorT(_NODE* root, SearchType searchType=BREADTH_FIRST, bool pureChildOnly=false);

	/** Destructor. */
	~FCDSceneNodeIteratorT();

	/** Retrieves the current node to process.
		@return The current node. */
	_NODE* GetNode();

	/** Advances the iteration pointer and retrieves the next node to process.
		@return The node to process. */
	_NODE* Next();

	/** Retrieves whether the full sub-tree has been processed. */
	inline bool IsDone() { return iterator >= queue.size(); }

	/** Advances the iteration pointer.
		@return The iterator. */
	inline FCDSceneNodeIteratorT& operator++() { Next(); return (*this); }

	/** Retrieves the current node to process.
		@return The current node. */
	inline _NODE* operator*() { return GetNode(); }
};

typedef FCDSceneNodeIteratorT<FCDSceneNode> FCDSceneNodeIterator; /**< A scene node iterator. */
typedef FCDSceneNodeIteratorT<const FCDSceneNode> FCDSceneNodeConstIterator; /**< A constant scene node iterator. */

#ifdef __APPLE__
#include "FCDocument/FCDSceneNodeIterator.hpp"
#endif // __APPLE__

#endif // _FCD_SCENE_NODE_ITERATOR_H_
