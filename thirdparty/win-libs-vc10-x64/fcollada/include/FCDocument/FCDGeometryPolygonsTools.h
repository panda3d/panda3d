/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

/**
	@file FCDGeometryPolygonsTools.h
	This file defines the FCDGeometryPolygonsTools namespace.
*/

#ifndef _FCD_GEOMETRY_POLYGONS_TOOLS_H_
#define _FCD_GEOMETRY_POLYGONS_TOOLS_H_

class FCDGeometryMesh;
class FCDGeometrySource;
class FCDGeometryPolygons;
class FCDGeometryPolygonsInput;

/** A translation map between old vertex position indices and the new indices.
	It is generated in the FCDGeometryPolygonsTools::GenerateUniqueIndices function. */
typedef fm::map<uint32, UInt32List> FCDGeometryIndexTranslationMap;
typedef fm::pvector<FCDGeometryIndexTranslationMap> FCDGeometryIndexTranslationMapList; /**< A dynamically-sized array of translation maps. */
typedef fm::vector<UInt32List> FCDNewIndicesList; /**< A dynamically-sized array of index lists. */

/** Holds commonly-used transformation functions for meshes and polygons sets. */
namespace FCDGeometryPolygonsTools
{

	/** MAX_BUFFER_COUNT is the default value to split by when packing vertices and indices.
		It represents the usual upper limit of video cards on buffer submission size*/
	#define MAX_BUFFER_COUNT	((uint16)~0) 

	/** Triangulates a mesh.
		@param mesh The mesh to triangulate. */
	FCOLLADA_EXPORT void Triangulate(FCDGeometryMesh* mesh);

	/** Triangulates a polygons set.
		@param polygons The polygons set to triangulate.
		@param recalculate Whether the statistics of the mesh should be recalculated
			after the tool has completed. */
	FCOLLADA_EXPORT void Triangulate(FCDGeometryPolygons* polygons, bool recalculate = true);

	/** Generates the texture tangents and binormals for a given source of texture coordinates.
		A source of normals and a source of vertex positions will be expected.
		@param mesh The mesh that contains the texture coordinate source.
		@param texcoordSource The source of texture coordinates that needs its tangents and binormals generated.
		@param generateBinormals Whether the binormals should also be generated.
			Do note that the binormal is always the cross-product of the tangent and the normal at a vertex-face pair. */
	FCOLLADA_EXPORT void GenerateTextureTangentBasis(FCDGeometryMesh* mesh, FCDGeometrySource* texcoordSource, bool generateBinormals = true);

	/** Prepares the mesh for using its geometry sources in vertex buffers with a unique index buffer.
		This is useful for older engines and the applications that only support one index
		per face-vertex pair.
		@param mesh The mesh to process.
		@param polygons The polygons set to isolate and process. If this pointer is NULL, the whole mesh is processed
			for one index buffer.
		@param translationMap Optional map that returns how to translate old vertex position indices into new indices.
			This map is necessary to support skins and morphers. */
	FCOLLADA_EXPORT void GenerateUniqueIndices(FCDGeometryMesh* mesh, FCDGeometryPolygons* polygons = NULL, FCDGeometryIndexTranslationMap* translationMap = NULL);

    /** Prepares the mesh for using its geometry sources in vertex buffers with a unique index buffer.
		This is useful for older engines and the applications that only support one index
		per face-vertex pair. This version of the GenerateUniqueIndices function fills in the outTranslationMaps,
        but does not modify the data in the FCDGeometryPolygon objects.
        @param mesh The mesh to process.
        @param polygons The polygons set to isolate and process. If this pointer is NULL, the whole mesh is processed
			for one index buffer.
        @param outIndices The unique index buffer data.
        @param outTranslationMaps How the vertex data needs to be relocated in order to use the unique index buffer data. */
	FCOLLADA_EXPORT void GenerateUniqueIndices(FCDGeometryMesh* mesh, FCDGeometryPolygons* polygonsToProcess, FCDNewIndicesList& outIndices, FCDGeometryIndexTranslationMapList& outTranslationMaps);

	/** Applies the translation map onto some vertex data.
        Refer to the information within the translation map to pre-cache the necessary amount of vertices. 
        @param targData The vertex buffer data to be filled-in.
        @param srcData The original vertex data.
        @param stride The stride within the target vertex buffer data and within the orignal vertex data.
        @param translationMap How to relocate the vertex data. */
	FCOLLADA_EXPORT void ApplyUniqueIndices(float* targData, float* srcData, uint32 stride, const FCDGeometryIndexTranslationMap* translationMap);

	/** Applies the translation map onto some vertex data.
        Refer to the information within the translation map to pre-cache the necessary amount of vertices. 
        @param targMesh The mesh object to be modified in-place.
        @param baseMesh The original mesh object that contains the original vertex data.
        @param newIndices The unique index buffer data.
        @param translationMaps How the vertex data needs to be relocated in order to use the unique index buffer data. */
	FCOLLADA_EXPORT void ApplyUniqueIndices(FCDGeometryMesh* targMesh, FCDGeometryMesh* baseMesh, const UInt32List& newIndices, const FCDGeometryIndexTranslationMapList& translationMaps);

