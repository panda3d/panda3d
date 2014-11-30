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
	@file FCDocument.h
	This file declares the COLLADA document object model top class: FCDocument.
*/

#ifndef _FC_DOCUMENT_H_
#define _FC_DOCUMENT_H_

#ifndef __FCD_OBJECT_H_
#include "FCDocument/FCDObject.h"
#endif // __FCD_OBJECT_H_
#ifndef _FU_PARAMETER_H_
#include "FUtils/FUParameter.h"
#endif // _FU_PARAMETER_H_

#if defined(WIN32)
template <class T> class FCOLLADA_EXPORT FCDLibrary; /**< Trick Doxygen. */
template <class T> class FCOLLADA_EXPORT FUUniqueStringMapT; /**< Trick Doxygen. */
#elif defined(LINUX) || defined(__APPLE__)
template <class T> class FCDLibrary; /**< Trick Doxygen. */
template <class T> class FUUniqueStringMapT; /**< Trick Doxygen. */
#endif // LINUX

class FCDAnimated;
class FCDAnimation;
class FCDAnimationChannel;
class FCDAnimationClip;
class FCDAsset;
class FCDCamera;
class FCDController;
class FCDEffect;
class FCDEntity;
class FCDEntityReference;
class FCDEmitter;
class FCDExternalReferenceManager;
class FCDExtra;
class FCDForceField;
class FCDGeometry;
class FCDImage;
class FCDLight;
class FCDMaterial;
class FCDObject;
class FCDPhysicsMaterial;
class FCDPhysicsModel;
class FCDPhysicsScene;
class FCDTexture;
class FCDSceneNode;
class FCDVersion;
class FUFileManager;

/**
	A layer declaration.
	Contains a name for the layer and the ids of all the entities within the layer.
*/
class FCOLLADA_EXPORT FCDLayer
{
public:
	fm::string name; /**< The layer name. There is no guarantee of uniqueness. */
	StringList objects; /**< The list of COLLADA entity ids which are contained by this layer. */
};

typedef fm::pvector<FCDLayer> FCDLayerList; /**< A dynamically-sized array of layer declarations. */

typedef FCDLibrary<FCDAnimation> FCDAnimationLibrary; /**< A COLLADA library of animation entities. */
typedef FCDLibrary<FCDAnimationClip> FCDAnimationClipLibrary; /**< A COLLADA library of animation clip entities. */
typedef FCDLibrary<FCDCamera> FCDCameraLibrary; /**< A COLLADA library of camera entities. */
typedef FCDLibrary<FCDController> FCDControllerLibrary; /**< A COLLADA library of controller entities. */
typedef FCDLibrary<FCDEffect> FCDEffectLibrary; /**< A COLLADA library of effect entities. */
typedef FCDLibrary<FCDEmitter> FCDEmitterLibrary; /**< A non-standard library of generic emitters. */
typedef	FCDLibrary<FCDForceField> FCDForceFieldLibrary; /**< A COLLADA library of force fields. */
typedef FCDLibrary<FCDGeometry> FCDGeometryLibrary; /**< A COLLADA library of geometric entities. */
typedef FCDLibrary<FCDImage> FCDImageLibrary; /**< A COLLADA library of images. */
typedef FCDLibrary<FCDLight> FCDLightLibrary; /**< A COLLADA library of light entities. */
typedef FCDLibrary<FCDMaterial> FCDMaterialLibrary; /**< A COLLADA library of visual material entities. */
typedef FCDLibrary<FCDSceneNode> FCDVisualSceneNodeLibrary; /**< A COLLADA library of visual scene nodes. */
typedef FCDLibrary<FCDPhysicsModel> FCDPhysicsModelLibrary; /**< A COLLADA library of physics model entities. */
typedef FCDLibrary<FCDPhysicsMaterial> FCDPhysicsMaterialLibrary; /**< A COLLADA library of physics material entities. */
typedef	FCDLibrary<FCDPhysicsScene> FCDPhysicsSceneLibrary; /**< A COLLADA library of physics scene nodes. */
typedef FUUniqueStringMapT<char> FUSUniqueStringMap; /**< A set of unique strings. */
typedef fm::map<FCDExtra*, FCDExtra*> FCDExtraSet; /**< A set of extra trees. */

