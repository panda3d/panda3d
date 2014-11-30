/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

/**
	@file FCDGeometrySource.h
	This file contains the FCDGeometrySource class.
*/

#ifndef _FCD_GEOMETRY_SOURCE_H_
#define _FCD_GEOMETRY_SOURCE_H_

#ifndef __FCD_OBJECT_WITH_ID_H_
#include "FCDocument/FCDObjectWithId.h"
#endif // __FCD_OBJECT_WITH_ID_H_
#ifndef _FU_DAE_ENUM_H_
#include "FUtils/FUDaeEnum.h"
#endif // _FU_DAE_ENUM_H_
#ifndef _FCD_PARAMETER_ANIMATABLE_H_
#include "FCDocument/FCDParameterAnimatable.h"
#endif // _FCD_PARAMETER_ANIMATABLE_H_

class FCDExtra;

/**
	A COLLADA data source for geometric meshes.

	A COLLADA data source for geometric meshes contains a list of floating-point values and the information
	to parse these floating-point values into meaningful content: the stride of the list and the type of data
	that the floating-point values represent. When the floating-point values are split according to the stride,
	you get the individual source values of the given type. A data source may also have a user-generated name to
	identify the data within. The name is optional and is used to keep
	around the user-friendly name for texture coordinate sets or color sets.

	Each source values of the COLLADA data source may be animated individually, or together: as an element.

	@ingroup FCDGeometry
*/
class FCOLLADA_EXPORT FCDGeometrySource : public FCDObjectWithId
{
private:
	DeclareObjectType(FCDObjectWithId);
	DeclareParameter(fstring, FUParameterQualifiers::SIMPLE, name, FC("Name"));
	DeclareParameterListAnimatable(float, FUParameterQualifiers::SIMPLE, sourceData, FC("Data"))
	DeclareParameter(uint32, FUParameterQualifiers::SIMPLE, stride, FC("Stride"));
	DeclareParameter(uint32, FUParameterQualifiers::SIMPLE, sourceType, FC("Value Type")); // FUDaeGeometryInput::Semantic sourceType;
	DeclareParameterRef(FCDExtra, extra, FC("Extra Tree"));

public:
	/** Constructor: do not use directly.
		Use FCDGeometryMesh::AddSource or FCDGeometryMesh::AddValueSource instead.
		@param document The COLLADA document which owns the data source. */
	FCDGeometrySource(FCDocument* document);

	/** Destructor. */
	virtual ~FCDGeometrySource();

	/** Copies the data source into a clone.
		The clone may reside in another document.
		@param clone The empty clone. If this pointer is NULL, a new data source
			will be created and you will need to release the returned pointer manually.
		@return The clone. */
	FCDGeometrySource* Clone(FCDGeometrySource* clone = NULL) const;

	/** Retrieves the name of the data source. The name is optional and is used to
		keep around a user-friendly name for texture coordinate sets or color sets.
		@return The name of the data source. */
	inline const fstring& GetName() const { return name; }

	/** Retrieves the pure data of the data source. This is a dynamically-sized array of
		floating-point values that contains all the data of the source.
		@return The pure data of the data source. */
	inline float* GetData() { return !sourceData.empty() ? &sourceData.front() : NULL; }
	inline const float* GetData() const { return !sourceData.empty() ? &sourceData.front() : NULL; } /**< See above. */

	/** [INTERNAL] Retrieve the reference to the source data.
		@return The reference to the source data.
	*/
	inline FCDParameterListAnimatableFloat& GetSourceData(){ return sourceData; }
	inline const FCDParameterListAnimatableFloat& GetSourceData() const { return sourceData; }

	/** Retrieves a ptr to the data of the data source. This allows external objects to
		store pointers to our data even when the data memory is reallocated
		@return The ptr to the pure data of the data source. */
	inline float** GetDataPtr() { return (float**) sourceData.GetDataPtr(); }
	inline const float** GetDataPtr() const { return (const float**) sourceData.GetDataPtr(); } /**< See above. */

	/** Retrieves the amount of data inside the source.
		@return The number of data entries in the source. */
	inline size_t GetDataCount() const { return sourceData.size(); }

	/** Sets the amount of data contained inside the source.
		It is preferable to set the stride and to use SetValueCount.
		No initialization of new values is done.
		@param count The amount of data for the source to contain. */
	void SetDataCount(size_t count);