	/** Reverts the modifications done when applying the unique index
        buffer data translation unto some polygons input.
        IMPORTANT: this function is un-tested, DO NOT USE.
        @param inSrc ?
        @param outSrc ?
        @param translationMap ? */
	FCOLLADA_EXPORT void RevertUniqueIndices(const FCDGeometryPolygonsInput& inSrc, FCDGeometryPolygonsInput& outSrc, const FCDGeometryIndexTranslationMap& translationMap);

    /** Used to pack a vertex buffer object with 3D float vertex data.
        @param destBuffer The vertex buffer memory.
        @param destBufferStride The size of one vertex in the vertex buffer.
        @param source The original mesh vertex data.
        @param vCount The number of vertex elements in the original mesh vertex data. 
        @param vtxPackingMap The packing map, generated by GenerateVertexPackingMap, for this vertex data.
        @param translationMap The translation map, generated by GenerateUniqueIndices, for this vertex data. */
	FCOLLADA_EXPORT void PackVertexBufferV3(uint8* destBuffer, uint32 destBuffStride, 
		const FCDGeometrySource* source, uint32 vCount, uint16* vtxPackingMap,
		const FCDGeometryIndexTranslationMap& translationMap);

    /** Used to pack a vertex buffer object object with 4 bytes color data.
        @param destBuffer The vertex buffer memory.
        @param destBufferStride The size of one vertex in the vertex buffer.
        @param source The original mesh vertex data.
        @param vCount The number of vertex elements in the original mesh vertex data. 
        @param vtxPackingMap The packing map, generated by GenerateVertexPackingMap, for this vertex data.
        @param translationMap The translation map, generated by GenerateUniqueIndices, for this vertex data. */
	FCOLLADA_EXPORT void PackVertexBufferColor(uint8* destBuffer, uint32 destBuffStride, 
		const FCDGeometrySource* source, uint32 vCount, uint16* vtxPackingMap,
		const FCDGeometryIndexTranslationMap& translationMap);
	
    /** Used to pack a vertex buffer object object with 2D float vertex data.
        @param destBuffer The vertex buffer memory.
        @param destBufferStride The size of one vertex in the vertex buffer.
        @param source The original mesh vertex data.
        @param vCount The number of vertex elements in the original mesh vertex data. 
        @param vtxPackingMap The packing map, generated by GenerateVertexPackingMap, for this vertex data.
        @param translationMap The translation map, generated by GenerateUniqueIndices, for this vertex data. */
	FCOLLADA_EXPORT void PackVertexBufferV2(uint8* destBuffer, uint32 destBuffStride, 
		const FCDGeometrySource* source, uint32 vCount, uint16* vtxPackingMap,
		const FCDGeometryIndexTranslationMap& translationMap);
	
	/** Generates a packing map for a mesh' given vertex data.
        @param maxIndex The total number of indices in the input index data.
        @param maxIndices The maximum number of indices to include in one packing map. Generally, this number would be 2^16-1.
        @param maxVertices The maximum number of vertices to include in one packing map. Generally, this number would also be 2^16-1.
        @param inIndices The input unique index data.
        @param outIndices The resulting indices. 
        @param outPackingMap The packing maps. These maps are used to identify 
        @param outNVertices The number of vertices for each split packing map.
        @return The number of indices packed away. */
	FCOLLADA_EXPORT uint16 GenerateVertexPackingMap(size_t maxIndex, size_t maxIndices, size_t maxVertices, const uint32* inIndices,
        uint16* outIndices, UInt16List* outPackingMap, uint16* outNVertices=NULL);

    /** Retrieves the largest unique index within a given translation map.
        This function is unnecessary expensive and we suggest that you cache its results.
        @param translationMap A translation map.
        @return The largest unique index within the translation map. */
	FCOLLADA_EXPORT uint32 FindLargestUniqueIndex(const FCDGeometryIndexTranslationMap& translationMap);

	/** Splits the mesh's polygons sets to ensure that none of them have more than a given number of indices within their
		index buffers. If you intend of using the GenerateUniqueIndices tool on your meshes, you should run it before this tool.
		This function affects only the indices and the number of polygon sets within a mesh and therefore has no impact on
		skins or morphers attached to the given mesh.
		@param mesh The mesh to process.
		@param maximumIndexCount The maximum number of indices to have within each polygons set. */
	FCOLLADA_EXPORT void FitIndexBuffers(FCDGeometryMesh* mesh, size_t maximumIndexCount);

	/** Reverses all the normals of a mesh.
		Since they are related to normals, this function also reverses geometric
		tangents and binormals as well as texture tangents and binormals.
		@param mesh The mesh to process. */
	FCOLLADA_EXPORT void ReverseNormals(FCDGeometryMesh* mesh);
};

#endif // _FCD_GEOMETRY_POLYGONS_TOOLS_H_
