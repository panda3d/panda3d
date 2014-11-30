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
	@file FCDGeometryPolygonsInput.h
	This file defines FCDGeometryPolygonsInput class.
*/

#ifndef _FCD_GEOMETRY_POLYGONS_INPUT_H_
#define _FCD_GEOMETRY_POLYGONS_INPUT_H_

#ifndef __FCD_OBJECT_H_
#include "FCDocument/FCDObject.h"
#endif // __FCD_OBJECT_H_
#ifndef _FU_PARAMETER_H_
#include "FUtils/FUParameter.h"
#endif // _FU_PARAMETER_H_

class FCDGeometrySource;
class FCDGeometryPolygons;

/**
	An input data source for one mesh polygons set.
	This structure knows the type of input data in the data source
	as well as the set and offset for the data. It also contains a
	pointer to the mesh data source.

	Many polygon set inputs may have the same offset (or 'idx' in older versions) when multiple
	data sources are compressed together within the COLLADA document.
	In this case, one and only one of the polygon set input should contain
	indices. To find the indices of any polygon set input,
	it is recommended that you use the FCDGeometryPolygons::FindIndicesForIdx function.

	@ingroup FCDGeometry
*/
class FCOLLADA_EXPORT FCDGeometryPolygonsInput : public FCDObject, FUTracker
{
private:
	DeclareObjectType(FCDObject);

	FCDGeometryPolygons* parent;
	DeclareParameterPtr(FCDGeometrySource, source, FC("Data Source"));
	DeclareParameter(int32, FUParameterQualifiers::SIMPLE, set, FC("Input Set"));
	DeclareParameter(uint32, FUParameterQualifiers::SIMPLE, offset, FC("Stream Offset"));
	DeclareParameterList(UInt32, indices, FC("Data Indices"));

public:
	/** Constructor.
		@param parent The polygons sets that contains the input.
		@param offset The offset for the indices of this input
			within the interlaced tesselation information. */
    FCDGeometryPolygonsInput(FCDocument* document, FCDGeometryPolygons* parent);

	/** Destructor. */
	~FCDGeometryPolygonsInput();

	/** Retrieves the type of data to input.
		FCollada doesn't support data sources which are interpreted
		differently depending on the input. Therefore, this information
		is retrieved from the source.
		@return The source data type. */
	FUDaeGeometryInput::Semantic GetSemantic() const; 

	/** Retrieves the data source references by this polygon set input.
		This is the data source into which the indices are indexing.
		You need to take the data source stride into consideration
		when un-indexing the data.
		@return The data source. */
	FCDGeometrySource* GetSource() { return source; }
	const FCDGeometrySource* GetSource() const { return source; } /**< See above. */

	/** Sets the data source referenced by this polygon set input.
		@param source The data source referenced by the input indices. */
	void SetSource(FCDGeometrySource* source);

	/** Retrieves the offset within the interlaced COLLADA indices for this input's indices.
		Changing this value after the input's construction is not recommended.
		@param _offset The offset within the interlaced COLLADA indices for this input's indices. */
	inline void SetOffset(uint32 _offset) { offset = _offset; }

	/** Retrieves the offset within the interlaced COLLADA indices for this input's indices.
		@return The interlaced indices offset. */
	inline uint32 GetOffset() const { return offset; }

	/** Retrieves the input set.
		The input set is used to group together the texture coordinates with the texture tangents and binormals.
		In ColladaMax, this value should also represent the map channel index for texture coordinates
		and vertex color channels.
		@return The input set. */
	inline int32 GetSet() const { return set; }

	/** Sets the input set.
		The input set is used to group together the texture coordinates with the texture tangents and binormals.
		In ColladaMax, this value should also represent the map channel index for texture coordinates
		and vertex color channels.
		@param _set The new input set. If the given value is -1, this input does not belong to any set.*/
	inline void SetSet(int32 _set) { set = _set; }
	
	/** Checks whether this polygon set input owns the local indices for its offset.
		Since an offset may be shared, only one polygon set input will own the local indices.
		@return Whether this polygon set owns the local indices. */
	inline bool OwnsIndices() const { return !indices.empty(); }
	
	/** Sets the local indices for the input's offset.
		This function may fail if another input already owns the indices for this offset.
		@param indices A static list of indices.
		@param count The number of indices in the given list. */
	void SetIndices(const uint32* indices, size_t count);

	/** Sets the number of indices in this input's index list.
		Any additional indices allocated will not be initialized.
		@param count The number of indices in this input's index list. */
	void SetIndexCount(size_t count);

	/** Reserves memory for a given number of indices.
		This is useful for performance reasons, since AddIndex can be extremely costly otherwise.
		@param count The total number of indices to prepare memory for. */
	void ReserveIndexCount(size_t count);

	/** Adds an index to the indices of this input.
		Be careful when adding indices directly to multiple inputs: if
		multiple inputs share an index list, you may be adding unwanted indices.
		@param index The index to add. */
	void AddIndex(uint32 index);

	/** Adds a batch of indices to the end of the index list of this input.
		Be careful when adding indices directly to multiple inputs: if
		multiple inputs share an index list, you may be adding unwanted indices.
		@param indices The indices to add. */
	void AddIndices(const UInt32List& indices);

	/** Retrieves the list of indices for this input.
		@return The list of indices for this input. */
	uint32* GetIndices() { return const_cast<uint32*>(const_cast<const FCDGeometryPolygonsInput*>(this)->GetIndices()); }
	const uint32* GetIndices() const; /**< See above. */

	/** Retrieves the number of indices for this input.
		@return The number of indices for this input. */
	size_t GetIndexCount() const;

private:
	// FUTracker interface.
	virtual void OnObjectReleased(FUTrackable* object);

	// Finds the index buffer for this list.
	FUParameterUInt32List& FindIndices() { return const_cast<FUParameterUInt32List&>(const_cast<const FCDGeometryPolygonsInput*>(this)->FindIndices()); }
	const FUParameterUInt32List& FindIndices() const;
};

#endif // _FCD_GEOMETRY_POLYGONS_INPUT_H_
