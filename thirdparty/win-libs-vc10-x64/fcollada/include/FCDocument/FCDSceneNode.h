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
	@file FCDSceneNode.h
	This file contains the FCDSceneNode class.
*/

#ifndef _FCD_SCENE_NODE_
#define _FCD_SCENE_NODE_

#ifndef _FCD_ENTITY_H_
#include "FCDocument/FCDEntity.h"
#endif // _FCD_ENTITY_H_
#ifndef _FCD_TRANSFORM_H_
#include "FCDocument/FCDTransform.h" /** @todo Remove this include by moving the FCDTransform::Type enum to FUDaeEnum.h. */
#endif // _FCD_TRANSFORM_H_
#ifndef _FCD_ENTITY_INSTANCE_H_
#include "FCDocument/FCDEntityInstance.h" /** @todo Remove this include by moving the FCDEntityInstance::Type enum to FUDaeEnum.h. */
#endif // _FCD_ENTITY_INSTANCE_H_

class FCDocument;
class FCDEntityInstance;
class FCDSceneNode;
class FCDTransform;

/**
	A COLLADA visual scene node.
	This class is also used to represent COLLADA visual scene entities.

	A visual scene node contains child scene nodes to make a tree.
	A visual scene node may appear multiple times within the scene graph,
	but checks are made to verify that there are no cycles within the graph.

	A visual scene node also contained an ordered list of transformations
	and a list of entity instances.

	NOTE: The GenerateSampledMatrixAnimation function was moved to the
	FCDSceneNodeTools namespace to improve DLL support.

	@ingroup FCDocument
*/
class FCOLLADA_EXPORT FCDSceneNode : public FCDEntity
{
private:
	DeclareObjectType(FCDEntity);

	// Hierarchy and instances
	DeclareParameterTrackList(FCDSceneNode, parents, FC("Parents"));
	DeclareParameterTrackList(FCDSceneNode, children, FC("Child Nodes"));
	DeclareParameterContainer(FCDTransform, transforms, FC("Transforms"));
	DeclareParameterContainer(FCDEntityInstance, instances, FC("Instances"));

	// Visibility parameter.
	// Should be a boolean, but is animated as float.
	DeclareParameterAnimatable(float, FUParameterQualifiers::SIMPLE, visibility, FC("Visibility"));

	// The number of entities that target this node
	uint32 targetCount;

	// Mainly for joints.
	DeclareParameter(fm::string, FUParameterQualifiers::SIMPLE, daeSubId, FC("Sub-id"));

public:
	DeclareFlag(TransformsDirty, 0); /**< Whether the transforms have been dirtied. */
	DeclareFlag(Joint, 1); /**< Whether the scene node is a joint. */
	DeclareFlagCount(2); /**< . */

public:
	/** Constructor: do not use directly.
		Instead, use the FCDSceneNode::AddChild function for child
		visual scene nodes or the FCDLibrary::AddEntity function
		for visual scenes.
		@param document The COLLADA document that owns the scene node. */
	FCDSceneNode(FCDocument* document);

	/** Destructor. */
	virtual ~FCDSceneNode();

	/** Retrieves the type of the entity class.
		@return The type of entity class: SCENE_NODE. */
	virtual Type GetType() const { return SCENE_NODE; }

	/** Retrieves the number of parent nodes for this visual scene node.
		@return The number of parents. */
	inline size_t GetParentCount() const { return parents.size(); };

	/** Retrieves a specific parent of the visual scene node.
		@param index The index of the parent.
		@return The parent visual scene node. This pointer will be NULL if
			the scene node has no parents or if the index is out-of-bounds. */
	inline FCDSceneNode* GetParent(size_t index = 0) { FUAssert(index == 0 || index < parents.size(), return NULL); return (!parents.empty()) ? parents.at(index) : NULL; }
	inline const FCDSceneNode* GetParent(size_t index = 0) const { FUAssert(index == 0 || index < parents.size(), return NULL); return (!parents.empty()) ? parents.at(index) : NULL; } /**< See above. */

	/** Retrieves the list of parents for the visual scene node.
		@return The list of parents. */
	inline const FCDSceneNode** GetParents() const { return parents.begin(); }

	/** Retrieves the number of child nodes for this visual scene node.
		@return The number of children. */
	inline size_t GetChildrenCount() const { return children.size(); };

	/** Retrieves a specific child of the visual scene node.
		@param index The index of the child.
		@return The child scene node. This pointer will be NULL if the
			index is out-of-bounds. */
	inline FCDSceneNode* GetChild(size_t index) { FUAssert(index < children.size(), return NULL); return children.at(index); }
	inline const FCDSceneNode* GetChild(size_t index) const { FUAssert(index < children.size(), return NULL); return children.at(index); } /**< See above. */

