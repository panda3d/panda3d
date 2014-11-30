/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

#ifndef _FCD_SCENE_NODE_H_
#include "FCDocument/FCDSceneNode.h"
#endif // _FCD_SCENE_NODE_H_

//
// FCDSceneNodeIterator
//

template <class NODE_T>
FCDSceneNodeIteratorT<NODE_T>::FCDSceneNodeIteratorT(NODE_T* root, SearchType searchType, bool pureChildOnly)
{
	queue.reserve(8);

	if (searchType == BREADTH_FIRST)
	{
		queue.push_back(root);

		// Fill in the queue completely,
		// in case the user using this iterator wants to modify stuff.
		for (size_t index = 0; index < queue.size(); ++index)
		{
			NODE_T* it = queue[index];
			size_t childCount = it->GetChildrenCount();
			for (size_t c = 0; c < childCount; ++c)
			{
				NODE_T* node = it->GetChild(c);
				if (!pureChildOnly || node->GetParent(0) == it) queue.push_back(node);
			}

			if (!pureChildOnly)
			{
				size_t instanceCount = it->GetInstanceCount();
				for (size_t i = 0; i < instanceCount; ++i)
				{
					const FCDEntity* entity = it->GetInstance(i)->GetEntity();
					if (entity != NULL && entity->HasType(FCDSceneNode::GetClassType())) queue.push_back((NODE_T*) entity);
				}
			}
		}
	}
	else // depth first
	{
		// In depth-first mode, we need a breadth-first processing queue and a depth-first iteration queue.
		typedef fm::pair<NODE_T*,size_t> Child;
		fm::vector<Child> processQueue;
		processQueue.reserve(8);
		processQueue.push_back(Child(root, 0));
		if (searchType == DEPTH_FIRST_PREORDER) queue.push_back(root);

		while (!processQueue.empty())
		{
			Child& it = processQueue.back();
			size_t nodeChildCount = it.first->GetChildrenCount();
			size_t nodeInstanceCount = it.first->GetInstanceCount();
			if (it.second < nodeChildCount)
			{
				NODE_T* add = it.first->GetChild(it.second++);
				if (pureChildOnly && add->GetParent(0) != it.first) continue;
				if (searchType == DEPTH_FIRST_PREORDER) queue.push_back(add);
				if (add->GetChildrenCount() == 0 && add->GetInstanceCount() == 0)
				{
					// Don't bother processing this node.
					if (searchType == DEPTH_FIRST_POSTORDER) queue.push_back(add);
				}
				else
				{
					processQueue.push_back(Child(add, 0));
				}
			}
			else if (it.second < nodeChildCount + nodeInstanceCount && !pureChildOnly)
			{
				for (; it.second < nodeChildCount + nodeInstanceCount; ++it.second)
				{
					size_t instanceIndex = it.second - nodeChildCount;
					const FCDEntity* entity = it.first->GetInstance(instanceIndex)->GetEntity();
					if (entity != NULL && entity->HasType(FCDSceneNode::GetClassType()))
					{
						NODE_T* add = (NODE_T*) entity;
						if (add->GetChildrenCount() == 0 && add->GetInstanceCount() == 0)
						{
							// Don't bother processing this node.
							queue.push_back(add);
						}
						else
						{
							processQueue.push_back(Child(add, 0));
							++(it.second);
							break;
						}
					}
				}
			}
			else
			{
				if (searchType == DEPTH_FIRST_POSTORDER) queue.push_back(it.first);
				processQueue.pop_back();
			}
		}
	}

	iterator = 0;
}

template <class NODE_T>
FCDSceneNodeIteratorT<NODE_T>::~FCDSceneNodeIteratorT()
{
	queue.clear();
}

template <class NODE_T>
NODE_T* FCDSceneNodeIteratorT<NODE_T>::GetNode()
{
	return iterator < queue.size() ? queue[iterator] : NULL;
}

template <class NODE_T>
NODE_T* FCDSceneNodeIteratorT<NODE_T>::Next()
{
	++iterator;
	return iterator < queue.size() ? queue[iterator] : NULL;
}
