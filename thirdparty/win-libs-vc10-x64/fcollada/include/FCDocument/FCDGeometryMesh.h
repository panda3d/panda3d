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
	@file FCDGeometryMesh.h
	This file contains the FCDGeometryMesh class.
*/

#ifndef _FCD_GEOMETRY_MESH_H_
#define _FCD_GEOMETRY_MESH_H_

#ifndef __FCD_OBJECT_H_
#include "FCDocument/FCDObject.h"
#endif // __FCD_OBJECT_H_
#ifndef _FU_PARAMETER_H_
#include "FUtils/FUParameter.h"
#endif // _FU_PARAMETER_H_
#ifndef _FU_DAE_ENUM_H_
#include "FUtils/FUDaeEnum.h"
#endif // _FU_DAE_ENUM_H_

class FCDocument;
class FCDGeometry;
class FCDGeometryPolygons;
class FCDGeometrySource;

typedef fm::pvector<FCDGeometrySource> FCDGeometrySourceList; /**< A dynamically-sized array of FCDGeometrySource objects. */
typedef fm::pvector<const FCDGeometrySource> FCDGeometrySourceConstList; /**< A dynamically-sized array of FCDGeometrySource constant objects. */
typedef fm::pvector<FCDGeometryPolygons> FCDGeometryPolygonsList; /**< A dynamically-sized array of geometry polygon sets. */

/**
	A COLLADA geometric mesh.
	A COLLADA geometric mesh is a list of vertices tied together in polygons.
	A set of per-vertex data is used to determine the vertices of the mesh.
	This data usually includes a single list: of vertex positions, but it may also
	contain per-vertex colors, per-vertex normals or per-vertex texture coordinates.
	The other data sources declare per-vertex-face data.

	The faces of a mesh may be split across different groups, as they may have
	different materials assigned to them. The FCDGeometryPolygons objects contains one such group
	of faces.

	@ingroup FCDGeometry
*/
class FCOLLADA_EXPORT FCDGeometryMesh : public FCDObject
{
private:
	DeclareObjectType(FCDObject);

	FCDGeometry* parent;
	DeclareParameterContainer(FCDGeometrySource, sources, FC("Data Sources"));
	DeclareParameterContainer(FCDGeometryPolygons, polygons, FC("Polygons Sets"));

	DeclareParameterTrackList(FCDGeometrySource, vertexSources, FC("Vertex Sources"));
	size_t faceCount, holeCount;
	size_t faceVertexCount;

	bool isConvex; //determines if the mesh is defined as a <convex_mesh>
	bool convexify; //determines if the mesh needs to be turned into a convex mesh
	DeclareParameter(fm::string, FUParameterQualifiers::SIMPLE, convexHullOf, FC("Convex Hull of"));

public:
	/** Contructor: do not use directly. Use FCDGeometry::AddMesh instead.
		@param document The COLLADA document which owns this mesh.
		@param parent The geometry entity which contains this mesh. */
	FCDGeometryMesh(FCDocument* document, FCDGeometry* parent);

	/** Destructor. */
	virtual ~FCDGeometryMesh();

	/** [INTERNAL] Links a convex mesh to its source geometry (in convexHullOf).
		The geometry's mesh is copied into the current one and convexified.
		This function is used at the end of the import of a document to verify that the
		geometry was found.
		@return The status of the linkage. */
	bool Link();

	/** Retrieve the parent of this geometric spline: the geometry entity.
		@return The geometry entity that this spline belongs to. */
	FCDGeometry* GetParent() { return parent; }
	const FCDGeometry* GetParent() const { return parent; } /**< See above. */

	/** Retrieves the number of faces within the geometric mesh.
		@return The number of faces within the geometric mesh. */
	inline size_t GetFaceCount() const { return faceCount; }