	/** Retrieves the list of children of the visual scene node.
		@return The list of child scene nodes. */
	inline const FCDSceneNode** GetChildren() const { return children.begin(); }

	/** Creates a new child scene node.
		@return The new child scene node. */
	FCDSceneNode* AddChildNode();

	/** Attaches a existing scene node to this visual scene node.
		This function will fail if attaching the given scene node
		to this visual scene node creates a cycle within the scene graph.
		@param sceneNode The scene node to attach.
		@return Whether the given scene node was attached to this scene node. */
	bool AddChildNode(FCDSceneNode* sceneNode);

	/** Removes a scene node from this scene node direct child list.
		This function should be used to detach a scene node with multiple parents.
		To completely delete a scene node, you should use the FCDSceneNode::Release function
		on the scene node to delete.
		If the given child node is instanced multiple times within this scene node,
		only the first instance will be removed.
		@param childNode The child node to remove from the direct hierarchy. */
	void RemoveChildNode(FCDSceneNode* childNode);

	/** Retrieves the number of entity instances at this node of the scene graph.
		@return The number of entity instances. */
	inline size_t GetInstanceCount() const { return instances.size(); };

	/** Retrieves a specific entity instance.
		@param index The index of the instance.
		@return The entity instance at the given index. This pointer will be
			NULL if the index is out-of-bounds. */
	inline FCDEntityInstance* GetInstance(size_t index) { FUAssert(index < instances.size(), return NULL); return instances.at(index); }
	inline const FCDEntityInstance* GetInstance(size_t index) const { FUAssert(index < instances.size(), return NULL); return instances.at(index); } /**< See above. */

	/** Retrieves the list of entity instances at this node of the scene graph.
		@return The list of entity instances. */
	inline const FCDEntityInstance** GetInstances() const { return instances.begin(); }

	/** Creates a new entity instance.
		Only geometric entities, controllers, light and cameras
		can be instantiated in the scene graph.
		To instantiate visual scene nodes, use the AddChildNode function.
		@param entity The entity to instantiate. This pointer cannot be NULL.
		@return The entity instance structure. This pointer will be NULL
			if the entity cannot be instantiated here or if the entity is a scene node. */
	FCDEntityInstance* AddInstance(FCDEntity* entity);

	/** Creates a new entity instance.
		Only geometric entities, controllers, light and cameras
		can be instantiated in the scene graph.
		To instantiate visual scene nodes, use the AddChildNode function.
		@param type The type of entity to instantiate. 
		@return The entity instance structure. This pointer will be NULL
			if the entity cannot be instantiated here. */
	FCDEntityInstance* AddInstance(FCDEntity::Type type);

	/** Retrieves the number of transforms for this node of the scene graph.
		@return The number of transforms. */
	inline size_t GetTransformCount() const { return transforms.size(); };

	/** Retrieves a specific transform.
		@param index The index of the transform.
		@return The transform at the given index. This pointer will be NULL
			if the index is out-of-bounds. */
	inline FCDTransform* GetTransform(size_t index) { FUAssert(index < transforms.size(), return NULL); return transforms.at(index); }
	inline const FCDTransform* GetTransform(size_t index) const { FUAssert(index < transforms.size(), return NULL); return transforms.at(index); } /**< See above. */

	/** Retrieves the list of transforms for this node of the scene graph.
		@return The list of transforms. */
	inline const FCDTransform** GetTransforms() const { return transforms.begin(); } /**< See above. */

	/** Creates a new transform for this visual scene node.
		The transforms are processed in order and COLLADA is column-major.
		For row-major matrix stacks, such as DirectX, this implies that the
		transformations will be processed in reverse order.
		By default, a transform is added at the end of the list.
		@param type The type of transform to create.
		@param index The index at which to insert the transform. Set this value to -1
			to indicate that you want this transform at the end of the stack.
		@return The created transform. */
	FCDTransform* AddTransform(FCDTransform::Type type, size_t index = (size_t)-1);

	/** Retrieves the asset information structures that affect
		this entity in its hierarchy.
		@param assets A list of asset information structures to fill in. */
	inline void GetHierarchicalAssets(FCDAssetList& assets) { GetHierarchicalAssets(*(FCDAssetConstList*) &assets); }
	virtual void GetHierarchicalAssets(FCDAssetConstList& assets) const; /**< See above. */

