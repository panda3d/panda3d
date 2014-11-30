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
	@file FCDMorphController.h
	This file contains the FCDMorphController and the FCDMorphTarget classes.
*/

#ifndef _FCD_MORPH_CONTROLLER_H_
#define _FCD_MORPH_CONTROLLER_H_

#ifndef _FU_DAE_ENUM_H_
#include "FUtils/FUDaeEnum.h"
#endif // _FU_DAE_ENUM_H_
#ifndef __FCD_OBJECT_H_
#include "FCDocument/FCDObject.h"
#endif // __FCD_OBJECT_H_
#ifndef _FCD_PARAMETER_ANIMATABLE_H_
#include "FCDocument/FCDParameterAnimatable.h"
#endif // _FCD_PARAMETER_ANIMATABLE_H_

class FCDocument;
class FCDAnimated;
class FCDController;
class FCDGeometry;
class FCDMorphController;

/**
	A COLLADA morph target.
	A morph target is just a geometric entity with a weight assigned.
	The weight may be animated.
*/
class FCOLLADA_EXPORT FCDMorphTarget : public FCDObject
{
private:
	DeclareObjectType(FCDObject);
	FCDMorphController* parent;
	DeclareParameterPtr(FCDGeometry, geometry, FC("Mesh"))
	DeclareParameterAnimatable(float, FUParameterQualifiers::SIMPLE, weight, FC("Weight"));

public:
	/** Constructor: do not use directly.
		Instead, use the FCDMorphController::AddTarget function.
		@param document The COLLADA document that owns the morph target.
		@param parent The morph controller that contains the morph target. */
	FCDMorphTarget(FCDocument* document, FCDMorphController* parent);

	/** Destructor. */
	virtual ~FCDMorphTarget();

	/** Retrieves the morph controller which contains this target.
		@return The parent morpher. */
	FCDMorphController* GetParent() { return parent; }
	const FCDMorphController* GetParent() const { return parent; } /**< See above. */

	/** Retrieves the target geometry.
		This is what the morphed geometry should look like if
		the morphing weight is set to 1.0f.
		@return The target geometry. */
	FCDGeometry* GetGeometry() { return geometry; }
	const FCDGeometry* GetGeometry() const { return geometry; } /**< See above. */

	/** Sets the target geometry.
		This is what the morphed geometry should look like if
		the morphing weight is set to 1.0f. As such, the target geometry
		should be similar to the base target geometry. You can verify
		this using the FCDMorphController::IsSimilar function.
		@param geometry The target geometry. */
	void SetGeometry(FCDGeometry* geometry);

	/** Retrieves the morphing weight.
		@return The morphing weight. */
	FCDParameterAnimatableFloat& GetWeight() { return weight; }
	const FCDParameterAnimatableFloat& GetWeight() const { return weight; } /**< See above. */

	/** Sets the morphing weight.
		This function has no impact on any animations that target the morphing weight.
		@param _weight The morphing weight. */
	void SetWeight(float _weight) { weight = _weight; SetDirtyFlag(); }

	/** Retrieves whether the morphing weight is animated.
		@return Whether the morphing weight is animated. */
	DEPRECATED(3.05A, GetWeight().IsAnimated()) bool IsAnimated() const;

	/** Retrieves the animation associated with the morphing weight.
		@return The animated value associated with the morphing weight.
			This pointer will be NULL if the morphing weight is not animated. */
	DEPRECATED(3.05A, GetWeight().GetAnimated()) FCDAnimated* GetAnimatedWeight(); 
	DEPRECATED(3.05A, GetWeight().GetAnimated()) const FCDAnimated* GetAnimatedWeight() const; /**< See above. */
};

/**
	A COLLADA morpher.

	A morpher holds a base geometry and a set of morph targets
	that contains a geometry and a weight. The geometry must be similar to the
	base geometry and the weights are used to interpolate the vertex positions
	of the base geometry. To be similar, two meshes must have the same number of
	vertices and two splines must have the same number of control points.
	The morphing weights can be animated.
	
	There are two interpolation functions defined in COLLADA.
	See the FUDaeMorphMethod::Method enumerated type for more information.

	@see FCDMorphTarget FUDaeMorphMethod
	@ingroup FCDGeometry
*/
class FCOLLADA_EXPORT FCDMorphController : public FCDObject
{
private:
	DeclareObjectType(FCDObject);
	FCDController* parent;

	DeclareParameter(uint32, FUParameterQualifiers::SIMPLE, method, FC("Method")); // FUDaeMorphMethod::Method
	DeclareParameterPtr(FCDEntity, baseTarget, FC("Base Mesh"));
	DeclareParameterContainer(FCDMorphTarget, morphTargets, FC("Morph Targets"));

public:
	/** Constructor: do not use directly. 
		Instead, use the FCDController::CreateMorphController function.
		@param document The COLLADA document that owns the morpher.
		@param parent The COLLADA controller that contains this morpher. */
	FCDMorphController(FCDocument* document, FCDController* parent);

	/** Destructor. */
	virtual ~FCDMorphController();

	/** Retrieves the parent entity for the morpher.
		@return The parent controller entity. */
	FCDController* GetParent() { return parent; }
	const FCDController* GetParent() const { return parent; } /**< See above. */

	/** Retrieves the base entity controlled by this morpher.
		This entity may be a geometry or another controller.
		@return The base target. */
	FCDEntity* GetBaseTarget() { return baseTarget; }
	const FCDEntity* GetBaseTarget() const { return baseTarget; } /**< See above. */

	/** Sets the base entity controlled by this morpher.
		This entity may be a geometry or another controller.
		Since the morph targets must be similar to this entity,
		all the morph targets that are not similar to the new base entity will be removed.
		@param entity The new base entity. */
	void SetBaseTarget(FCDEntity* entity);

	/** Retrieves the list of the morph targets.
		All the morph target geometries should be similar to the base entity.
		@return The morph targets. */
	DEPRECATED(3.05A, GetTargetCount and GetTarget(index)) void GetTargets() const {}

	/** Retrieves the number of morph targets.
		@return The number of morph targets. */
	size_t GetTargetCount() const { return morphTargets.size(); }

	/** Retrieves a specific morph target.
		@param index The index of the morph target.
		@return The morph target. This pointer will be NULL if the index is out-of-bounds. */
	FCDMorphTarget* GetTarget(size_t index) { FUAssert(index < GetTargetCount(), return NULL); return morphTargets.at(index); }
	const FCDMorphTarget* GetTarget(size_t index) const { FUAssert(index < GetTargetCount(), return NULL); return morphTargets.at(index); } /**< See above. */

	/** Adds a new morph target.
		@param geometry The morph target geometry.
		@param weight The morphing weight.
		@return The new morph target. */
	FCDMorphTarget* AddTarget(FCDGeometry* geometry = NULL, float weight = 0.0f);

	/** Retrieves the method used to interpolate between the different morph targets.
		@return The interpolation method. */
	FUDaeMorphMethod::Method GetMethod() const { return (FUDaeMorphMethod::Method) *method; }

	/** Sets the method used to interpolate between the different morph targets.
		@param _method The interpolation method. */
	void SetMethod(FUDaeMorphMethod::Method _method) { method = _method; }

	/** Retrieves whether a given entity is similar to the base target.
		Entities must be similar to be able to morph between them.
		@param entity An entity.
		@return Whether the given entity is similar to the base target. */
	bool IsSimilar(FCDEntity* entity);
};

#endif // _FCD_MORPH_CONTROLLER_H_

