/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

/**
	@file FCDPhysicsAnalyticalGeometry.h
	This file contains all the analytical geometry classes including Box, 
	Plane, Sphere, Cylinder, Capsule, Tapered Cylinder, and Tapered Capsule.
*/

#ifndef _FCD_PHYSICS_ANALYTICAL_GEOM_H_
#define _FCD_PHYSICS_ANALYTICAL_GEOM_H_

#ifndef _FCD_ENTITY_H_
#include "FCDocument/FCDEntity.h"
#endif // _FCD_ENTITY_H_

class FCDocument;
class FCDPhysicsShape;

/**
	A COLLADA physics analytical geometry.

	@ingroup FCDocument
*/
class FCOLLADA_EXPORT FCDPhysicsAnalyticalGeometry : public FCDEntity
{
private:
	DeclareObjectType(FCDEntity);

public:
	/** The geometry type of the analytical geometry class.
		Used this information to up-cast an entity instance. */
	enum GeomType { 
		BOX, /**< A box. */
		PLANE, /**< An infinate plane. */
		SPHERE, /**< A sphere. */
		CYLINDER, /**< A cylinder. */
		CAPSULE, /**< A cylinder with spheres at the end. */
		TAPERED_CYLINDER, /**< A cylinder with different sized flat faces. */
		TAPERED_CAPSULE /**< A capsule with different sized spheres. */
	};

	/** Constructor: do not use directly. Create new analytical geometries by 
		using FCDPhysicsShape::CreateAnalyticalGeometry function.
		@param document The COLLADA document that contains this physics scene.
	*/
	FCDPhysicsAnalyticalGeometry(FCDocument* document);

	/** Destructor. */
	virtual ~FCDPhysicsAnalyticalGeometry();

	/** Retrieves the entity type for this class. This function is part of the 
		FCDEntity interface.
		@return The entity type: PHYSICS_ANALYTICAL_GEOMETRY. */
	virtual Type GetType() const {return PHYSICS_ANALYTICAL_GEOMETRY;}

	/** Retrieves the analytical geometry type for this class. 
		@return The analytical geometry type. */
	virtual GeomType GetGeomType() const = 0;

	/** Calculates the volume of this analytical geometry.
		@return The volume. */
	virtual float CalculateVolume() const = 0;

	/** Copies the analytical geometry into a clone.
		@param clone The empty clone. If this pointer is NULL, a analytical
			geometry will be created and you will need to release the returned 
			pointer manually.
		@param cloneChildren Whether to recursively clone this entity's 
			children.
		@return The clone. */
	virtual FCDEntity* Clone(FCDEntity* clone = NULL, bool cloneChildren = false) const;
};

/**
	A COLLADA physics box.

	@ingroup FCDocument
*/
class FCOLLADA_EXPORT FCDPASBox : public FCDPhysicsAnalyticalGeometry
{
private:
	DeclareObjectType(FCDPhysicsAnalyticalGeometry);

public:
	/** Constructor: do not use directly. Create new analytical geometries by 
		using FCDPhysicsShape::CreateAnalyticalGeometry function.
		@param document The COLLADA document that contains this physics box. */
	FCDPASBox(FCDocument* document);

	/** Destructor. */
	virtual ~FCDPASBox() {}

	/** Retrieves the analytical geometry type for this class. 
		@return The analytical geometry type: BOX. */
	virtual GeomType GetGeomType() const {return BOX;}

	/** Calculates the volume of this analytical geometry.
		@return The volume. */
	virtual float CalculateVolume() const;

	/** Copies the physics box into a clone.
		@param clone The empty clone. If this pointer is NULL, a analytical
			geometry will be created and you will need to release the returned 
			pointer manually.
		@param cloneChildren Whether to recursively clone this entity's 
			children.
		@return The clone. */
	virtual FCDEntity* Clone(FCDEntity* clone = NULL, bool cloneChildren = false) const;

public:
	FMVector3 halfExtents; /**< Half extents of the box in 3 dimensions. */
};

/**
	A COLLADA physics plane.

	@ingroup FCDocument
*/
class FCOLLADA_EXPORT FCDPASPlane : public FCDPhysicsAnalyticalGeometry
{
private:
	DeclareObjectType(FCDPhysicsAnalyticalGeometry);

public:
	/** Constructor: do not use directly. Create new analytical geometries by 
		using FCDPhysicsShape::CreateAnalyticalGeometry function.
		@param document The COLLADA document that contains this physics plane.
	*/
	FCDPASPlane(FCDocument* document);

