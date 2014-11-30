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
	@file FCDAnimated.h
	This file contains the FCDAnimated class.
*/

#ifndef _FCD_ANIMATED_H_
#define _FCD_ANIMATED_H_

#ifndef __FCD_OBJECT_H_
#include "FCDocument/FCDObject.h"
#endif // __FCD_OBJECT_H_

class FCDocument;
class FCDAnimated;
class FCDAnimationCurve;
class FCDAnimationChannel;
class FCDAnimationMultiCurve;

typedef fm::pvector<float> FloatPtrList; /**< A dynamically-sized array of floating-point value pointers. */
typedef fm::pvector<FCDAnimationCurve> FCDAnimationCurveList; /**< A dynamically-sized array of animation curves. */
typedef FUTrackedList<FCDAnimationCurve> FCDAnimationCurveTrackList; /**< A dynamically-sized array of tracked animation curves. */
typedef fm::vector<FCDAnimationCurveTrackList> FCDAnimationCurveListList; /**< A dynamically-sized array of animation curve lists. */
typedef fm::pvector<FCDAnimationChannel> FCDAnimationChannelList; /**< A dynamically-sized array of animation channels. */
typedef fm::pvector<FCDAnimated> FCDAnimatedList; /**< A dynamically-sized array of animated values. */

/**
	An animated element.
	An animated element encapsulates a set of floating-point values that are
	marked as animated.

	For this purpose, an animated element holds a list of floating-point values,
	their animation curves and their COLLADA qualifiers for the generation of
	COLLADA targets. For animated list elements, an animated element holds an array index.

	There are many classes built on top of this class. They represent
	the different element types that may be animated, such as 3D points,
	colors and matrices.

	@ingroup FCDocument
*/
class FCOLLADA_EXPORT FCDAnimated : public FCDObject, FUTracker
{
private:
	DeclareObjectType(FCDObject);

protected:
	/** The list of value pointers. */
	FloatPtrList values;

	/** The list of target qualifiers.
		There is always one qualifier for one value pointer. */
	StringList qualifiers; 

	/** The list of animation curves.
		There is always one curve for one value pointer, although
		that curve may be the NULL pointer to indicate a non-animated value. */
	FCDAnimationCurveListList curves;

	/** The target object who contain the float values.*/
	FCDObject* target;

	/** The array index for animated element that belong
		to a list of animated elements. This value may be -1
		to indicate that the element does not belong to a list.
		Otherwise, the index should always be unsigned. */
	int32 arrayElement;

public:
	DeclareFlag(RelativeAnimation, 0); /**< Flag to indicate that the animation should be applied relative to the default value. */
	DeclareFlagCount(1); /**< The FCDAnimated class declares one flag. */

public:
	/** Constructor.
		@param object The FCollada object that owns this animated element.
		@param valueCount The number of values inside the animated element.
		@param qualifiers A constant array of UTF8 string defining the qualifier for each value.
			You should check out the arrays in FUDaeAccessor for examples.
		@param values A constant array containing the value pointers. */
	FCDAnimated(FCDObject* object, size_t valueCount, const char** qualifiers, float** values);

	/** Constructor.
		@param document The COLLADA document that owns this animated element.
		@param valueCount The number of values inside the animated element.
		@param qualifiers A constant array of UTF8 string defining the qualifier for each value.
			You should check out the arrays in FUDaeAccessor for examples.
		@param values A constant array containing the value pointers. */
	FCDAnimated(FCDocument* document, size_t valueCount, const char** qualifiers, float** values);

	/** Destructor. */
	virtual ~FCDAnimated();

	/** Retrieves the number of values contained within this animated element.
		@return The number of values. */
	inline size_t GetValueCount() const { return values.size(); }

    /** Retrieves the number of animation curves affecting one value of an animated element.
        @param index The value index.
        @return The number of curves affecting this value. */
    inline size_t GetCurveCount(size_t index) const { FUAssert(index < GetValueCount(), return 0); return curves[index].size(); }

	/** Retrieves the animation curve affecting the value of an animated element.
		@param index The value index.
		@param curveIndex The index of the curve within the list of curves affecting the
			value at the given index.
		@return The curve affecting the value at the given index. This pointer will
			be NULL if one of the index is out-of-bounds or if the value is not animated. */
	inline FCDAnimationCurve* GetCurve(size_t index, size_t curveIndex = 0) { FUAssert(index < GetValueCount(), return NULL); return curveIndex < curves.at(index).size() ? curves.at(index).at(curveIndex) : NULL; }
	inline const FCDAnimationCurve* GetCurve(size_t index, size_t curveIndex = 0) const { FUAssert(index < GetValueCount(), return NULL); return curveIndex < curves.at(index).size() ? curves.at(index).at(curveIndex) : NULL; } /**< See above. */