	/** Retrieves the stride of the data within the source.
		There is no guarantee that the number of data values within the source is a multiple of the stride,
		yet you should always verify that the stride is at least the wanted dimension. For example, there is
		no guarantee that your vertex position data source has a stride of 3. 3dsMax is known to always
		export 3D texture coordinate positions.
		@return The stride of the data. */
	inline uint32 GetStride() const { return stride; }

	/** Retrieves the number of individual source values contained in the source.
		@return The number of source values. */
	inline size_t GetValueCount() const { return sourceData.size() / stride; }

	/** Retrieves the max number of values this input can handle before memory is reallocated.
		@return The number of source values. */
	inline size_t GetValueReserved() const { return sourceData.capacity() / stride; }

	/** Sets the number of individual source values contained in the source.
		No initialization of new values is done.
		@param count The number of individual source values to contain in this source. */
	inline void SetValueCount(size_t count) { FUAssert(stride > 0, return); SetDataCount(count * stride); }

	/** Retrieves one source value out of this source.
		@param index The index of the source value.
		@return The source value. */
	inline const float* GetValue(size_t index) const { FUAssert(index < GetValueCount(), return NULL); return &sourceData.at(index * stride); } /**< See above. */

	/** Sets one source value out of this source.
		@param index The index of the source value.
		@param value The new value. */
	inline void SetValue(size_t index, const float* value) { FUAssert(index < GetValueCount(), return); for (size_t i = 0; i < stride; ++i) sourceData.set(stride * index + i, value[i]); }

	/** Retrieves the type of data contained within the source.
		Common values for the type of data are POSITION, NORMAL, COLOR and TEXCOORD.
		Please see FUDaeGeometryInput for more information.
		@see FUDaeGeometryInput.
		@return The type of data contained within the source. */
	inline FUDaeGeometryInput::Semantic GetType() const { return (FUDaeGeometryInput::Semantic) *sourceType; }

	/** Sets the type of data contained within the source.
		Modifying the source type of an existing source is not recommended.
		Common values for the type of data are POSITION, NORMAL, COLOR and TEXCOORD.
		Please see FUDaeGeometryInput for more information.
		@see FUDaeGeometryInput.
		@param type The type of data to be contained within the source. */
	inline void SetType(FUDaeGeometryInput::Semantic type) { sourceType = type; }

	/** Retrieves the list of animated values for the data of the source.
		@return The list of animated values. */
	inline FUObjectContainer<FCDAnimated>& GetAnimatedValues() { return sourceData.GetAnimatedValues(); }
	inline const FUObjectContainer<FCDAnimated>& GetAnimatedValues() const { return sourceData.GetAnimatedValues(); } /**< See above. */

	/** Sets the user-friendly name of the data source. The name is optional and is used to
		keep around a user-friendly name for texture coordinate sets or color sets.
		@param _name The user-friendly name of the data source. */		
	inline void SetName(const fstring& _name) { name = _name; SetDirtyFlag(); }

	/** Overwrites the data contained within the data source.
		@param _sourceData The new data for this source.
		@param _sourceStride The stride for the new data.
		@param offset The offset at which to start retrieving the new data.
			This argument defaults at 0 to indicate that the data copy should start from the beginning.
		@param count The number of data entries to copy into the data source.
			This argument defaults at 0 to indicate that the data copy should include everything. */
	void SetData(const FloatList& _sourceData, uint32 _sourceStride, size_t count=0, size_t offset=0);

	/** Sets the stride for the source data.
		@param _stride The stride for the source data. */
	inline void SetStride(uint32 _stride) { stride = _stride; SetDirtyFlag(); }

	/** [INTERNAL] Set the source type.
		@param type The source type. */
	void SetSourceType(FUDaeGeometryInput::Semantic type) { sourceType = type; }

	/** Retrieves the extra information contained by this data source.
		@return The extra tree. This pointer will be NULL,
			in the const-version of this function, if there is no extra information.
			In the modifiable-version of this function:
			you will always get a valid extra tree that you can fill in. */
	FCDExtra* GetExtra();
	inline const FCDExtra* GetExtra() const { return extra; } /**< See above. */

	/** [INTERNAL] Clones this data source. You will need to release the returned pointer manually.
		@return An identical copy of the data source. */
	FCDGeometrySource* Clone() const;
};

#endif // _FCD_GEOMETRY_SOURCE_H_
