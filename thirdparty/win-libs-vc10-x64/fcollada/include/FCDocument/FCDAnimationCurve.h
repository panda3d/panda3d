/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

/**
	@file FCDAnimationCurve.h
	This file contains the FCDAnimationCurve class and the conversion functions/functors.
*/

#ifndef _FCD_ANIMATION_CURVE_H_
#define _FCD_ANIMATION_CURVE_H_

#ifndef _FU_DAE_ENUM_H_
#include "FUtils/FUDaeEnum.h"
#endif // _FU_DAE_ENUM_H_
#ifndef __FCD_OBJECT_H_
#include "FCDocument/FCDObject.h"
#endif // __FCD_OBJECT_H_

class FCDAnimated;
class FCDAnimationClip;
class FCDAnimationChannel;
class FCDAnimationKey;
class FCDConversionFunctor;

typedef float (*FCDConversionFunction)(float v); /**< A simple conversion function. */

typedef fm::pvector<FCDAnimationClip> FCDAnimationClipList; /**< A dynamically-sized array of animation clips. */
typedef fm::pvector<FCDAnimationKey> FCDAnimationKeyList; /**< A dynamically-sized array of curve keys. */

/**
	A COLLADA single-dimensional animation curve.
	An animation curve holds the keyframes necessary
	to animate an animatable floating-point value.

	There are multiple interpolation mechanisms supported by COLLADA.
	FCollada supports the CONSTANT, LINEAR and BEZIER interpolations.

	@see FUDaeInterpolation FUDaeInfinity
	@ingroup FCDocument
*/
class FCOLLADA_EXPORT FCDAnimationCurve : public FCDObject
{
private:
	DeclareObjectType(FCDObject);

	// The channel owning this curve.
	FCDAnimationChannel* parent;

	// Targeting information
	int32 targetElement;
	fm::string targetQualifier;

	// Curve information
	FCDAnimationKeyList keys;
	FUDaeInfinity::Infinity preInfinity, postInfinity;
	
	// Driver information
	FUTrackedPtr<FCDAnimated> inputDriver;
	int32 inputDriverIndex;

	// Animation clips that depend on this curve
	FCDAnimationClipList clips;
	FloatList clipOffsets;
	FCDAnimationClip* currentClip;
	float currentOffset;
	static bool is2DEvaluation;

public:
	DeclareFlag(AnimChanged, 0);	// On Member Value Changed
	DeclareFlagCount(1);

public:
	/** Constructor: do not use directly.
		Instead, use the FCDAnimationChannel::AddCurve function.
		You should also attach the new curve to an animated
		element using the FCDAnimated::SetCurve function.
		@param document The COLLADA document that owns the animation curve.
		@param parent The animation channel that contains the curve. */
	FCDAnimationCurve(FCDocument* document, FCDAnimationChannel* parent);

	/** Destructor. */
	virtual ~FCDAnimationCurve();

	/** Retrieves the animation channel that contains this animation curve.
		@return The parent animation channel. */
	inline FCDAnimationChannel* GetParent() { return parent; }
	inline const FCDAnimationChannel* GetParent() const { return parent; } /**< See above. */

	/** Retrieves the list of keys for the animation curve.
		@return The list of keys. */
	inline FCDAnimationKey** GetKeys() { return keys.begin(); }
	inline const FCDAnimationKey** GetKeys() const { return keys.begin(); } /**< See above. */

	/** Retrieves the number of keys within the animation curve.
		@return The number of keys. */
	inline size_t GetKeyCount() const { return keys.size(); }

	/** Sets the number of keys within the animation curve.
		@param count The new number of keys in the curve.
		@param interpolation If creating new keys, the interpolation type
			for the new keys. */
	void SetKeyCount(size_t count, FUDaeInterpolation::Interpolation interpolation);

	/** Retrieves one key in the animation curve.
		@param index The index of the key to retrieve.
		@return The key. */
	inline FCDAnimationKey* GetKey(size_t index) { FUAssert(index < keys.size(), return NULL); return keys.at(index); }
	inline const FCDAnimationKey* GetKey(size_t index) const { FUAssert(index < keys.size(), return NULL); return keys.at(index); } /**< See above. */

	/** Appends a key to the animation curve.
		@param interpolation The interpolation type for the new key.
		@return The new key. */
	FCDAnimationKey* AddKey(FUDaeInterpolation::Interpolation interpolation);