	/** Retrieves the list of the curves affecting the values of an animated element.
		This list may contain the NULL pointer, where a value is not animated.
		@return The list of animation curves. */
	inline FCDAnimationCurveListList& GetCurves() { return curves; }
	inline const FCDAnimationCurveListList& GetCurves() const { return curves; } /**< See above. */

	/** Assigns a curve to a value of the animated element.
		The previously assigned curve will be deleted.
		@param index The value index.
		@param curve The new curve(s) that will affect the value at the given index.
		@return Whether the curve was successfully assigned. Will return false if
			the index is out-of-bounds. */
	bool AddCurve(size_t index, FCDAnimationCurve* curve);
	bool AddCurve(size_t index, FCDAnimationCurveList& curve); /**< See above. */

	/** Removes the curves affecting a value of the animated element.
		@param index The value index.
		@return Whether a curve was successfully removed. Will return false
			if there was no curve to release or the index is out-of-bounds. */
	bool RemoveCurve(size_t index);

	/** Retrieves the value of an animated element.
		@param index The value index.
		@return The value at the given index. This pointer will
			be NULL if the index is out-of-boudns. */
	inline float* GetValue(size_t index) { FUAssert(index < GetValueCount(), return NULL); return values.at(index); }
	inline const float* GetValue(size_t index) const { FUAssert(index < GetValueCount(), return NULL); return values.at(index); } /**< See above. */

	/** [INTERNAL] Overwrites the value pointer of an animated element.
		Used when changing the list size within FCDParameterAnimatableList.
		@param index The value index.
		@param value The new value pointer for this index. */
	inline void SetValue(size_t index, float* value) { FUAssert(index < GetValueCount(), return); values.at(index) = value; }

	/** Retrieves the qualifier of the value of an animated element.
		@param index The value index.
		@return The qualifier for the value. The value returned will be an
			empty string when the index is out-of-bounds. */
	const fm::string& GetQualifier(size_t index) const;

	/** [INTERNAL] Retrieve the qualifier list directly.
		@return The reference to the qualifier list.
	*/
	StringList& GetQualifiers(){ return qualifiers; }

	/** Retrieves an animated value given a valid qualifier.
		@param qualifier A valid qualifier.
		@return The animated value for this qualifier. This pointer will be
			NULL if the given qualifier is not used within this animated element. */
	float* FindValue(const fm::string& qualifier);
	const float* FindValue(const fm::string& qualifier) const; /**< See above. */

	/** Retrieves an animation curve given a valid qualifier.
		@param qualifier A valid qualifier.
		@return The animation curve for this qualifier. This pointer will be
			NULL if the given qualifier is not used within this animated element
			or if the value for the given qualifier is not animated. */
	inline FCDAnimationCurve* FindCurve(const char* qualifier) { size_t index = FindQualifier(qualifier); return index < GetValueCount() ? GetCurve(index) : NULL; }
	inline FCDAnimationCurve* FindCurve(const fm::string& qualifier) { return FindCurve(qualifier.c_str()); } /**< See above. */
	inline const FCDAnimationCurve* FindCurve(const char* qualifier) const { size_t index = FindQualifier(qualifier); return index < GetValueCount() ? GetCurve(index) : NULL; } /**< See above. */
	inline const FCDAnimationCurve* FindCurve(const fm::string& qualifier) const { return FindCurve(qualifier.c_str()); } /**< See above. */

	/** Retrieves an animation curve given a value pointer.
		@param value A value pointer contained within the animated element.
		@return The animation curve for this qualifier. This pointer will be
			NULL if the value pointer is not contained by this animated element
			or if the value is not animated. */
	inline FCDAnimationCurve* FindCurve(const float* value) { size_t index = FindValue(value); return index < GetValueCount() ? GetCurve(index) : NULL; }
	inline const FCDAnimationCurve* FindCurve(const float* value) const { size_t index = FindValue(value); return index < GetValueCount() ? GetCurve(index) : NULL; } /**< See above. */

	/** Retrieves the value index for a given qualifier.
		@param qualifier A valid qualifier.
		@return The value index. This value will be -1 to indicate that the
			qualifier does not belong to this animated element. */
	size_t FindQualifier(const char* qualifier) const;
	inline size_t FindQualifier(const fm::string& qualifier) const { return FindQualifier(qualifier.c_str()); } /**< See above. */

	/** Retrieves the value index for a given value pointer.
		@param value A value pointer contained within the animated element.
		@return The value index. This value will be -1 to indicate that the
			value pointer is not contained by this animated element. */
	size_t FindValue(const float* value) const;

	/** Sets the FCDObject this animated value will notify when making changes
		@param _target Pointer to the FCDObject we will notify*/
	void SetTargetObject(FCDObject* _target);

	/** Returns the FCDObject this animated value will notify when making changes
		@return Pointer to the FCDObject we notify*/
	FCDObject* GetTargetObject() { return target; } 