	/** Retrieves the visual scene node with the given id.
		This function looks through the whole sub-tree of visual scene nodes
		for the wanted COLLADA id.
		@param daeId The COLLADA id to look for.
		@return The visual scene node which has the given COLLADA id. This pointer
			will be NULL if no visual scene node can be found with the given COLLADA id. */
	virtual FCDEntity* FindDaeId(const fm::string& daeId) { return const_cast<FCDEntity*>(const_cast<const FCDSceneNode*>(this)->FindDaeId(daeId)); }
	virtual const FCDEntity* FindDaeId(const fm::string& daeId) const; /** < See above. */

	/** Retrieves the optional sub-id of the node.
		This sub-id is neither unique nor guaranteed to exist.
		@return The sub-id of the node. */
	inline const fm::string& GetSubId() const { return daeSubId; }

	/** Sets the sub-id for this node.
		The sub-id of an object is not required to be unique.
		@param id The new sub-id of the node. */
	void SetSubId(const fm::string& id);

	/** Retrieves the visual scene node with the given sub-id.
		This function looks through the whole sub-tree of visual scene nodes
		for the wanted COLLADA sub-id.
		@param subId The COLLADA sub-id to look for.
		@return The visual scene node which has the given COLLADA sub-id. This pointer
			will be NULL if no visual scene node can be found with the given COLLADA sub-id. */
	inline FCDEntity* FindSubId(const fm::string& subId) { return const_cast<FCDEntity*>(const_cast<const FCDSceneNode*>(this)->FindSubId(subId)); }
	const FCDEntity* FindSubId(const fm::string& subId) const; /**< See above. */

	/** Retrieves whether the visual scene node is visible.
		A hidden visual scene node will not be rendered but will
		still affect the world. This parameter is a floating-point value
		because it is animated. It should be intepreted as a Boolean value.
		@return Whether the scene node is visible. */
	inline FCDParameterAnimatableFloat& GetVisibility() { return visibility; }
	inline const FCDParameterAnimatableFloat& GetVisibility() const { return visibility; } /**< See above. */
	inline bool IsVisible() const { return visibility > 0.5f; } /**< See above. */

	/** Sets the visibility of the visual scene node.
		A hidden visual scene node will not be rendered but will
		still affect the world.
		@param isVisible Whether the visual scene node is visible. */
	inline void SetVisibility(bool isVisible) { visibility = isVisible ? 1.0f : 0.0f; }

	/** Retrieves whether this visual scene node is the target of an entity.
		@return Whether this is an entity target. */
	inline bool IsTarget() const { return targetCount > 0; }

	/** Retrieves whether this visual scene node is a joint.
		Joints are called bones in 3dsMax. A joint is a scene node that is used in skinning.
		@return Whether this node is a joint. */
	DEPRECATED(3.05A, GetJointFlag) bool IsJoint() const { return GetJointFlag(); }

	/** Retrieves the local transform for this visual scene node.
		This function does not handle or apply animations.
		@return The local transform. */
	FMMatrix44 ToMatrix() const;

	/** Retrieves the local transform for this visual scene node.
		This is a shortcut to the above function. The above function
		will be deprecated in the future.
		@return The local transform. */
	inline FMMatrix44 CalculateLocalTransform() const { return ToMatrix(); }

	/** Retrieves the world transform for this visual scene node.
		This function is not optimized and will not work with node instances.
		@return The world transform. */
	FMMatrix44 CalculateWorldTransform() const;

	/** Copies the entity information into a clone.
		All the overwriting functions of this function should call this function
		to copy the COLLADA id and the other entity-level information.
		All the up-classes of this class should implement this function.
		The cloned entity may reside in another document.
		@param clone The empty clone. If this pointer is NULL, a new entity
			will be created and you will need to release the returned pointer manually.
		@param cloneChildren Whether to recursively clone this entity's children.
		@return The clone. */
	virtual FCDEntity* Clone(FCDEntity* clone = NULL, bool cloneChildren = false) const;

	/** [INTERNAL] Increments the number of entities target this node.
		To set targets, use the FCDTargetedEntity::SetTarget function. */
	inline void IncrementTargetCount() { ++targetCount; }

	/** [INTERNAL] Decrements the number of entities target this node.
		To set targets, use the FCDTargetedEntity::SetTarget function. */
	inline void DecrementTargetCount() { if (targetCount > 0) --targetCount; }

	/** [INTERNAL] Cleans up the sub identifiers.
		The sub identifiers must be unique with respect to its parent. This method corrects the sub ids if there are conflicts. */
	virtual void CleanSubId();
};

#endif // _FCD_SCENE_NODE_