	/** Destructor. */
	virtual ~FCDPASPlane() {}

	/** Retrieves the analytical geometry type for this class. 
		@return The analytical geometry type: PLANE. */
	virtual GeomType GetGeomType() const {return PLANE;}

	/** Calculates the volume of this analytical geometry.
		@return The volume. */
	virtual float CalculateVolume() const;

	/** Copies the physics plane into a clone.
		@param clone The empty clone. If this pointer is NULL, a analytical
			geometry will be created and you will need to release the returned 
			pointer manually.
		@param cloneChildren Whether to recursively clone this entity's 
			children.
		@return The clone. */
	virtual FCDEntity* Clone(FCDEntity* clone = NULL, bool cloneChildren = false) const;

public:
	FMVector3 normal; /**< The normal for the plane. */
	float d; /**< The value that positions the plane. If the normal of the plane is at (A, B, C), the equation of the plane is Ax + By + Cz + d = 0. */
};

/**
	A COLLADA physics sphere.

	@ingroup FCDocument
*/
class FCOLLADA_EXPORT FCDPASSphere : public FCDPhysicsAnalyticalGeometry
{
private:
	DeclareObjectType(FCDPhysicsAnalyticalGeometry);

public:
	/** Constructor: do not use directly. Create new analytical geometries by 
		using FCDPhysicsShape::CreateAnalyticalGeometry function.
		@param document The COLLADA document that contains this physics sphere.
	*/
	FCDPASSphere(FCDocument* document);

	/** Destructor. */
	virtual ~FCDPASSphere() {}

	/** Retrieves the analytical geometry type for this class. 
		@return The analytical geometry type: SPHERE. */
	virtual GeomType GetGeomType() const {return SPHERE;}

	/** Calculates the volume of this analytical geometry.
		@return The volume. */
	virtual float CalculateVolume() const;

	/** Copies the physics sphere into a clone.
		@param clone The empty clone. If this pointer is NULL, a analytical
			geometry will be created and you will need to release the returned 
			pointer manually.
		@param cloneChildren Whether to recursively clone this entity's 
			children.
		@return The clone. */
	virtual FCDEntity* Clone(FCDEntity* clone = NULL, bool cloneChildren = false) const;

public:
	float radius; /**< The radius of the sphere. */
};

/**
	A COLLADA physics cylinder.

	@ingroup FCDocument
*/
class FCOLLADA_EXPORT FCDPASCylinder : public FCDPhysicsAnalyticalGeometry
{
private:
	DeclareObjectType(FCDPhysicsAnalyticalGeometry);

public:
	/** Constructor: do not use directly. Create new analytical geometries by 
		using FCDPhysicsShape::CreateAnalyticalGeometry function.
		@param document The COLLADA document that contains this physics 
			cylinder. */
	FCDPASCylinder(FCDocument* document);

	/** Destructor. */
	virtual ~FCDPASCylinder() {}

	/** Retrieves the analytical geometry type for this class. 
		@return The analytical geometry type: CYLINDER. */
	virtual GeomType GetGeomType() const {return CYLINDER;}

	/** Calculates the volume of this analytical geometry.
		@return The volume. */
	virtual float CalculateVolume() const;

	/** Copies the physics cylinder into a clone.
		@param clone The empty clone. If this pointer is NULL, a analytical
			geometry will be created and you will need to release the returned 
			pointer manually.
		@param cloneChildren Whether to recursively clone this entity's 
			children.
		@return The clone. */
	virtual FCDEntity* Clone(FCDEntity* clone = NULL, bool cloneChildren = false) const;

public:
	float height; /**< The height of the cylinder. */
	FMVector2 radius; /**< The radius in the X direction and Z direction of the cylinder. */
};

/**
	A COLLADA physics capsule.

	@ingroup FCDocument
*/
class FCOLLADA_EXPORT FCDPASCapsule : public FCDPhysicsAnalyticalGeometry
{
private:
	DeclareObjectType(FCDPhysicsAnalyticalGeometry);

public:
	/** Constructor: do not use directly. Create new analytical geometries by 
		using FCDPhysicsShape::CreateAnalyticalGeometry function.
		@param document The COLLADA document that contains this physics 
			capsule. */
	FCDPASCapsule(FCDocument* document);

	/** Desctructor. */
	virtual ~FCDPASCapsule() {}

