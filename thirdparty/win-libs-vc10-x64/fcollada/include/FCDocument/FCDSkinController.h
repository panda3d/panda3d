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
	@file FCDSkinController.h
	This file contains the FCDSkinController class.
*/

#ifndef _FCD_SKIN_CONTROLLER_H_
#define _FCD_SKIN_CONTROLLER_H_

#ifndef __FCD_OBJECT_H_
#include "FCDocument/FCDObject.h"
#endif // __FCD_OBJECT_H_
#ifndef _FU_PARAMETER_H_
#include "FUtils/FUParameter.h"
#endif // _FU_PARAMETER_H_

class FCDocument;
class FCDController;
class FCDGeometry;
class FCDSceneNode;
class FCDEntityReference; 
class FUUri;

/**
	A COLLADA weighted vertex-joint binding used in skinning.
	@ingroup FCDGeometry
*/
struct FCOLLADA_EXPORT FCDJointWeightPair
{
	/** Default constructor: sets both the joint index and the weight to zero. */
	FCDJointWeightPair() { jointIndex = -1; weight = 0.0f; }

	/** Constructor: sets the joint index and the weight to the given values.
		@param _jointIndex The jointIndex.
		@param _weight Its weight. */
	FCDJointWeightPair(int32 _jointIndex, float _weight) { jointIndex = _jointIndex; weight = _weight; }

	/** A joint index.
		Use this index within the skin's joint list.
		Look-out for the special joint index: -1. It indicates that
		the bind-shape position should be used. */
	int32 jointIndex;

	/** The weight of this influence on the vertex. */
	float weight;
};

/**
	A COLLADA skin controller vertex.
	This structure contains a list of joint-weight pairs that defines
	how to modify a given mesh vertex in order to skin it properly.
	@ingroup FCDGeometry
*/
class FCOLLADA_EXPORT FCDSkinControllerVertex
{
private:
	fm::vector<FCDJointWeightPair> pairs;

public:
	/** Retrieve the number of joint-weight pairs defined for this vertex.
		@return The number of joint-weight pairs. */
	inline size_t GetPairCount() const { return pairs.size(); }

	/** Sets the number of joint-weight pairs defined for this vertex.
		@param count The number of joint-weight pairs defined for this vertex. */
	void SetPairCount(size_t count);

	/** Retrieves a joint-weight pair.
		@param index The index of the joint-weight pair.
		@return The joint-weight pair at the given index. */
	inline FCDJointWeightPair* GetPair(size_t index) { FUAssert(index < pairs.size(), return NULL); return &(pairs.at(index)); }
	inline const FCDJointWeightPair* GetPair(size_t index) const { FUAssert(index < pairs.size(), return NULL); return &(pairs.at(index)); } /**< See above. */

	/** Adds a new joint-weight pair to this vertex.
		No verification will be made to ensure that the sum of the weights equal 1.0.
		@param jointIndex The index of the joint within the skin controller's joint list.
		@param weight The influence weight for this joint, on this vertex. */
	void AddPair(int32 jointIndex, float weight);
};

/**
	A COLLADA skin controller joint.
	The controller does not reference the scene nodes directly: that's the instance's job.
	Instead, the skin controllers keeps track of the sub-ids of the scene nodes and their
	bind poses.
	@ingroup FCDGeometry
*/
class FCOLLADA_EXPORT FCDSkinControllerJoint
{
private:
	fm::string id;
	FMMatrix44 bindPoseInverse;

public:
	/** Retrieves the identifier of the scene node(s) representing this joint.
		@return The identifier of the joint. */
	inline const fm::string& GetId() const { return id; }

	/** Sets the identifier of the scene node(s) representing this joint.
		@param id The identifier of the joint. */
	void SetId(const fm::string& id);

	/** Retrieves the inverse bind-pose matrix of the joint.
		@return The inverse bind-pose matrix. */
	inline const FMMatrix44& GetBindPoseInverse() const { return bindPoseInverse; }

	/** Sets the inverse bind-pose matrix of the joint.
		@param inverseBindPose The inverse bind-pose matrix. */
	inline void SetBindPoseInverse(const FMMatrix44& inverseBindPose) { bindPoseInverse = inverseBindPose; }
};

/**
	A COLLADA skin controller.

	The skin controller holds the information to skin a geometric object.
	That information includes a target/base entity and its bind-pose matrix,
	a list of joints and their bind pose and the influences for the joints.

	The influences are a list, for each vertex of the target entity, of which
	joints affect the vertex and by how much.

	@ingroup FCDGeometry
*/

class FCOLLADA_EXPORT FCDSkinController : public FCDObject
{
private:
	DeclareObjectType(FCDObject);
	FCDController* parent;

	FUObjectRef<FCDEntityReference> target;
	DeclareParameter(FMMatrix44, FUParameterQualifiers::SIMPLE, bindShapeTransform, FC("Base Mesh Bind-pose Transform"));
	
