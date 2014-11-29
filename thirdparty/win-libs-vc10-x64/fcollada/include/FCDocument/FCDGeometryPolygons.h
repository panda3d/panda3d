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
	@file FCDGeometryPolygons.h
	This file defines the FCDGeometryPolygons class.
*/

#ifndef _FCD_GEOMETRY_POLYGONS_H_
#define _FCD_GEOMETRY_POLYGONS_H_

#ifndef __FCD_OBJECT_H_
#include "FCDocument/FCDObject.h"
#endif // __FCD_OBJECT_H_
#ifndef _FU_DAE_ENUM_H_
#include "FUtils/FUDaeEnum.h"
#endif // _FU_DAE_ENUM_H_
#ifndef _FU_PARAMETER_H_
#include "FUtils/FUParameter.h"
#endif // _FU_PARAMETER_H_

class FCDocument;
class FCDExtra;
class FCDMaterial;
class FCDGeometryMesh;
class FCDGeometrySource;
class FCDGeometryPolygonsInput;

typedef fm::pvector<FCDGeometryPolygonsInput> FCDGeometryPolygonsInputList; /**< A dynamically-sized array of FCDGeometryPolygonsInput objects. */
typedef fm::pvector<const FCDGeometryPolygonsInput> FCDGeometryPolygonsInputConstList; /**< A dynamically-sized array of FCDGeometryPolygonsInput objects. */
typedef fm::map<const FCDGeometrySource*, FCDGeometrySource*> FCDGeometrySourceCloneMap; /**< A map of old FCDGeometrySource objects to newly cloned FCDGeometrySource objects. */

/**
	A mesh polygon set.
	Each polygon set contains a list of inputs and the tessellation information
	to make polygons out of the data and indices of the input. FCollada
	supports triangle lists as well as polygon lists and lists of polygons with holes.
	This implies that each face has an undeterminate number of vertices.
	The tessellation information creates polygons, but may also creates holes within the polygons.

	@ingroup FCDGeometry
*/
class FCOLLADA_EXPORT FCDGeometryPolygons : public FCDObject
{
public:
	/** The types of primitives. */
	enum PrimitiveType
	{
		LINES, /**< A list of lines. Only one element is contained in the face-vertex count list.
			It represents the total number of line vertices. The total number of lines is
			equal to half the total number of line vertices. */
		LINE_STRIPS, /**< A list of continuous lines. Each element in the face-vertex count list
			represents the number of consecutive line vertices before restarting. */
		POLYGONS, /**< A list of polygons. All the polygons may be triangles.
			This is the most common primitive type. The polygons may have holes. Each element in the
			face-vertex count list represent the number of vertices for one polygon. */
		TRIANGLE_FANS, /**< A list of triangle fans. Each element in the face-vertex count list
			represents the number of vertices for one fan. A triangle fan is defined by re-using the first
			vertex for every triangle. Advancing pairs are then used in order to generate adjacent triangles
			such that if there are 5 vertices, then 3 triangles are created: {0,1,2}, {0,2,3} and {0,3,4}. */
		TRIANGLE_STRIPS, /**< A list of continuous triangles. Each element in the face-vertex count list
			represents the number of vertices for one strip. A triangle strip is defined by re-using two
			advancing vertices from the previous triangle for the next triangle. If there are 5 vertices in 
			the strip, then 3 triangles are created: {0,1,2}, {1,2,3}, {2,3,4}. Note that vertex winding must
			also be taken into consideration and every even triangle in the strip has its vertices swapped
			from the above pattern. */
		POINTS /**< A list of Camera-facing sprites. The face-vertex count list will contain one element that 
			represents the total number of points. Two non-COLLADA geometry sources (POINT_SIZE and POINT_ROTATION)
			are specific to this type. */
	};

private:
	DeclareObjectType(FCDObject);

	FCDGeometryMesh* parent;
	DeclareParameterContainer(FCDGeometryPolygonsInput, inputs, FC("Data Inputs"));
	DeclareParameterList(UInt32, faceVertexCounts, FC("Per-face Vertex counts"));
	DeclareParameterList(UInt32, holeFaces, FC("Hole face indices"));
	DeclareParameter(uint32, FUParameterQualifiers::SIMPLE, primitiveType, FC("Primitive Type")); // PrimitiveType

	// Buffered statistics
	size_t faceVertexCount;
	size_t faceOffset;
	size_t faceVertexOffset;
	size_t holeOffset;

	// Material for this set of polygons
	DeclareParameter(fstring, FUParameterQualifiers::SIMPLE, materialSemantic, FC("Material Semantic"));