/** @defgroup FCDocument COLLADA Document Object Model. */

/** The top class for the COLLADA object model.

	This class holds all the COLLADA libraries, the scene graphs and the
	document's asset tag.

	It also holds some global information, such as the animation start and end
	time and the layers. This global information is only exported by ColladaMaya
	right now.

	@ingroup FCDocument COLLADA Document Object Model
*/
class FCOLLADA_EXPORT FCDocument : public FCDObject
{
private:
	DeclareObjectType(FCDObject);

	FUFileManager* fileManager;
	FUObjectRef<FCDExternalReferenceManager> externalReferenceManager;
	fstring fileUrl;
	FCDVersion* version;
	FCDExtraSet extraTrees;

	FUSUniqueStringMap* uniqueNameMap;
	DeclareParameterRef(FCDEntityReference, visualSceneRoot, FC("Root Visual Scene"));
	DeclareParameterContainer(FCDEntityReference, physicsSceneRoots, FC("Root Physics Scenes"));

	// Document parameters
	DeclareParameterRef(FCDAsset, asset, FC("Asset Tag"));
	DeclareParameterRef(FCDExtra, extra, FC("Extra Tree"));
	bool hasStartTime, hasEndTime;
	DeclareParameter(float, FUParameterQualifiers::SIMPLE, startTime, FC("Start Time"));
	DeclareParameter(float, FUParameterQualifiers::SIMPLE, endTime, FC("End Time"));
	FCDLayerList layers; // Maya-only

	// Parsed and merged libraries
	DeclareParameterRef(FCDAnimationLibrary, animationLibrary, FC("Animation Library"));
	DeclareParameterRef(FCDAnimationClipLibrary, animationClipLibrary, FC("Animation Clip Library"));
	DeclareParameterRef(FCDCameraLibrary, cameraLibrary, FC("Camera Library"));
	DeclareParameterRef(FCDControllerLibrary, controllerLibrary, FC("Controller Library"));
	DeclareParameterRef(FCDEffectLibrary, effectLibrary, FC("Effect Library"));
	DeclareParameterRef(FCDForceFieldLibrary, forceFieldLibrary, FC("Force-field Library"));
	DeclareParameterRef(FCDGeometryLibrary, geometryLibrary, FC("Geometry Library"));
	DeclareParameterRef(FCDImageLibrary, imageLibrary, FC("Image Library"));
	DeclareParameterRef(FCDLightLibrary, lightLibrary, FC("Light Library"));
	DeclareParameterRef(FCDMaterialLibrary, materialLibrary, FC("Material Library"));
	DeclareParameterRef(FCDPhysicsModelLibrary, physicsModelLibrary, FC("Physics Model Library"));
	DeclareParameterRef(FCDPhysicsMaterialLibrary, physicsMaterialLibrary, FC("Physics Material Library"));
	DeclareParameterRef(FCDPhysicsSceneLibrary, physicsSceneLibrary, FC("Physics Scene Library"));
	DeclareParameterRef(FCDVisualSceneNodeLibrary, visualSceneLibrary, FC("Visual Scene Library"));
	DeclareParameterRef(FCDEmitterLibrary, emitterLibrary, FC("Emitter Library"));

	// Animated values
	typedef fm::map<FCDAnimated*, FCDAnimated*> FCDAnimatedSet;
	FCDAnimatedSet animatedValues;

public:
	/** Construct a new COLLADA document. */
	FCDocument();

	/** COLLADA document destructor. This clears out all the memory related to the document. */
	virtual ~FCDocument();

	/** Retrieves the asset information for this COLLADA document. The asset information should always be present.
		@return A pointer to the asset information structure. This pointer should never be NULL. */
	inline FCDAsset* GetAsset() { return asset; }
	inline const FCDAsset* GetAsset() const { return asset; } /**< See above. */

