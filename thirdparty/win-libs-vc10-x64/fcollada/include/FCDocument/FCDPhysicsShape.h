/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

/**
	@file FCDPhysicsShape.h
	This file contains the FCDPhysicsShape class.
*/

#ifndef _FCD_PHYSICS_SHAPE_H_
#define _FCD_PHYSICS_SHAPE_H_

#ifndef __FCD_OBJECT_H_
#include "FCDocument/FCDObject.h"
#endif // __FCD_OBJECT_H_
#ifndef _FCD_PHYSICS_ANALYTICAL_GEOM_H_
#include "FCDocument/FCDPhysicsAnalyticalGeometry.h"
#endif // _FCD_PHYSICS_ANALYTICAL_GEOM_H_
#ifndef _FCD_TRANSFORM_H_
#include "FCDocument/FCDTransform.h" /** @todo Remove this include by moving the FCDTransform::Type enum to FUDaeEnum.h. */
#endif // _FCD_TRANSFORM_H_
#ifndef _FU_DAE_ENUM_H_
#include "FUtils/FUDaeEnum.h"
#endif // _FU_DAE_ENUM_H_

class FCDEntityInstance;
class FCDGeometry;
class FCDGeometryInstance;
class FCDPhysicsAnalyticalGeometry;
class FCDPhysicsMaterial;

typedef FUObjectContainer<FCDTransform> FCDTransformContainer; /**< A dynamically-sized containment array for transforms. */

/**
	A COLLADA shape object.

	A shape describes the boundary used for collision detection within a rigid
	body. It belongs to a single rigid body and cannot be referenced from other
	elements. 

	@ingroup FCDocument
*/
class FCOLLADA_EXPORT FCDPhysicsShape : public FCDObject
{
private:
	DeclareObjectType(FCDObject);

	bool hollow;
	FUTrackedPtr<FCDPhysicsMaterial> physicsMaterial;
	bool ownsPhysicsMaterial;
	
	//one of these two will define the rigid body
	FUTrackedPtr<FCDGeometryInstance> geometry;
	FUObjectRef<FCDPhysicsAnalyticalGeometry> analGeom;

	float* mass;
	float* density;
	FCDTransformContainer transforms;
	FCDEntityInstance* instanceMaterialRef;

	bool isDensityMoreAccurate;

public:
	/** Constructor: do not use directly. Create new shapes using the 
		FCDPhysicsRigidBody::AddPhysicsShape function.
		@param document The COLLADA document that contains this rigid body. */
	FCDPhysicsShape(FCDocument* document);

	/** Destructor. */
	virtual ~FCDPhysicsShape();

	/** Retrieves the mass of this shape. 
		@return The mass. */
	float GetMass() const;
	const float* GetMassPointer() const { return mass; } /** [INTERNAL] */

	/** Sets the mass of this shape.
		@param mass The new mass. */
	void SetMass(float mass);

	/** Retrieves whether density of this shape is more accurate than mass. It is more accurate if mass is not defined. In that case mass is
		calculated using approximations when it is non-analytic.
		@return True if density is more accurate. */
	bool IsDensityMoreAccurate() const { return isDensityMoreAccurate; }

	/** [INTERNAL] Set 'DensityMoreAccurate'.
		@parame value Value to be set.
	*/
	void SetDensityMoreAccurate(bool value){ isDensityMoreAccurate = value; }

	/** Retrieves the density of this shape.
		@return The density. */
	float GetDensity() const;
	const float* GetDensityPointer() const { return density; } /** [INTERNAL] */

	/** Sets the density of this shape.
		@param density The new density. */
	void SetDensity(float density);

	/** Retrives whether this shape is hollow.
		@return True if this shape is hollow. */
	bool IsHollow() const {return hollow;}

	/** Sets whether this shape is hollow.
		@param _hollow True if this shape is hollow. */
	void SetHollow(bool _hollow) {hollow = _hollow;}

	/** Retrieves whether this shape is an analyical geometry such as a box,
		plane, sphere, cylinder, tapered cylinder, capsule, or tapered capsule.
		@return True if this shape is an analytical geometry. */
	bool IsAnalyticalGeometry() const { return analGeom != NULL; }