	/** Retrieves the number of holes within the faces of the geometric mesh.
		As one face may contain multiple holes, this value may be larger than the number of faces.
		@return The number of holes within the faces of the geometric mesh. */
	inline size_t GetHoleCount() const { return holeCount; }

	/** Retrieves the total number of per-face vertices in the mesh.
		This function makes no assumption about the uniqueness of the per-face vertices.
		@return The total number of per-face vertices in the mesh. */
	inline size_t GetFaceVertexCount() const { return faceVertexCount; }

	/** Retrieves whether the mesh is defined as convex or not.
		@return Whether the mesh is defined as convex. */
	inline bool IsConvex() const { return isConvex; }

	/** Sets whether the mesh should be defined as convex.
		@param _isConvex Whether the mesh is convex. */
	inline void SetConvex(bool _isConvex) { isConvex = _isConvex; SetDirtyFlag(); }

	/** Retrieves the attribute determining if the mesh needs to be made convex.
		@return The flag determining if the mesh needs to be made convex. */
	inline bool GetConvexify() const { return convexify; }

	/** Raise a flag that the mesh needs to be made convex and the host application should 
		take care of it. In the future, we could compute ourselves the convex hull of the mesh.
		A good algorithm could be the "Quickhull".
		@param _convexify Whether the mesh needs to be made convex. */
	inline void SetConvexify(bool _convexify) { convexify = _convexify; SetDirtyFlag();}

	/** Retrieves the name of the concave mesh we need to convexify.
		@return The name of the geometry containing the mesh to convexify. */
	const fm::string& GetConvexHullOf() const {return convexHullOf;}

	/** Retrieves the convex hull of this mesh.
		@return The convex hull geometry created using this mesh. This pointer will be
			NULL if no convex hull was created using this mesh. */
	inline FCDGeometryMesh* FindConvexHullOfMesh() { return const_cast<FCDGeometryMesh*>(const_cast<const FCDGeometryMesh*>(this)->FindConvexHullOfMesh()); }
	const FCDGeometryMesh* FindConvexHullOfMesh() const; /**< See above. */

	/** Sets the name of the geometry of which this mesh is the convex hull of.
	@param _geom The geometry of which this mesh is the convex hull of.*/
	void SetConvexHullOf(FCDGeometry* _geom);

	/** [INTERNAL] Set the convec hull name directly.
		@param id The name id.
	*/
	void SetConvexHullOf(const fm::string& id){ convexHullOf = id; }

	/** Retrieves the COLLADA id of the mesh.
		This is a shortcut to the parent geometry's COLLADA id.
		@return The COLLADA id of the mesh. */
	const fm::string& GetDaeId() const;

	/** Retrieves the number of independent polygon groups.
		Each polygon group is represented within a FCDGeometryPolygons object.
		An independent polygon group is usually created to assign a different material to different parts of the mesh
		or to assign partial texture coordinates and texture tangents to different parts of the mesh.
		@return The number of independent polygon groups. */
	inline size_t GetPolygonsCount() const { return polygons.size(); }

	/** Retrieves a specific polygon group.
		Each polygon group is represented within a FCDGeometryPolygons object.
		An independent polygon group is usually created to assign a different material to different parts of the mesh
		or to assign partial texture coordinates and texture tangents to different parts of the mesh.
		@param index The index of the polygon group. This index should be less than the number
			of independent polygon groups returned by the GetPolygonsCount function.
		@return The polygon group. This pointer will be NULL if the index is out-of-bounds. */
	inline FCDGeometryPolygons* GetPolygons(size_t index) { FUAssert(index < GetPolygonsCount(), return NULL); return polygons.at(index); }
	inline const FCDGeometryPolygons* GetPolygons(size_t index) const { FUAssert(index < GetPolygonsCount(), return NULL); return polygons.at(index); } /**< See above. */

	/** Creates a new polygon group.
		Each polygon group is represented within a FCDGeometryPolygons object.
		The new polygon group will be assigned all the existing per-vertex data sources.
		No material will be assigned to the new polygon group.
		@return The new polygon group. This pointer should never be NULL. */
	FCDGeometryPolygons* AddPolygons();