	/** Retrieves the base extra tree for this COLLADA document. An extra tree should always be present,
		but is likely to be empty.
		@return A pointer to the base extra tree. This pointer should never be NULL. */
	inline FCDExtra* GetExtra() { return extra; }
	inline const FCDExtra* GetExtra() const { return extra; } /**< See above. */

	/** Retrieves the version numbers for this COLLADA document. The version numbers should always be present.
		@return The version number structure.*/
	inline FCDVersion& GetVersion() { return *version; }
	inline const FCDVersion& GetVersion() const { return *version; } /**< See above. */

	/** [INTERNAL] Retrieves the local file manager for the COLLADA document. Used to resolve URIs and transform file
		paths into their relative or absolute equivalent. May be deprecated in future versions.
		@return The file manager for this COLLADA document. This pointer should never be NULL. */
	inline FUFileManager* GetFileManager() { return fileManager; }
	inline const FUFileManager* GetFileManager() const { return fileManager; } /**< See above. */

	/** Retrieves the currently instanced visual scene.
		NOTE: GetVisualSceneRoot is deprecated. Please start using GetVisualSceneInstance.
		@return The currently instanced visual scene. */
	inline FCDSceneNode* GetVisualSceneInstance() { return const_cast<FCDSceneNode*>(const_cast<const FCDocument*>(this)->GetVisualSceneInstance()); }
	const FCDSceneNode* GetVisualSceneInstance() const; /**< See above. */
	DEPRECATED(3.04A, FCDocument::GetVisualSceneInstance) inline FCDSceneNode* GetVisualSceneRoot() { return GetVisualSceneInstance(); } /**< See above. */
	DEPRECATED(3.04A, FCDocument::GetVisualSceneInstance) inline const FCDSceneNode* GetVisualSceneRoot() const { return GetVisualSceneInstance(); } /**< See above. */

	/** Retrieves the reference to the currently instanced visual scene.
		@return The instanced visual scene reference. */
	inline FCDEntityReference* GetVisualSceneInstanceReference() { return visualSceneRoot; }
	inline const FCDEntityReference* GetVisualSceneInstanceReference() const { return visualSceneRoot; } /**< See above. */

	/** Retrieves the number of instanced physics scenes.
		@return The number of instanced physics scenes. */
	inline size_t GetPhysicsSceneInstanceCount() const { return physicsSceneRoots.size(); }

	/** Retrieves one instanced physics scene.
		@param index The index of the physics scene to retrieve.
			If the index is out-of-bounds, NULL is returned.
		@return A currently instanced physics scene. */
	inline FCDPhysicsScene* GetPhysicsSceneInstance(size_t index = 0) { return const_cast<FCDPhysicsScene*>(const_cast<const FCDocument*>(this)->GetPhysicsSceneInstance(index)); }
	const FCDPhysicsScene* GetPhysicsSceneInstance(size_t index = 0) const; /**< See above. */
	DEPRECATED(3.04A, FCDocument::GetPhysicsSceneInstance) inline FCDPhysicsScene* GetPhysicsSceneRoot(size_t index = 0) { return GetPhysicsSceneInstance(index); } /**< See above. */
	DEPRECATED(3.04A, FCDocument::GetPhysicsSceneInstance) inline const FCDPhysicsScene* GetPhysicsSceneRoot(size_t index = 0) const { return GetPhysicsSceneInstance(index); } /**< See above. */

	/** Adds one instanced physics scene to the document.
		@param scene The newly instanced physics scene. */
	void AddPhysicsSceneInstance(FCDPhysicsScene* scene);

	/** Retrieves a reference to the instanced physics scene.
		@param index The index of the physics scene instance reference to
			retrieve. If the index is out-of-bounds, NULL is returned.
		@return The reference to the index physics scene reference. */
	inline FCDEntityReference* GetPhysicsSceneInstanceReference(size_t index = 0) { if (index == 0 && physicsSceneRoots.empty()) return NULL; FUAssert(index < physicsSceneRoots.size(), return NULL); return physicsSceneRoots[index]; }
	inline const FCDEntityReference* GetPhysicsSceneInstanceReference(size_t index = 0) const { if (index == 0 && physicsSceneRoots.empty()) return NULL; FUAssert(index < physicsSceneRoots.size(), return NULL); return physicsSceneRoots[index]; }