	/** Retrieves the array index for an animated element.
		This value is used only for animated elements that belong
		to a list of animated elements within the COLLADA document.
		@return The array index. This value will be -1 to indicate that
			the animated element does not belong to a list. */
	inline int32 GetArrayElement() const { return arrayElement; }

	/** Sets the array index for an animated element.
		This value is used only for animated elements that belong
		to a list of animated elements within the COLLADA document.
		@param index The array index. This value should be -1 to indicate that
			the animated element does not belong to a list. */
	inline void SetArrayElement(int32 index) { arrayElement = index; SetDirtyFlag(); }

	/** Retrieves whether this animated element has any animation curves
		affecting its values.
		@return Whether any curves affect this animated element. */
	bool HasCurve() const;

	/** Creates one multi-dimensional animation curve from this animated element.
		This function is useful is your application does not handle animations
		per-values, but instead needs one animation per-element.
		@return The multi-dimensional animation curve. */
	FCDAnimationMultiCurve* CreateMultiCurve() const;

	/** Creates one multi-dimensional animation curve from a list of animated element.
		This function is useful if your application does not handle animations
		per-values. For example, we use this function is ColladaMax for animated scale values,
		where one scale value is two rotations for the scale rotation pivot and one
		3D point for the scale factors.
		@param toMerge The list of animated elements to merge
		@return The multi-dimensional animation curve. */
	static FCDAnimationMultiCurve* CreateMultiCurve(const FCDAnimatedList& toMerge);

	/** Evaluates the animated element at a given time.
		This function directly and <b>permanently</b> modifies the values
		of the animated element according to the curves affecting them.
		@param time The evaluation time. */
	void Evaluate(float time);

	/** Clones an animated element.
		@param document The COLLADA document that owns the cloned animated element.
		@return The cloned animated element. */
	FCDAnimated* Clone(FCDocument* document) const;

	/** Clones an animated element.
		@param clone An animated element to be the clone of this element.
		@return The clone. */
	FCDAnimated* Clone(FCDAnimated* clone) const;

	/** [INTERNAL]  See FUTracker
		On object deletion, remove ourselves as we no longer have any values to animate */
	virtual void OnObjectReleased(FUTrackable* object);
};

/** A COLLADA custom animated value.
	Used for animated extra elements. A single value is used multiple times to hold
	as many value pointers are necessary to hold the animation curves.
	@ingroup FCDocument */
class FCOLLADA_EXPORT FCDAnimatedCustom : public FCDAnimated
{
private:
	DeclareObjectType(FCDAnimated);
	float dummy;

	bool Link(xmlNode* node);

public:
	/** [INTERNAL]
        Don't build directly.
        @param object The object that owns this animated value. */
	FCDAnimatedCustom(FCDObject* object);

	/** [INTERNAL] Initialized a custom animated element from
		another animated element. The custom animated element will
		be resized to copy the given animated element.
		@param copy The animated element to copy. */
	void Copy(const FCDAnimated* copy);

	/** Retrieves the floating-point value used for all the value pointers.
		@return The dummy floating-point value. */
	float& GetDummy() { return dummy; }
	const float& GetDummy() const { return dummy; } /**< See above. */

	/** Resizes the wanted qualifiers.
		Using the FUDaeAccessor types or the FCDAnimatedStandardQualifiers types is recommended.
		@param count The new size of the animated element.
		@param qualifiers The new qualifiers for the animated element.
		@param prependDot Whether to prepend the '.' character for all the qualifiers of the animated element. */
	void Resize(size_t count, const char** qualifiers = NULL, bool prependDot = true);
    
	/** Resizes the wanted qualifiers.
		@param qualifiers The new qualifiers for the animated element.
		@param prependDot Whether to prepend the '.' character for all the qualifiers of the animated element. */
	void Resize(const StringList& qualifiers = NULL, bool prependDot = true);
};

/** The common qualifier lists. */
namespace FCDAnimatedStandardQualifiers
{
	/** Common accessor type string arrays.
		These are NULL-terminated and can be used with the AddAccessor function. */
	FCOLLADA_EXPORT extern const char* EMPTY[1]; /**< Used for qualifying single values. */
	FCOLLADA_EXPORT extern const char* XYZW[4]; /**< Used for position and vector values. */
	FCOLLADA_EXPORT extern const char* RGBA[4]; /**< Used for color value. */

	FCOLLADA_EXPORT extern const char* ROTATE_AXIS[4]; /**< Used for angle-axis rotation transforms. */
	FCOLLADA_EXPORT extern const char* SKEW[7]; /**< Used for the skew transforms. */
	FCOLLADA_EXPORT extern const char* MATRIX[16]; /**< Used for animating matrix values. */
	FCOLLADA_EXPORT extern const char* LOOKAT[9]; /**< Used for the look-at transforms. */
};

#endif // _FCD_ANIMATED_H_

