/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

/**
	@file FCDAnimationCurveTools.h
	This file contains the FCDAnimationCurveTools namespace.
*/

#ifndef _FCD_ANIMATION_CURVE_TOOLS_H_
#define _FCD_ANIMATION_CURVE_TOOLS_H_

class FCDAnimationCurve;
class FCDAnimationMultiCurve;

typedef fm::pvector<FCDAnimationCurve> FCDAnimationCurveList; /**< A dynamically-sized array of animation curves. */
typedef fm::pvector<const FCDAnimationCurve> FCDAnimationCurveConstList; /**< A dynamically-sized array of constant animation curve pointers. */
typedef float (*FCDCollapsingFunction)(float* values, uint32 count); /**< A collapsing function. It converts multiple floating-point values into one floating-point value. */

/** Contains tools to merge, collapse and otherwise modify animation curve. */
namespace FCDAnimationCurveTools
{
	/** Merges multiple single-dimensional animation curves into one
		multi-dimensional animation curve.
		For each NULL element found within the 'toMerge' list, the corresponding
		default value is used. If there are not enough default values provided, zero is assumed.
		The number of dimensions for the output animation curve is taken as the size of the 'toMerge' list.
		@param toMerge The list of single-dimensional animation curves to merge. This list may
			contain NULL elements, as explained above.
		@param defaultValues The list of default values to use when a NULL element is encountered.
			Default values should be provided even for the elements that are not NULL. */
	FCOLLADA_EXPORT FCDAnimationMultiCurve* MergeCurves(const FCDAnimationCurveConstList& toMerge, const FloatList& defaultValues);
	inline FCDAnimationMultiCurve* MergeCurves(const FCDAnimationCurveList& toMerge, const FloatList& defaultValues) { return MergeCurves(*(const FCDAnimationCurveConstList*) &toMerge, defaultValues); } /**< See above. */

	/** Collapses a multi-dimensional curve into a one-dimensional curve.
		@see Average TakeFirst
		@param curve The multi-dimensional curve to collapse.
		@param collapse The function to use to collapse multiple floating-point
			values into one. Set this to NULL to use the default collapsing
			function, which averages all the values. */
	FCOLLADA_EXPORT FCDAnimationCurve* Collapse(const FCDAnimationMultiCurve* curve, FCDCollapsingFunction collapse = NULL);

	/** Retrieves the first floating-point value of a list of floating-point values.
		This is a typical conversion function.
		@param values The list of floating-point values.
		@param count The number of values within the given list. */
	float TakeFirst(float* values, uint32 count);

	/** Retrieves the average value of a list of floating-point values.
		This is a typical conversion function.
		@param values The list of floating-point values.
		@param count The number of values within the given list. */
	float Average(float* values, uint32 count);
};

#endif // _FCD_ANIMATION_CURVE_TOOLS_H_