	/** Adds an empty reference in the list of instanced physics scenes.
		@return The new, empty, reference. */
	FCDEntityReference* AddPhysicsSceneInstanceReference();

	/** [INTERNAL] Retrieves the map of unique ids for this document.
		@return The map of unique ids for this document. */
	inline FUSUniqueStringMap* GetUniqueNameMap() { return uniqueNameMap; }
	inline const FUSUniqueStringMap* GetUniqueNameMap() const { return uniqueNameMap; } /**< See above. */

	/** Retrieves the external reference manager.
		@return The external reference manager. */
	inline FCDExternalReferenceManager* GetExternalReferenceManager() { return externalReferenceManager; }
	inline const FCDExternalReferenceManager* GetExternalReferenceManager() const { return externalReferenceManager; } /**< See above. */

	/** Retrieves the file URL for this document.
		@return The file URL for the document. */
	const fstring& GetFileUrl() const { return fileUrl; }

	/** Sets the file URL for this document.
		Useful when working with external references.
		@param filename The filename for the document. */
	void SetFileUrl(const fstring& filename);

	/** Returns whether a start time is being enforced for the document.
		@return Whether the document has a start time. */
	inline bool HasStartTime() const { return hasStartTime; }
	/** Retrieves the start time set for the document.
		@return The document start time. */
	inline float GetStartTime() const { return startTime; }
	/** Enforces a certain time as the start time for the document.
		@param time The document start time. */
	inline void SetStartTime(float time) { startTime = time; hasStartTime = true; }

	/** Returns whether a end time is being enforced for the document.
		@return Whether the document has a end time. */
	inline bool HasEndTime() const { return hasEndTime; }
	/** Retrieves the end time set for the document.
		@return The document end time. */
	inline float GetEndTime() const { return endTime; }
	/** Enforces a certain time as the end time for the document.
		@param time The document end time. */
	inline void SetEndTime(float time) { endTime = time; hasEndTime = true; }

	/** Evaluate the animation objects at the given time
		@param time The time to evaluate the objects at */
	inline void SetCurrentTime(float time);

	/** Retrieves the list of entity layers.
		@return The list of entity layers. */
	inline FCDLayerList& GetLayers() { return layers; }
	inline const FCDLayerList& GetLayers() const { return layers; } /**< See above. */

	/** Retrieves the number of entity layers contained within the document.
		@return The number of layers. */
	inline size_t GetLayerCount() const { return layers.size(); }

	/** Retrieves a specific entity layer contained within the document.
		@param index The index of the layer.
		@return The entity layer. This pointer will be NULL if the index
			is out-of-bounds. */
	inline FCDLayer* GetLayer(size_t index) { FUAssert(index < GetLayerCount(), return NULL); return layers.at(index); }
	inline const FCDLayer* GetLayer(size_t index) const { FUAssert(index < GetLayerCount(), return NULL); return layers.at(index); } /**< See above. */

	/** Adds an entity layer to the document.
		@return The new layer. */
	FCDLayer* AddLayer();

	/** Releases an entity layer from the document
		@param layer The layer to release. */
	void ReleaseLayer(FCDLayer* layer);

	/** Retrieves the animation library. The animation library contains the animation curves
		within a tree structure. To create and find animation curves, do not use the animation
		library directly: use the FCDAnimated class, the FindAnimatedValue() function and the
		RegisterAnimatedValue() function.
		@return The animation library. */
	inline FCDAnimationLibrary* GetAnimationLibrary() { return animationLibrary; }
	inline const FCDAnimationLibrary* GetAnimationLibrary() const { return animationLibrary; } /**< See above. */

	/** Retrieves the animation clip library. The animation clip library contains a list of animation clips.
		Each animation clip instantiates nodes from the animation library. Sections of the animation curves
		belonging to the instantiated animation nodes are thereby packaged together as animation clips.
		@return The animation clip library. */
	inline FCDAnimationClipLibrary* GetAnimationClipLibrary() { return animationClipLibrary; }
	inline const FCDAnimationClipLibrary* GetAnimationClipLibrary() const { return animationClipLibrary; } /**< See above. */