	/** Retrieves if the mesh consists of only triangles.
		@return The boolean value. */
	bool IsTriangles() const;

	/** Retrieves the polygons sets which use the given material semantic.
		Useful when processing material instances.
		@param semantic The material semantic to find.
		@param sets A list of polygon sets to fill in.
			This list is not cleared. */
	void FindPolygonsByMaterial(const fstring& semantic, FCDGeometryPolygonsList& sets);

	/** Retrieves the list of per-vertex data sources.
		There should usually be one per-vertex data source that contains positions.
		All the sources within this list are also present within the data source list.
		@return The list of per-vertex data sources. */
	DEPRECATED(3.05A, GetVertexSourceCount and GetVertexSource(index)) void GetVertexSources() const {}

	/** Retrieves the number of per-vertex data sources.
		This number should always be lesser or equal to the number of data sources, as each per-vertex
		data source is also included within the list of data sources.
		@return The number of per-vertex data sources. */
	inline size_t GetVertexSourceCount() const { return vertexSources.size(); }

	/** Retrieves a specific per-vertex data source.
		All the per-vertex data sources are also included in the list of data sources.
		@param index The index of the per-vertex data source. This index should be less than the number of
			per-vertex data sources returns by the GetVertexSourceCount function.
		@return The per-vertex data source. This pointer will be NULL if the index is out-of-bounds. */
	inline FCDGeometrySource* GetVertexSource(size_t index) { FUAssert(index < GetVertexSourceCount(), return NULL); return vertexSources.at(index); }
	inline const FCDGeometrySource* GetVertexSource(size_t index) const { FUAssert(index < GetVertexSourceCount(), return NULL); return vertexSources.at(index); } /**< See above. */

	/** Creates a new per-vertex data source for this geometric mesh.
		The per-vertex data source will be added to both the per-vertex data source list and the data source list.
		The new per-vertex data source will automatically be added to all the existing polygon groups.
		@param type The type of data that will be contained within the source.
			Defaults to UNKNOWN, so that you may provide the source type later.
		@return The new per-vertex data source. This pointer should never be NULL. */
	FCDGeometrySource* AddVertexSource(FUDaeGeometryInput::Semantic type = FUDaeGeometryInput::UNKNOWN);

	/** Sets a source as per-vertex data.
		@param source A source that will now contain per-vertex data. */
	void AddVertexSource(FCDGeometrySource* source);

	/** Transforms a source of per-vertex data into a source of per-vertex-face data.
		Note: the offsets of the inputs is not changed and no data is released.
		@param source A source that will now contain per-vertex data. */
	void RemoveVertexSource(FCDGeometrySource* source);

	/** Retrieves whether a given geometry source is a per-vertex source of this mesh.
		@param source A source contained within this mesh.
		@return Whether the source is a per-vertex source of the mesh. */
	inline bool IsVertexSource(const FCDGeometrySource* source) const { return vertexSources.contains(const_cast<FCDGeometrySource*>(source)); }

	/** [INTERNAL] Retrieves the data source that matches the given COLLADA id.
		@param id A valid COLLADA id.
		@return The data source. This pointer will be NULL if no matching data source was found. */		
	FCDGeometrySource* FindSourceById(const fm::string& id) { return const_cast<FCDGeometrySource*>(const_cast<const FCDGeometryMesh*>(this)->FindSourceById(id)); }
	const FCDGeometrySource* FindSourceById(const fm::string& id) const; /**< See above. */

	/** Retrieves the first data source that matches the given geometry data type.
		@param type A geometry data type.
		@return The first data source that matches the data type. This pointer will be NULL
			if no matching data source was found. */
	FCDGeometrySource* FindSourceByType(FUDaeGeometryInput::Semantic type) { return const_cast<FCDGeometrySource*>(const_cast<const FCDGeometryMesh*>(this)->FindSourceByType(type)); }
	const FCDGeometrySource* FindSourceByType(FUDaeGeometryInput::Semantic type) const; /**< See above. */

