/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

/**
	@file FCDGeometry.h
	This file contains the FCDGeometry class.
	The FCDGeometry class holds the information for one mesh or spline
	entity, within the COLLADA geometry library.
*/
#ifndef _FCD_GEOMETRY_H_
#define _FCD_GEOMETRY_H_

#ifndef _FCD_ENTITY_H_
#include "FCDocument/FCDEntity.h"
#endif // _FCD_ENTITY_H_
#ifndef _FU_DAE_ENUM_H_
#include "FUtils/FUDaeEnum.h"
#endif // _FU_DAE_ENUM_H_

class FCDocument;
class FCDGeometryMesh;
class FCDGeometrySpline;

/**
	@defgroup FCDGeometry COLLADA Document Geometry Entity
	@addtogroup FCDocument
*/

/**
	A COLLADA geometry entity.
	There are three types of COLLADA geometry entities: meshes, NURBS surfaces and splines.
	*NURBS Surfaces are part of the Premium Build only.*

	Meshes are collections of polygons where the vertices always have a position,
	usually have a normal to define smooth or hard edges and may be colored or textured.

	Parameterized surfaces (PSurface) are a list of parameterized patches defined as a sequence of 
	weighted control points that may be associated with two knot vectors
	and possibly trimming curves to define a 3D surface.

	Note: "PSurfaces" are part of the Premium Build only.

	Splines are a sequence of control points used to generate a smooth curve.

	@ingroup FCDGeometry
*/

class FCOLLADA_EXPORT FCDGeometry : public FCDEntity
{
private:
	DeclareObjectType(FCDEntity);

	// Contains only one of the following, in order of importance.
	DeclareParameterRef(FCDGeometryMesh, mesh, FC("Mesh"));
	DeclareParameterRef(FCDGeometrySpline, spline, FC("Spline"));

public:
	/** Contructor: do not use directly.
		Create new geometries using the FCDLibrary::AddEntity function.
		@param document The COLLADA document which owns the new geometry entity. */
	FCDGeometry(FCDocument* document);

	/** Destructor. */
	virtual ~FCDGeometry();

	/**	[INTERNAL] Set geometry information.
		@param data The specific geometry data structure.
	*/
	void SetMesh(FCDGeometryMesh* data){ mesh = data; }
	void SetSpline(FCDGeometrySpline* data){ spline = data; } /** see above */

	/** Retrieves the entity class type.
		This function is a part of the FCDEntity interface.
		@return The entity class type: GEOMETRY. */
	virtual Type GetType() const { return GEOMETRY; }

	/** Retrieves whether the type of this geometry is a mesh.
		@return Whether this geometry is a mesh. */
	bool IsMesh() const { return mesh != NULL; }

	/** Retrieves the mesh information structure for this geometry.
		Verify that this geometry is a mesh using the IsMesh function prior to calling this function.
		@return The mesh information structure. This pointer will be NULL when the geometry is a spline or is undefined. */
	FCDGeometryMesh* GetMesh() { return mesh; }
	const FCDGeometryMesh* GetMesh() const { return mesh; } /**< See above. */

	/** Sets the type of this geometry to mesh and creates an empty mesh structure.
		This function will release any mesh or spline structure that the geometry may already contain
		@return The mesh information structure. This pointer will always be valid. */
	FCDGeometryMesh* CreateMesh();

	/** Retrieves whether the type of this geometry is a spline.
		@return Whether this geometry is a spline. */
	bool IsSpline() const { return spline != NULL; }

	/** Retrieves the spline information structure for this geometry.
		Verify that this geometry is a spline using the IsSpline function prior to calling this function.
		@return The spline information structure. This pointer will be NULL when the geometry is a mesh or is undefined. */
	FCDGeometrySpline* GetSpline() { return spline; }
	const FCDGeometrySpline* GetSpline() const { return spline; } /**< See above. */

	/** Sets the type of this geometry to spline and creates an empty spline structure.
		This function will release any mesh or spline structure that the geometry may already contain.
		@return The spline information structure. This pointer will always be valid. */
	FCDGeometrySpline* CreateSpline();


	/** Retrieves whether the type of this geometry is a parameterized surface.
		@return Whether this geometry is a parameterized surface. */
	bool IsPSurface() const { return false; }

	/** Copies the geometry entity into a clone.
		The clone may reside in another document.
		@param clone The empty clone. If this pointer is NULL, a new geometry entity
			will be created and you will need to release the returned pointer manually.
		@param cloneChildren Whether to recursively clone this entity's children.
		@return The clone. */
	virtual FCDEntity* Clone(FCDEntity* clone = NULL, bool cloneChildren = false) const;
};

#endif // _FCD_GEOMETRY_H_