	/** Retrieves the camera library. The camera library contains a list of cameras, which may be
		instantiated within the scene graph. COLLADA supports two camera types: perspective and orthographic.
		@return The camera library. */
	inline FCDCameraLibrary* GetCameraLibrary() { return cameraLibrary; }
	inline const FCDCameraLibrary* GetCameraLibrary() const { return cameraLibrary; } /**< See above. */

	/** Retrieves the controller library. The controller library contains a list of controllers, which may
		be instantiated within the scene graph. COLLADA supports two controller types: skin and morph.
		@return The controller library. */
	inline FCDControllerLibrary* GetControllerLibrary() { return controllerLibrary; }
	inline const FCDControllerLibrary* GetControllerLibrary() const { return controllerLibrary; } /**< See above. */

	/** Retrieves the geometry library. The geometry library contains a list of basic geometries, which may
		be instantiated within the scene graph and may be used by controllers.
		COLLADA supports two geometry types: mesh and spline.
		@return The geometry library. */
	inline FCDGeometryLibrary* GetGeometryLibrary() { return geometryLibrary; }
	inline const FCDGeometryLibrary* GetGeometryLibrary() const { return geometryLibrary; } /**< See above. */

	/** Retrieves the physics force field library.
		Force fields are emitters of physical force and have no COMMON profile in COLLADA.
		@return The force field library. */
	inline FCDForceFieldLibrary* GetForceFieldLibrary() { return forceFieldLibrary; }
	inline const FCDForceFieldLibrary* GetForceFieldLibrary() const { return forceFieldLibrary; } /**< See above. */

	/** Retrieves the image library. The image library contains a list of images. Images are used
		by effects for textures.
		@return The image library. */
	inline FCDImageLibrary* GetImageLibrary() { return imageLibrary; }
	inline const FCDImageLibrary* GetImageLibrary() const { return imageLibrary; } /**< See above. */

	/** Retrieves the light library. The light library contains a list of light, which may be
		instantiated within the scene graph. COLLADA supports four light types: ambient, directional,
		point and spot lights.
		@return The light library. */
	inline FCDLightLibrary* GetLightLibrary() { return lightLibrary; }
	inline const FCDLightLibrary* GetLightLibrary() const { return lightLibrary; } /**< See above. */

	/** Retrieves the visual material library. The visual material library contains a list of visual materials,
		which are bound to mesh polygons within the scene graph. A visual material instantiates an effect and
		presets the effect parameters for a given visual result.
		@return The visual material library. */
	inline FCDMaterialLibrary* GetMaterialLibrary() { return materialLibrary; }
	inline const FCDMaterialLibrary* GetMaterialLibrary() const { return materialLibrary; } /**< See above. */

	/** Retrieves the effect library. The effect library contains a list of effects, which may be instantiated
		by materials. An effect defines an interface for a rendering shader. A ColladaFX effect may contain multiple
		passes and techniques for different platforms or level of details.
		@return The effect library. */
	inline FCDEffectLibrary* GetEffectLibrary() { return effectLibrary; } 
	inline const FCDEffectLibrary* GetEffectLibrary() const { return effectLibrary; } /**< See above. */

	/** Retrieves the visual scene library. The visual scene library contains an acyclic directed graph of
		visual scene nodes: a visual scene node contains one or more parent nodes and zero or more child nodes.
		A visual scene node also contains 3D transformations: translation, rotation, scale, skew, as well as
		the compound transformations: lookAt and matrix. A visual scene node also contains instances of
		geometries, controllers, cameras and/or lights. Only one visual scene should be used at one time
		by the global scene.
		@return The visual scene library. */
	inline FCDVisualSceneNodeLibrary* GetVisualSceneLibrary() { return visualSceneLibrary; }
	inline const FCDVisualSceneNodeLibrary* GetVisualSceneLibrary() const { return visualSceneLibrary; } /**< See above. */