	// Extra information tree
	DeclareParameterRef(FCDExtra, extra, FC("Extra Tree"));

public:
	/** Constructor: do not use directly. Instead, use the FCDGeometryMesh::AddPolygons function
		to create new polygon sets.
		@param document The COLLADA document which owns this polygon set.
		@param parent The geometric mesh which contains this polygon set.*/
	FCDGeometryPolygons(FCDocument* document, FCDGeometryMesh* parent);

	/** Destructor. */
	virtual ~FCDGeometryPolygons();

	/** Retrieves the geometry that contains this polygons.
		@return The parent geometry. */
	inline FCDGeometryMesh* GetParent() { return parent; }
	inline const FCDGeometryMesh* GetParent() const { return parent; } /**< See above. */

	/** Retrieves the extra information tree for this entity instance. The 
		prefered way to save extra information in FCollada is at the entity 
		level. Use this extra information tree to store any information you 
		want exported and imported back.
		@return The extra information tree. */
	FCDExtra* GetExtra();
	inline const FCDExtra* GetExtra() const { return const_cast<FCDGeometryPolygons*>(this)->GetExtra(); } /**< See above. */

	/** Retrieves the primitive type for this polygons set.
		@return The primitive type. */
	inline PrimitiveType GetPrimitiveType() const { return (PrimitiveType) *primitiveType; }

	/** Sets the primitive type for this polygons set.
		Important note: no attempt is made at fixing up the indices. You should
		only do this operation of empty polygons set.
		@param type The new primitive type. */
	inline void SetPrimitiveType(PrimitiveType type) { primitiveType = type; SetDirtyFlag(); }

	/** Retrieves the list of face-vertex counts. Each face within the polygon set
		has one or more entry within this list, depending on the number of holes within that face.
		Each face-vertex count indicates the number of ordered indices
		within the polygon set inputs that are used to generate a face or its holes.
		To find out if a face-vertex count represents a face or its holes, check
		the hole-faces list retrieved using the GetHoleFaces function.
		Indirectly, the face-vertex count indicates the degree of the polygon.
		@see GetHoleFaces @see GetHoleCount
		@return The list of face-vertex counts.*/
	inline const uint32* GetFaceVertexCounts() const { return faceVertexCounts.begin(); } /**< See above. */

	/** Adds a new count to the face-vertex count list.
		This function only modifies the face-vertex count list.
		To also add indices to the inputs for the new face, use the AddFace function.
		@param count The number of vertices in the new face. */
	void AddFaceVertexCount(uint32 count);

	/** Retrieves the number of face-vertex counts within the polygon set.
		This value also represents the total the number of faces and holes within the polygon set.
		@return The number of face-vertex counts within the polygon set. */
	inline size_t GetFaceVertexCountCount() const { return faceVertexCounts.size(); }

	/** Sets the number of face-vertex counts within the polygon set.
		Any additional face-vertex count will not be initialized and
		any removed face-vertex count will not remove the equivalent
		indices within the polygon set inputs.
		@param count The new number of face-vertex counts within the polygon set. */
	void SetFaceVertexCountCount(size_t count);

	/** [DEPRECATED]
		Retrieves if the polygons is a list of polygons (returns false), or a list of triangles
		(returns true).
		@return The boolean answer. */
	DEPRECATED(3.05, TestPolyType) bool IsTriangles() const;

	/** Tests if the polygon can be aproximated with a constant face count
		Returns: 3 if all faces are triangles, 4 if all faces are triangles, else -1 */
	int32 TestPolyType() const;

	/** Retrieves the number of holes within the faces of the polygon set.
		@return The number of holes within the faces of the polygon set. */
	inline size_t GetHoleCount() const { return holeFaces.size(); }

	/** Retrieves the number of faces within the polygon set.
		@return The number of faces within the polygon set. */
	inline size_t GetFaceCount() const { return faceVertexCounts.size() - GetHoleCount(); }

	/** Retrieves the number of faces which appear before this polygon set within the geometric mesh.
		This value is useful when traversing all the faces of a geometric mesh.
		@return The number of faces in previous polygon sets. */
	inline size_t GetFaceOffset() const { return faceOffset; }

	/** Retrieves the total number of face-vertex pairs within the polygon set.
		This value is the total of all the values within the face-vertex count list.
		Do remember that the list of face-vertex pairs includes holes.
		@return The total number of face-vertex pairs within the polygon set. */
	inline size_t GetFaceVertexCount() const { return faceVertexCount; }