	/** Gets the analytic geometry. NULL is returned if IsAnalyticalGeometry
		returns false.
		@return The analytical geometry. */
	FCDPhysicsAnalyticalGeometry* GetAnalyticalGeometry() { return analGeom; }
	const FCDPhysicsAnalyticalGeometry* GetAnalyticalGeometry() const { return analGeom; } /**< See above. */

	/** [INTERNAL] Set the analytical geometry.
		@param aGeom The analytical geometry.
	*/
	void SetAnalyticalGeometry(FCDPhysicsAnalyticalGeometry* aGeom){ analGeom = aGeom; }

	/** Retrieves whether this shape is a geometry instance such as from mesh,
		convex mesh, or spline.
		@returns True if this shape is a geometry instance. */
	bool IsGeometryInstance() const { return geometry != NULL; }

	/** Gets the geometry instance. NULL is returned if IsGeometryInstance
		returns false.
		@return The geometry instance. */
	FCDGeometryInstance* GetGeometryInstance() { return geometry; }
	const FCDGeometryInstance* GetGeometryInstance() const { return geometry; } /**< See above. */

	/** [INTERNAL] Set the geometry instance.
		@param instance The new geometry instance.
	*/
	void SetGeometryInstance(FCDGeometryInstance* instance){ geometry = instance; } 

	/** Creates a geometry instance for this shape. If IsAnalyticalGeometry 
		returned true before this call, it will return false afterwards as the
		analytical geometry is released.
		@param geom The geometry to instance.
		@param createConvexMesh True if want to create a convex mesh from geom
		@return The new geometry instance. */
	FCDGeometryInstance* CreateGeometryInstance(FCDGeometry* geom, bool createConvexMesh=false);

	/** Creates an analytical geometry for this shape. If IsGeometryInstance
		returned true before this call, it will return false afterwards as the
		geometry instance is released.
		@param type The type of analytical geometry to create.
		@return The new analytical geometry. */
	FCDPhysicsAnalyticalGeometry* CreateAnalyticalGeometry(FCDPhysicsAnalyticalGeometry::GeomType type);

	/** Retrieves the transforms for this shape.
		@return The transforms. */
	FCDTransformContainer& GetTransforms() {return transforms;}
	const FCDTransformContainer& GetTransforms() const {return transforms;} /**< See above. */

	/** Adds a transform for this shape.
		@param type The type of transform.
		@param index The position in the transform list to add it to; an index 
			greater than the size indicates adding the transform to the end. 
		@return The new transform. */
	FCDTransform* AddTransform(FCDTransform::Type type, size_t index = (size_t)-1);

	/** Retrieves the physics material for this shape.
		@return The physics material for this shape. */
	FCDPhysicsMaterial* GetPhysicsMaterial() {return physicsMaterial;}
	const FCDPhysicsMaterial* GetPhysicsMaterial() const {return physicsMaterial;} /**< See above. */

	/** Sets the physics material for this shape.
		@param physicsMaterial The physics material for this shape. */
	void SetPhysicsMaterial(FCDPhysicsMaterial* physicsMaterial);

	/** Adds a physics material for this shape. This shape is responsible for
		releasing the physics material. 
		@return The new physics material. */
	FCDPhysicsMaterial* AddOwnPhysicsMaterial();

	/** Copies the shape into a clone.
		@param clone The empty clone. If this pointer is NULL, a new physics 
			shape will be created and you will need to release the returned 
			pointer manually.
		@return The clone. */
	FCDPhysicsShape* Clone(FCDPhysicsShape* clone = NULL) const;

	/** Calculates the volume of the shape. Currently it is calculating a 
		simple bounding box volume if IsGeometryInstance it true, and it is 
		calculating the exact volume if IsAnalyticalGeometry is true. Note that
		a volume of 1.0f is returned if it is a spline or plane.
		@return The volume of the shape. */
	float CalculateVolume() const;

	/** [INTERNAL] Set the material instance.
		@instance The new material instance.
	*/
	void SetInstanceMaterial(FCDEntityInstance* instance){ instanceMaterialRef = instance; }

	/** [INTERNAL] Retrieve the material instance.
		@return The material instance.
	*/
	FCDEntityInstance* GetInstanceMaterial(){ return instanceMaterialRef; }

	/** [INTERNAL] Determin if it owns the material.
		@return True if this owns the material, false otherwise.
	*/
	bool OwnsPhysicsMaterial() { return ownsPhysicsMaterial; }


};

#endif // _FCD_PHYSICS_SHAPE_H_