	/** Retrieves the physics model library.
		The physics model library contains a list of physics models.
		@return The physics model library. */
	inline FCDPhysicsModelLibrary* GetPhysicsModelLibrary() { return physicsModelLibrary; }
	inline const FCDPhysicsModelLibrary* GetPhysicsModelLibrary() const { return physicsModelLibrary; } /**< See above. */

	/** Retrieves the physics material library.
		The physics material library contains a list of physics material.
		@return The physics material library. */
	inline FCDPhysicsMaterialLibrary* GetPhysicsMaterialLibrary() { return physicsMaterialLibrary; }
	inline const FCDPhysicsMaterialLibrary* GetPhysicsMaterialLibrary() const { return physicsMaterialLibrary; } /**< See above. */

	/** Retrieves the physics scene library.
		The physics scene library contains a list of physics scene nodes.
		@return The physics scene library. */
	inline FCDPhysicsSceneLibrary* GetPhysicsSceneLibrary() { return physicsSceneLibrary; }
	inline const FCDPhysicsSceneLibrary* GetPhysicsSceneLibrary() const { return physicsSceneLibrary; } /**< See above. */

	/** Retrieves the emitter library.
		The emitter library contains a list of emitter definitions.
		@return The emitter library. */
	inline FCDEmitterLibrary* GetEmitterLibrary() { return emitterLibrary; }
	inline const FCDEmitterLibrary* GetEmitterLibrary() const { return emitterLibrary; } /**< See above. */

	/** Insert a new visual scene within the visual scene library.
		The new visual scene will be used as the root visual scene.
		@return The newly created visual scene. */
	FCDSceneNode* AddVisualScene();

	/** Insert a new physics scene within the physics material library.
		The new physics scene will be used as the root physics scene.
		@return The newly created physics scene. */
	FCDPhysicsScene* AddPhysicsScene();
	
	/** Retrieves the animation tree node that matches the given COLLADA id.
		@param daeId A valid COLLADA id.
		@return The animation tree node. This pointer will be NULL if
			no matching animation tree node was found. */
	FCDAnimation* FindAnimation(const fm::string& daeId);

	/** Retrieves the animation clip that matches the given COLLADA id.
		@param daeId A valid COLLADA id.
		@return The animation clip. This pointer will be NULL if
			no matching animation clip was found. */
	FCDAnimationClip* FindAnimationClip(const fm::string& daeId);

	/** Retrieves the camera that matches the given COLLADA id.
		@param daeId A valid COLLADA id.
		@return The camera. This pointer will be NULL if no matching camera was found. */
	FCDCamera* FindCamera(const fm::string& daeId);

	/** Retrieves the controller that matches the given COLLADA id.
		@param daeId A valid COLLADA id.
		@return The controller. This pointer will be NULL if no matching controller was found. */
	FCDController* FindController(const fm::string& daeId);

	/** Retrieves the effect that matches the given COLLADA id.
		@param daeId A valid COLLADA id.
		@return The effect. This pointer will be NULL if no matching effect was found. */
	FCDEffect* FindEffect(const fm::string& daeId);

	/** Retrieves the entity that matches the given COLLADA id.
		This function will look through all the libraries for any entity
		with the given COLLADA id.
		@param daeId A valid COLLADA id.
		@return The entity. This pointer will be NULL if no matching entity was found. */
	FCDEntity* FindEntity(const fm::string& daeId);

	/** Retrieves the emitter that matches the given COLLADA id. 
		@param daeId A valid COLLADA id.
		@return The emitter. This pointer will be NULL if no matching emitter was found. */
	FCDEmitter* FindEmitter(const fm::string& daeId);

	/** Retrieves the force field that matches the given COLLADA id. 
		@param daeId A valid COLLADA id.
		@return The force field. This pointer will be NULL if no matching force field was found. */
	FCDForceField* FindForceField(const fm::string& daeId);

	/** Retrieves the geometry that matches the given COLLADA id.
		@param daeId A valid COLLADA id.
		@return The geometry. This pointer will be NULL if no matching geometry was found. */
	FCDGeometry* FindGeometry(const fm::string& daeId);