	/** Retrieves the number of face-vertex pairs for a given face.
		This value includes face-vertex pairs that create the polygon and its holes.
		@param index A face index.
		@return The number of face-vertex pairs for a given face. */
	size_t GetFaceVertexCount(size_t index) const;

	/** Retrieves the total number of face-vertex pairs which appear
		before this polygon set within the geometric mesh.
		This value is useful when traversing all the face-vertex pairs of a geometric mesh.
		@return The number of face-vertex pairs in previous polygon sets. */
	inline size_t GetFaceVertexOffset() const { return faceVertexOffset; }

	/** Retrieves the number of holes which appear before this polygon set.
		This value is useful when traversing all the face-vertex pairs of a geometric mesh. */
	inline size_t GetHoleOffset() const { return holeOffset; }

	/** Retrieves the number of face-vertex pairs which appear
		before a given face within the polygon set.
		This value is useful when doing per-vertex mesh operations within the polygon set.
		@param index The index of the face.
		@return The number of face-vertex pairs before the given face, within the polygon set. */
	size_t GetFaceVertexOffset(size_t index) const;

	/** [INTERNAL] Sets the number of faces in previous polygon sets.
		Used by the FCDGeometryMesh::Recalculate function.
		@param offset The number of faces in previous polygon sets. */
	inline void SetFaceOffset(size_t offset) { faceOffset = offset; SetDirtyFlag(); }

	/** [INTERNAL] Sets the number of face-vertex pairs in previous polygon sets.
		Used by the FCDGeometryMesh::Recalculate function.
		@param offset The number of face-vertex pairs in previous polygon sets. */
	inline void SetFaceVertexOffset(size_t offset) { faceVertexOffset = offset; SetDirtyFlag(); }

	/** [INTERNAL] Sets the number of holes in previous polygon sets.
		Used by the FCDGeometryMesh::Recalculate function.
		@param offset The number of holes in previous polygon sets. */
	inline void SetHoleOffset(size_t offset) { holeOffset = offset; SetDirtyFlag(); }

	/** Creates a new face.
		Enough indices to fill the face will be added to the polygon set inputs: you will
		want to overwrite those, as they will all be set to zero.
		@param degree The degree of the polygon. This number implies the number of indices
			that will be expected, in order, within each of the input index lists. */
	virtual void AddFace(uint32 degree);

	/** Removes a face.
		@param index The index of the face to remove. All the indices associated
			with this face will also be removed. */
	virtual void RemoveFace(size_t index);

	/** Retrieves the list of polygon set inputs.
		@see FCDGeometryPolygonsInput
		@return The list of polygon set inputs. */
	DEPRECATED(3.05A, GetInputCount and GetInput(index)) void GetInputs() {}

	/** Retrieves the number of polygon set inputs.
		@return The number of polygon set inputs. */
	inline size_t GetInputCount() const { return inputs.size(); }

	/** Retrieves a specific polygon set input.
		@param index The index of the polygon set input. This index should
			not be greater than or equal to the number of polygon set inputs.
		@return The specific polygon set input. This pointer will be NULL if the index is out-of-bounds. */
	inline FCDGeometryPolygonsInput* GetInput(size_t index) { FUAssert(index < GetInputCount(), return NULL); return inputs.at(index); }
	inline const FCDGeometryPolygonsInput* GetInput(size_t index) const { FUAssert(index < GetInputCount(), return NULL); return inputs.at(index); } /**< See above. */

	/** Creates a new polygon set input.
		@param source The data source for the polygon set input.
		@param offset The tessellation indices offset for the polygon set input.
			If this value is new to the list of polygon inputs, you will need to fill in the indices.
			Please use the FindIndices function to verify that the offset is new and that indices need
			to be provided. The offset of zero is reserved for per-vertex data sources.
		@return The new polygon set input. */
	FCDGeometryPolygonsInput* AddInput(FCDGeometrySource* source, uint32 offset);

	/** Retrieves the number of hole entries within the face-vertex count list.
		@return The number of hole entries within the face-vertex count list. */
	inline size_t GetHoleFaceCount() const { return holeFaces.size(); }

	/** Sets the number of hole entries within the face-vertex count list.
		Any additional hole entries will need to be initialized by the application.
		Reducing the number of hole entries without taking special care to remove
		these entries from the face-vertex count list will result in new, unwanted, faces.
		@param count The number of hole entries within the face-vertex count list. */
	void SetHoleFaceCount(size_t count);

	/** Checks whether a given face-vertex count entries represents a hole.
		@param index The index of the face-vertex count entry.
		@return Whether this face-vertex count entry is a hole. */
	bool IsHoleFaceHole(size_t index);