	/** Retrieves the analytical geometry type for this class. 
		@return The analytical geometry type: CAPSULE. */
	virtual GeomType GetGeomType() const {return CAPSULE;}

	/** Calculates the volume of this analytical geometry.
		@return The volume. */
	virtual float CalculateVolume() const;

	/** Copies the physics capsule into a clone.
		@param clone The empty clone. If this pointer is NULL, a analytical
			geometry will be created and you will need to release the returned 
			pointer manually.
		@param cloneChildren Whether to recursively clone this entity's 
			children.
		@return The clone. */
	virtual FCDEntity* Clone(FCDEntity* clone = NULL, bool cloneChildren = false) const;

public:
	float height; /**< The height of the capsule. */
	FMVector2 radius; /**< The radius in the X direction and Z direction of the capsule. */
};

/**
	A COLLADA physics tapered capsule.

	@ingroup FCDocument
*/
class FCOLLADA_EXPORT FCDPASTaperedCapsule : public FCDPASCapsule
{
private:
	DeclareObjectType(FCDPASCapsule);

public:
	/** Constructor: do not use directly. Create new analytical geometries by 
		using FCDPhysicsShape::CreateAnalyticalGeometry function.
		@param document The COLLADA document that contains this physics tapered
			capsule. */
	FCDPASTaperedCapsule(FCDocument* document);
	
	/** Destructor. */
	virtual ~FCDPASTaperedCapsule() {}

	/** Retrieves the analytical geometry type for this class. 
		@return The analytical geometry type: TAPERED_CAPSULE. */
	virtual GeomType GetGeomType() const {return TAPERED_CAPSULE;}

	/** Calculates the volume of this analytical geometry.
		@return The volume. */
	virtual float CalculateVolume() const;

	/** Copies the physics tapered capsule into a clone.
		@param clone The empty clone. If this pointer is NULL, a analytical
			geometry will be created and you will need to release the returned 
			pointer manually.
		@param cloneChildren Whether to recursively clone this entity's 
			children.
		@return The clone. */
	virtual FCDPhysicsAnalyticalGeometry* Clone(FCDPhysicsAnalyticalGeometry* clone = NULL, bool cloneChildren = false) const;

public:
	//inherits all other attributes from Capsule
	FMVector2 radius2; /**< The second radius in the X direction and Z direction of the capsule. */
};

/**
	A COLLADA physics tapered cylinder.

	@ingroup FCDocument
*/
class FCOLLADA_EXPORT FCDPASTaperedCylinder : public FCDPASCylinder
{
private:
	DeclareObjectType(FCDPASCylinder);

public:
	/** Constructor: do not use directly. Create new analytical geometries by 
		using FCDPhysicsShape::CreateAnalyticalGeometry function.
		@param document The COLLADA document that contains this physics tapered
			cylinder. */
	FCDPASTaperedCylinder(FCDocument* document);

	/** Destructor. */
	virtual ~FCDPASTaperedCylinder() {}

	/** Retrieves the analytical geometry type for this class. 
		@return The analytical geometry type: TAPERED_CYLINDER. */
	virtual GeomType GetGeomType() const {return TAPERED_CYLINDER;}

	/** Calculates the volume of this analytical geometry.
		@return The volume. */
	virtual float CalculateVolume() const;

	/** Copies the physics tapered cylinder into a clone.
		@param clone The empty clone. If this pointer is NULL, a analytical
			geometry will be created and you will need to release the returned 
			pointer manually.
		@param cloneChildren Whether to recursively clone this entity's 
			children.
		@return The clone. */
	virtual FCDEntity* Clone(FCDEntity* clone = NULL, bool cloneChildren = false) const;

public:
	//inherits all other attributes from Cylinder
	FMVector2 radius2; /**< The second radius in the X direction and Z direction of the cylinder. */
};

/**
	[INTERNAL] The factory for COLLADA physics analytical shapes.

	Takes the type of analytical shape and returns a newly created one.

	@ingroup FCDEffect
*/
class FCOLLADA_EXPORT FCDPASFactory
{
private:
	FCDPASFactory() {}

public:
	/** Creates the physics analytical shape.
		@param document The COLLADA document that contains the physics 
			analytical shape to create.
		@param type The analytical geometry type of shape to dreate.
		@return The newly created analytical shape. */
	static FCDPhysicsAnalyticalGeometry* CreatePAS(FCDocument* document, FCDPhysicsAnalyticalGeometry::GeomType type);
};

#endif // _FCD_PHYSICS_ANALYTICAL_GEOMETRY_H_
