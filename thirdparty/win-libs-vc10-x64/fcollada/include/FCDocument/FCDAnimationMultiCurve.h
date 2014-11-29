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
	@file FCDAnimationMultiCurve.h
	This file contains the FCDAnimationMultiCurve class.
*/

#ifndef _FCD_ANIMATION_MULTI_CURVE_H_
#define _FCD_ANIMATION_MULTI_CURVE_H_

#ifndef __FCD_OBJECT_H_
#include "FCDocument/FCDObject.h"
#endif // __FCD_OBJECT_H_
#ifndef _FU_DAE_ENUM_H_
#include "FUtils/FUDaeEnum.h"
#endif // _FU_DAE_ENUM_H_

class FCDocument;
class FCDAnimationMKey;

typedef fm::pvector<FCDAnimationMKey> FCDAnimationMKeyList; /**< A dynamically-sized array of multi-dimensional animation keys. */

/**
	A COLLADA multi-dimensional animation curve.

	This is a utility class that is used to convert multiple
	animation curves into one animation curve that has multiple
	dimensions, but only one list of key inputs.

	FCollada will never create a multi-dimensional animation curve
	during the import of a COLLADA document.

	@ingroup FCDocument
*/
class FCOLLADA_EXPORT FCDAnimationMultiCurve : public FCDObject
{
private:
	DeclareObjectType(FCDObject);

	// The number of merged curves
	uint32 dimension;

	// Target information
	int32 targetElement;

	// Input information
	FCDAnimationMKeyList keys;
	FUDaeInfinity::Infinity preInfinity, postInfinity;

	//What sort of evaluation we do, 1D or 2D
	static bool is2DEvaluation;

public:
	/** Constructor.
		The number of dimensions will not change in the lifetime of a
		multi-dimensional curve.
		@param document The COLLADA document that owns the animation curve.
		@param dimension The number of dimensions for the animation curve. */
	FCDAnimationMultiCurve(FCDocument* document, uint32 dimension);

	/** Destructor. */
	virtual ~FCDAnimationMultiCurve();

	/** Retrieves the number of dimensions for the curve.
		@return The number of dimensions for the curve. */
	inline uint32 GetDimension() const { return dimension; }

	/** Retrieves the list of key inputs for the animation curve.
		@return The list of key inputs. */
	inline FCDAnimationMKey** GetKeys() { return keys.begin(); }
	inline const FCDAnimationMKey** GetKeys() const { return keys.begin(); } /**< See above. */

	/** Retrieves the number of keys within the animation curve.
		@return The number of keys. */
	inline size_t GetKeyCount() const { return keys.size(); }

	/** Sets the number of keys within the animation curve.
		@param count The new number of keys in the curve.
		@param interpolation If creating new keys, the interpolation type
			for the new keys. */
	void SetKeyCount(size_t count, FUDaeInterpolation::Interpolation interpolation);

	/** Retrieve one key of the animation curve.
		@param index The index of the key to retrieve.
		@return The indexed key. */
	inline FCDAnimationMKey* GetKey(size_t index) { FUAssert(index < keys.size(), return NULL); return keys.at(index); }
	inline const FCDAnimationMKey* GetKey(size_t index) const { FUAssert(index < keys.size(), return NULL); return keys.at(index); } /**< See above. */

	/** Appends a key to the animation curve.
		@param interpolation The interpolation type for the new key.
		@return The new key. */
	FCDAnimationMKey* AddKey(FUDaeInterpolation::Interpolation interpolation);

	/** Retrieves the type of behavior for the curve if the input value is
		outside the input interval defined by the curve keys and less than any key input value.
		@see FUDaeInfinity
		@return The pre-infinity behavior of the curve. */
	inline FUDaeInfinity::Infinity GetPreInfinity() const { return preInfinity; }

	/** Sets the behavior of the curve if the input value is
		outside the input interval defined by the curve keys and less than any key input value.
		@see FUDaeInfinity
		@param infinity The pre-infinity behavior of the curve. */
	inline void SetPreInfinity(FUDaeInfinity::Infinity infinity) { preInfinity = infinity; SetDirtyFlag(); }

	/** Retrieves the type of behavior for the curve if the input value is
		outside the input interval defined by the curve keys and greater than any key input value.
		@see FUDaeInfinity
		@return The post-infinity behavior of the curve. */
	inline FUDaeInfinity::Infinity GetPostInfinity() const { return postInfinity; }

	/** Sets the behavior of the curve if the input value is
		outside the input interval defined by the curve keys and greater than any key input value.
		@see FUDaeInfinity
		@param infinity The post-infinity behavior of the curve. */
	inline void SetPostInfinity(FUDaeInfinity::Infinity infinity) { postInfinity = infinity; SetDirtyFlag(); }

	/** Evaluates the animation curve.
		@param input An input value.
		@param output An array of floating-point values to fill in with the sampled values. */
	void Evaluate(float input, float* output) const;

	/** [INTERNAL] Retrieves the target element suffix for the curve.
		This will be -1 if the animated element does not belong to an
		animated element list.
		@return The target element suffix. */
	inline int32 GetTargetElement() const { return targetElement; }

	/** [INTERNAL] Sets the target element suffix for the curve.
		@param e The target element suffix. Set to value to -1
			if the animated element does not belong to an animated element list. */
	inline void SetTargetElement(int32 e) { targetElement = e; SetDirtyFlag(); }
	inline int32 GetTargetElement() { return targetElement; }

	/** [INTERNAL] Sets the target qualifier for the merged curve.
		Target qualifiers are transient information useful when exporting animation curves.
		@param index The dimension index of the target qualifier to modify.
		@param qualifier The new target qualifier. */
	//inline void SetTargetQualifier(size_t index, const fm::string& qualifier) { FUAssert(index < dimension, return); targetQualifiers[index] = qualifier; }
	//inline fm::string& GetTargetQualifier(size_t index) { FUAssert(index < dimension,); return targetQualifiers[index]; }

	/** Turns on or off the 2D Curve Evaluation.
		@param flag An on or off boolean flag.*/
	inline void Set2DCurveEvaluation(bool flag) { is2DEvaluation = flag; }

	/** Returns whether 2D Curve Evaluation is on or off.
		@return A boolean that indicates if the 2D Curve Evaluation is on or off. */
	inline bool Is2DCurveEvaluation() {return is2DEvaluation; }
};

#endif // _FCD_ANIMATION_MULTI_CURVE_H_