	/** Retrieves the image that matches the given COLLADA id.
		@param daeId A valid COLLADA id.
		@return The image. This pointer will be NULL if no matching image was found. */
	FCDImage* FindImage(const fm::string& daeId);

	/** Retrieves the layer that matches the given name.
		Note that there are no checks for uniqueness in layer names.
		@param name A layer name.
		@return The layer. This pointer will be NULL if no matching layer was found. */
	FCDLayer* FindLayer(const fm::string& name);

	/** Retrieves the light that matches the given COLLADA id.
		@param daeId A valid COLLADA id.
		@return The light. This pointer will be NULL if no matching light was found. */
	FCDLight* FindLight(const fm::string& daeId);

	/** Retrieves the visual material that matches the given COLLADA id.
		@param daeId A valid COLLADA id.
		@return The visual material. This pointer will be NULL if no matching visual material was found. */
	FCDMaterial* FindMaterial(const fm::string& daeId);

	/** Retrieves the visual scene that matches the given COLLADA id.
		@param daeId A valid COLLADA id.
		@return The visual scene. This pointer will be NULL if no matching visual scene was found. */
	FCDSceneNode* FindVisualScene(const fm::string& daeId);

	/** Retrieves the physics scene that matches the given COLLADA id.
		@param daeId A valid COLLADA id.
		@return The physics scene. This pointer will be NULL if no matching physics scene was found. */
	FCDPhysicsScene* FindPhysicsScene(const fm::string& daeId);

	/** Retrieves the physics material that matches the given COLLADA id.
		@param daeId A valid COLLADA id.
		@return The physics material. This pointer will be NULL if no matching physics material was found. */
	FCDPhysicsMaterial* FindPhysicsMaterial(const fm::string& daeId);

	/** Retrieves the physics model that matches the given COLLADA id.
		@param daeId A valid COLLADA id.
		@return The physics model. This pointer will be NULL if no matching physics model was found. */
	FCDPhysicsModel* FindPhysicsModel(const fm::string& daeId);

	/** Retrieves the visual scene node that matches the given COLLADA id. 
		This method searches through all the visual scenes within the visual scene library and
		their child visual scene nodes to find the correct visual scene node.
		@param daeId A valid COLLADA id.
		@return The visual scene node. This pointer will be NULL if no matching visual scene node was found. */
	const FCDSceneNode* FindSceneNode(const char* daeId) const;
	inline FCDSceneNode* FindSceneNode(const char* daeId) { return const_cast<FCDSceneNode*>(const_cast<const FCDocument*>(this)->FindSceneNode(daeId)); }

	/** [INTERNAL] Registers an animated value with the document. All animated values are
		listed within the document.
		@param animated The new animated value to list within the document. */
	void RegisterAnimatedValue(FCDAnimated* animated);

	/** [INTERNAL] Unregisters an animated value of the document. All animated values are
		listed within the document. This function must be called before deleting an animated value.
		@param animated The animated value to un-list from the document. */
	void UnregisterAnimatedValue(FCDAnimated* animated);

	/** [INTERNAL] Registers an extra tree with the document.
		All extra trees are listed within the document to support extra-technique plug-ins.
		@param tree The new extra tree to list within the document. */
	inline void RegisterExtraTree(FCDExtra* tree) { extraTrees.insert(tree, tree); }

	/** [INTERNAL] Unregisters an extra tree of the document.
		All extra trees are listed within the document to support extra-technique plug-ins.
		@param tree The extra tree to un-list from the document. */
	inline void UnregisterExtraTree(FCDExtra* tree) { FUAssert(extraTrees.find(tree) != extraTrees.end(), return); extraTrees.erase(tree); }

	/** [INTERNAL] Retrieves the set of extra trees.
		This function is meant only to be used for supporting the extra-technique plug-ins.
		@return The set of extra trees for this document. */
	inline FCDExtraSet& GetExtraTrees() { return extraTrees; }
};

#endif //_FC_DOCUMENT_H_