	/** Retrieves the first data source that matches the given name.
		@param name A valid COLLADA name.
		@return The first data source that matches the name. This pointer will be NULL
			if no matching data source was found. */
	FCDGeometrySource* FindSourceByName(const fstring& name) { return const_cast<FCDGeometrySource*>(const_cast<const FCDGeometryMesh*>(this)->FindSourceByName(name)); }
	const FCDGeometrySource* FindSourceByName(const fstring& name) const; /**< See above. */

	/** Retrieves the list of data sources that matches the given geometry data type.
		@param type A geometry data type.
		@param sources A list of data sources to fill in with the matching data sources.
			This list is not cleared and no check for uniqueness is done. */
	void FindSourcesByType(FUDaeGeometryInput::Semantic type, FCDGeometrySourceList& sources) { FindSourcesByType(type, *(FCDGeometrySourceConstList*) &sources); }
	void FindSourcesByType(FUDaeGeometryInput::Semantic type, FCDGeometrySourceConstList& sources) const; /**< See above. */

	/** Retrieves the per-vertex data that specifically contains the vertex positions.
		If there are more than one per-vertex data source that contains vertex positions, the first one is returned.
		@deprecated Use FindSourceByType instead.
		@return A per-vertex data source that contains vertex positions. This pointer will be NULL
			in the unlikely possibility that there are no per-vertex data source that contains vertex positions. */
	FCDGeometrySource* GetPositionSource() { return FindSourceByType(FUDaeGeometryInput::POSITION); }
	const FCDGeometrySource* GetPositionSource() const { return FindSourceByType(FUDaeGeometryInput::POSITION); } /**< See above. */

	/** Retrieves the list of all data sources.
		Some of the sources within this list are also present within the vertex data source list.
		@return The list of all data sources. */
	DEPRECATED(3.05A, GetSourceCount and GetSource(index)) void GetSources() const {}

	/** Retrieves the number of data sources contained within this geometric mesh.
		@return The number of data sources within the mesh. */
	inline size_t GetSourceCount() const { return sources.size(); }

	/** Retrieves a specific data source.
		@param index The index of the data source. This index should be less than the number of
			data sources returns by the GetSourceCount function.
		@return The data source. This pointer will be NULL if the index is out-of-bounds. */
	inline FCDGeometrySource* GetSource(size_t index) { FUAssert(index < GetSourceCount(), return NULL); return sources.at(index); }
	inline const FCDGeometrySource* GetSource(size_t index) const { FUAssert(index < GetSourceCount(), return NULL); return sources.at(index); } /**< See above. */

	/** Creates a new data source for this geometric mesh.
		The new data source will not be added to any of the existing polygon groups.
		@param type The type of data that will be contained within the source.
			Defaults to UNKNOWN, so that you may provide the source type later.
		@return The new per-vertex data source. This pointer should never be NULL. */
	FCDGeometrySource* AddSource(FUDaeGeometryInput::Semantic type = FUDaeGeometryInput::UNKNOWN);

	/** Copies the mesh into a clone.
		The clone may reside in another document.
		@param clone The empty clone. If this pointer is NULL, a new mesh
			will be created and you will need to release the returned pointer manually.
		@return The clone. */
	FCDGeometryMesh* Clone(FCDGeometryMesh* clone = NULL) const;

	/** [INTERNAL] Forces the recalculation of the hole count, vertex count, face-vertex counts and their offsets.
		Since the counts and offsets are buffered at the geometric mesh object level, this function allows the polygon
		groups to force the recalculation of the buffered values, when they are modified. */
	void Recalculate();
};

#endif // _FCD_GEOMETRY_MESH_H_