	/** Retrieves the list of entries within the face-vertex count list
		that are considered holes. COLLADA does not support holes within holes,
		so each entry within this list implies a hole within the previous face.
		@see GetFaceVertexCounts
		@return The list of hole entries within the face-vertex counts. */
	inline const uint32* GetHoleFaces() const { return holeFaces.begin(); } /**< See above. */

	/** Adds a new hole identifier.
		The face-vertex count entry should already exist and the identifier will be place
		in increasing order within the current list of entries within the face-vertex count list.
		@param index The index of the hole within the face-vertex count list. */
	void AddHole(uint32 index);

	/** Retrieves the number of holes within faces of the polygon set that appear
		before the given face index. This value is useful when trying to access
		a specific face of a mesh, as holes and faces appear together within the 
		face-vertex degree list.
		@param index A face index.
		@return The number of holes within the polygon set that appear
			before the given face index. */
	size_t GetHoleCountBefore(size_t index) const;

	/** Retrieves the number of holes within a given face.
		@param index A face index.
		@return The number of holes within the given face. */
	size_t GetHoleCount(size_t index) const;

	/** Retrieves the first polygon set input found that has the given data type.
		@param semantic A type of geometry data.
		@return The polygon set input. This pointer will be NULL if 
			no polygon set input matches the data type. */
	FCDGeometryPolygonsInput* FindInput(FUDaeGeometryInput::Semantic semantic) { return const_cast<FCDGeometryPolygonsInput*>(const_cast<const FCDGeometryPolygons*>(this)->FindInput(semantic)); }
	const FCDGeometryPolygonsInput* FindInput(FUDaeGeometryInput::Semantic semantic) const; /**< See above. */

	/** Retrieves the polygon set input that points towards a given data source.
		@param source A geometry data source.
		@return The polygon set input. This pointer will be NULL if
			no polygon set input matches the data source. */
	FCDGeometryPolygonsInput* FindInput(const FCDGeometrySource* source) { return const_cast<FCDGeometryPolygonsInput*>(const_cast<const FCDGeometryPolygons*>(this)->FindInput(source)); }
	const FCDGeometryPolygonsInput* FindInput(const FCDGeometrySource* source) const; /**< See above. */

	/** [INTERNAL] Retrieves the polygon set input that points towards a given data source.
		@param sourceId The COLLADA id of a geometry data source.
		@return The polygon set input. This pointer will be NULL if
			no polygon set input matches the COLLADA id. */
	FCDGeometryPolygonsInput* FindInput(const fm::string& sourceId);

	/** Retrieves all the polygon set inputs that have the given data type.
		@param semantic A type of geometry data.
		@param inputs A list of polygon set inputs to fill in. This list is not emptied by the function
			and may remain untouched, if no polygon set input matches the given data type. */
	inline void FindInputs(FUDaeGeometryInput::Semantic semantic, FCDGeometryPolygonsInputList& inputs) { const_cast<FCDGeometryPolygons*>(this)->FindInputs(semantic, *(FCDGeometryPolygonsInputConstList*)&inputs); }
	void FindInputs(FUDaeGeometryInput::Semantic semantic, FCDGeometryPolygonsInputConstList& inputs) const; /**< See above. */

	/** Retrieves the symbolic name for the material used on this polygon set.
		Match this symbolic name within a FCDGeometryInstance to get the
		correct material instance.
		@return A symbolic material name. */
	inline const fstring& GetMaterialSemantic() const { return materialSemantic; }

	/** Sets a symbolic name for the material used on this polygon set.
		This symbolic name will be matched in the FCDMaterialInstance contained within 
		a FCDGeometryInstance to assign the correct material.
		@param semantic The symbolic material name. */
	inline void SetMaterialSemantic(const fchar* semantic) { materialSemantic = semantic; SetDirtyFlag(); }
	inline void SetMaterialSemantic(const fstring& semantic) { materialSemantic = semantic; SetDirtyFlag(); } /**< See above. */

	/** [INTERNAL] Recalculates the buffered offset and count values for this polygon set. */
	virtual void Recalculate();

	/** [INTERNAL] Creates a copy of this mesh.
		You should use the FCDGeometry::Clone function instead of this function.
		@param clone The clone polygon set.
		@param cloneMap A match-map of the original geometry sources to the clone geometry sources for the mesh.
		@return The clone polygon set. */
	virtual FCDGeometryPolygons* Clone(FCDGeometryPolygons* clone, const FCDGeometrySourceCloneMap& cloneMap) const;
};

#endif // _FCD_GEOMETRY_POLYGONS_H_