	fm::vector<FCDSkinControllerJoint> joints;
	fm::vector<FCDSkinControllerVertex> influences;

public:
	/** Constructor: do not use directly.
		Instead, use the FCDController::CreateSkinController function.
		@param document The COLLADA document that owns the skin.
		@param parent The COLLADA controller that contains this skin. */
	FCDSkinController(FCDocument* document, FCDController* parent);

	/** Destructor. */
	virtual ~FCDSkinController();
	
	/** Retrieves the parent entity for the morpher.
		@return The parent controller entity. */
	inline FCDController* GetParent() { return parent; }
	inline const FCDController* GetParent() const { return parent; } /**< See above. */

	/** Retrieves the target entity.
		This entity may be a geometric entity or another controller.
		@return The target entity. */
	FCDEntity* GetTarget();
	const FCDEntity* GetTarget() const;

	/** Retrieves the Uri to the skin target.
		This can be an internal or external link
		@return The uri to the target */
	FUUri GetTargetUri() const;

	/** Sets the URI of the target mesh.
		@param uri The Uri to a local or external controller or geometry */
	void SetTargetUri(const FUUri& uri);

	/** Sets the target entity.
		This function has very important ramifications, as the number
		of vertices may change. The influences list will be modified to
		follow the number of vertices.
		This entity may be a geometric entity or another controller.
		@param _target The target entity. */
	void SetTarget(FCDEntity* _target);

	/** Retrieves the bind-pose transform of the target entity.
		@return The bind-pose transform. */
	const FMMatrix44& GetBindShapeTransform() const { return *bindShapeTransform; }

	/** Sets the bind-pose transform of the target entity.
		@param bindPose The bind-pose transform. */
	void SetBindShapeTransform(const FMMatrix44& bindPose) { bindShapeTransform = bindPose;	SetDirtyFlag(); }

	/** Retrieves the number of joints that influence the skin.
		@return The number of joints. */
	inline size_t GetJointCount() const { return joints.size(); }

	/** Sets the number of joints that influence the skin.
		@param count The number of joints that influence the skin. */
	void SetJointCount(size_t count);

	/** Retrieves the list of joints that influence the skin.
		@return The list of joints that influence the skin. */
	inline FCDSkinControllerJoint* GetJoints() { return !joints.empty() ? &(joints.front()) : NULL; }
	inline const FCDSkinControllerJoint* GetJoints() const { return !joints.empty() ? &(joints.front()) : NULL; } /**< See above. */

	/** Retrieves an indexed joint from the list of joints that influence this skin.
		@param index The index of the joint.
		@return The joint at the given index. */
	inline FCDSkinControllerJoint* GetJoint(size_t index) { FUAssert(index < joints.size(), return NULL); return &joints.at(index); }
	inline const FCDSkinControllerJoint* GetJoint(size_t index) const { FUAssert(index < joints.size(), return NULL); return &joints.at(index); } /**< See above. */

	/** Adds a joint to influence the skin.
		@param jSubId The sub-id of the scene node(s) that represent the joint.
		@param inverseBindPose The inverse bind-pose of the joint. */
	FCDSkinControllerJoint* AddJoint(const fm::string jSubId = "", const FMMatrix44& inverseBindPose = FMMatrix44::Identity);

	/** Retrieves the number of vertices with influences defined in the skin controller.
		@return The number of influenced vertices. */
	inline size_t GetInfluenceCount() const { return influences.size(); }

	/** Sets the number of vertices with influences defined in the skin controller.
		@param count The number of influences vertices. */
	void SetInfluenceCount(size_t count);

	/** Retrieves a list of the per-vertex influences for the skin.
		@return The list of per-vertex influences. */
	inline FCDSkinControllerVertex* GetVertexInfluences() { return influences.size() > 0 ? &(influences.front()) : NULL; }
	inline const FCDSkinControllerVertex* GetVertexInfluences() const { return influences.size() > 0 ? &(influences.front()) : NULL; } /**< See above. */

	/** Retrieves the per-vertex influences for a given vertex. 
		@param index The vertex index.
		@return The per-vertex influences. */
	inline FCDSkinControllerVertex* GetVertexInfluence(size_t index) { FUAssert(index < influences.size(), return NULL); return &influences.at(index); }
	inline const FCDSkinControllerVertex* GetVertexInfluence(size_t index) const { FUAssert(index < influences.size(), return NULL); return &influences.at(index); } /**< See above. */

	/** Reduces the number of joints influencing each vertex.
		1) All the influences with a weight less than the minimum will be removed.
		2) If a vertex has more influences than the given maximum, they will be sorted and the
			most important influences will be kept.
		If some of the influences for a vertex are removed, the weight will be normalized.
		@param maxInfluenceCount The maximum number of influence to keep for each vertex.
		@param minimumWeight The smallest weight to keep. */
	void ReduceInfluences(uint32 maxInfluenceCount, float minimumWeight=0.0f);
};

#endif // _FCD_SKIN_CONTROLLER_H_