	/** Adds a new key to the animation curve at the given time.
		@param interpolation The interpolation type for the new key.
		@param input The input (x) value of the new key.
		@return The new key. */
	FCDAnimationKey* AddKey(FUDaeInterpolation::Interpolation interpolation, float input) { size_t blah; return AddKey(interpolation, input, blah); }

	/** Adds a new key to the animation curve at the given time.
		@param interpolation The interpolation type for the new key.
		@param input The input (x) value of the new key.
		@param index [OUT] The index in the array of the new key
		@return The new key. */
	FCDAnimationKey* AddKey(FUDaeInterpolation::Interpolation interpolation, float input, size_t& index);

	/** Removes the given key from this curves list and deletes it
		@param key The key to find and delete
		@return True on success, false if the key is not found */
	bool DeleteKey(FCDAnimationKey* key);

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

	/** Retrieves whether this animation curve has a driver.
		@return Whether there is a driver for this curve. */
	bool HasDriver() const;

	/** Retrieves the value pointer that drives this animation curve.
		@param driver A reference to receive the animated input driver. This pointer will
			be set to NULL when there is no input driver.
		@param index A reference to receive the animated input driver element index. */
	void GetDriver(FCDAnimated*& driver, int32& index);
	void GetDriver(const FCDAnimated*& driver, int32& index) const; /**< See above. */
	FCDAnimated* GetDriverPtr(){ return inputDriver; }

	/** [INTERNAL] Retrieve the driver index.
		@return The driver index.
	*/
	int32 GetDriverIndex(){ return inputDriverIndex; }

	/** Sets the value pointer that drives the animation curve.
		@param driver The driver animated value. Set this pointer to NULL
			to indicate that time drives the animation curve.
		@param index The driver animated value index. */
	void SetDriver(FCDAnimated* driver, int32 index);

	/** Retrieves the number of animation clips that use this animation curve.
		@return The number of animation clips. */
	inline size_t GetClipCount() const { return clips.size(); }

	/** Retrieves the list of animation clips that use this animation curve.
		@return The list of animation clips. */
	inline FCDAnimationClip** GetClips() { return clips.begin(); }
	inline const FCDAnimationClip** GetClips() const { return clips.begin(); } /**< See above. */

	/** Retrieves an animation clips that use this animation curve.
		@param index The index of the animation clip.
		@return The animation clip at the given index in the list of clips that use this curve. */
	inline FCDAnimationClip* GetClip(size_t index) { FUAssert(index < clips.size(), return NULL); return clips[index]; }
	inline const FCDAnimationClip* GetClip(size_t index) const { FUAssert(index < clips.size(), return NULL); return clips[index]; } /**< See above. */

	/** Adds an animation clip to the list of animation clips that use this curve.
		@param clip An animation clip that uses this curve. */
	void AddClip(FCDAnimationClip* clip);

	/** Updates the keys to match the timing of an animation clip that has been
		registered using RegisterAnimationClip, or returned from GetClips.
		@param clip The clip to update the keys to. */
	void SetCurrentAnimationClip(FCDAnimationClip* clip);

	/** Gets the offset for an animation clip. When the offset is added to the
		keys, it causes the animation curve to be repositioned so that the 
		animation clip starts at the beginning. 
		@param index The index of the animation clip to get offset for.
		@return The offset value. */
	inline const float GetClipOffset(size_t index) const { return clipOffsets.at(index); }

	/** Clones the animation curve. The animation clips can be cloned as well,
		but this may lead to an infinite recursion because cloning the clips
		will also clone its curves.
		@param clone The cloned animation curve. If this pointer is NULL, a new
			animation curve will be created for you. You will then need to release
			the pointer.
		@param includeClips True if want to also clone the animation clips.
		@return The cloned animation curve. */
	FCDAnimationCurve* Clone(FCDAnimationCurve* clone = NULL, bool includeClips = true) const;

	/** Applies a conversion function to the key output values of the animation curve.
		@param valueConversion The conversion function to use on the key outputs.
		@param tangentConversion The conversion function to use on the key tangents. */
	void ConvertValues(FCDConversionFunction valueConversion, FCDConversionFunction tangentConversion);
	void ConvertValues(FCDConversionFunctor* valueConversion, FCDConversionFunctor* tangentConversion); /**< See above. */

	/** Applies a conversion function to the key input values of the animation curve.
		@param timeConversion The conversion function to use on the key inputs.
		@param tangentWeightConversion The conversion function to use on the key tangent weights. */
	void ConvertInputs(FCDConversionFunction timeConversion, FCDConversionFunction tangentWeightConversion);
	void ConvertInputs(FCDConversionFunctor* timeConversion, FCDConversionFunctor* tangentWeightConversion); /**< See above. */

	/** Evaluates the animation curve.
		@param input An input value.
		@return The sampled value of the curve at the given input value. */
	float Evaluate(float input) const;

	/** [INTERNAL] Adds an animation clip to the list of animation clips that use this curve.
		@param clip An animation clip. */
	void RegisterAnimationClip(FCDAnimationClip* clip);

	/** [INTERNAL] Retrieves the target element suffix for the curve.
		This will be -1 if the animated element does not belong to an
		animated element list.
		@return The target element suffix. */
	inline int32 GetTargetElement() const { return targetElement; }

	/** [INTERNAL] Retrieves the target qualifier for the curve.
		This will be the empty string if that the curve affects
		a one-dimensional animated element.
		@return The target qualifier. */
	inline const fm::string& GetTargetQualifier() const { return targetQualifier; }

	/** [INTERNAL] Sets the target element suffix for the curve.
		@param e The target element suffix. Set to value to -1
			if the animated element does not belong to an animated element list. */
	inline void SetTargetElement(int32 e) { targetElement = e; SetDirtyFlag(); }

	/** [INTERNAL] Sets the target qualifier for the curve.
		@param q The target qualifier. You may sets this string to the empty string
			only if that the curve affects a one-dimensional animated element. */
	inline void SetTargetQualifier(const fm::string& q) { targetQualifier = q; SetDirtyFlag(); }

	/** [INTERNAL] Updates the offset for a given animation clip.
		@param offset The new offset. 
		@param clip The animation clip to associate with the offset. */
	void SetClipOffset(float offset, const FCDAnimationClip* clip);

	/** Sets the 2D Curve Evaluation flag.
		This flag can be set by the user to enable the
		slower, but higher quality 2D evaluation. If the
		flag is negative, the faster 1D slope-based evaluation is
		performed. Defaults to true.
		@param flag Whether to enable 2D curve evaluation. */
	static void Set2DCurveEvaluation(bool flag) { is2DEvaluation = flag; }

	/** Returns whether 2D Curve Evaluation is on or off.
		@return A boolean that indicates if the 2D Curve Evaluation is on or off. */
	static bool Is2DCurveEvaluation() {return is2DEvaluation; }
};

/** A simple conversion functor. */
class FCDConversionFunctor
{
public:
	FCDConversionFunctor() {} /**< Constructor. */
	virtual ~FCDConversionFunctor() {} /**< Destructor. */
	virtual float operator() (float v) = 0; /**< Main functor to override. @param v The value to convert. @return The converted value. */
};

/** A sample conversion functor: it scales the value by a given amount. */
class FCDConversionScaleFunctor : public FCDConversionFunctor
{
private:
	float scaleFactor;

public:
	FCDConversionScaleFunctor(float factor) { scaleFactor = factor; } /**< Constructor. @param factor The scale factor. */
	void SetScaleFactor(float factor) { scaleFactor = factor; } /**< Mutator. @param factor The new scale factor. */
	virtual ~FCDConversionScaleFunctor() {} /**< Destructor. */
	virtual float operator() (float v) { return v * scaleFactor; } /**< Scales the given value. @param v The value to scale. @return The scaled value. */
	virtual FMVector3 operator() (const FMVector3& v) { return v * scaleFactor; } /**< Scales the given FMVector3. @param v The value to scale. @return The scaled value. */
	virtual FMVector4 operator() (const FMVector4& v) { return v * scaleFactor; } /**< Scales the given FMVector4. @param v The value to scale. @return The scaled value. */
};

/** A sample conversion functor: it offsets the value by a given amount. */
class FCDConversionOffsetFunctor : public FCDConversionFunctor
{
private:
	float offset;

public:
	FCDConversionOffsetFunctor(float _offset) { offset = _offset; } /**< Constructor. @param _offset The value offset. */
	virtual ~FCDConversionOffsetFunctor() {} /**< Destructor. */
	virtual float operator() (float v) { return v + offset; } /**< Offsets the given value. @param v The value to offset. @return The offseted value. */
};

#endif // _FCD_ANIMATION_CURVE_H_
