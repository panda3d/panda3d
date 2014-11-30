/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

#ifndef _FCD_PARAMETER_ANIMATABLE_H_
#define _FCD_PARAMETER_ANIMATABLE_H_

/**
	@file FCDParameterAnimatable.h
	This file contains the FCDParameterAnimatable template and related functions.
*/

#ifndef _FU_PARAMETER_H_
#include <FUtils/FUParameter.h>
#endif // _FU_PARAMETER_H_
#ifndef _FM_ANGLEAXIS_H_
#include <FMath/FMAngleAxis.h>
#endif // _FM_ANGLEAXIS_H_
#ifndef _FM_LOOKAT_H_
#include <FMath/FMLookAt.h>
#endif // _FM_LOOKAT_H_
#ifndef _FM_SKEW_H_
#include <FMath/FMSkew.h>
#endif // _FM_SKEW_H_

class FCDAnimated;
class FUParameterizable;

/**
	An animatable value parameter.
	@ingroup FUParameter
*/
class FCOLLADA_EXPORT FCDParameterAnimatable
{
private:
	FUParameterizable* parent;
	FUObjectRef<FCDAnimated> animated;

public:
	/** Constructor.
		@param parent The FCollada object that owns this parameter. */
	FCDParameterAnimatable(FUParameterizable* parent);

	/** Destructor. */
	virtual ~FCDParameterAnimatable();

	/** Retrieves the parent object for this animatable parameter.
		@return The parent object of the parameter. */
	virtual FUParameterizable* GetParent() { return parent; }

	/** Retrieves the animation helper for this animatable parameter.
		This animation helper will be created if non-existant.
		@return The animation helper for this parameter. */
	FCDAnimated* GetAnimated();
	const FCDAnimated* GetAnimated() const; /**< See above. */

	/** Retrieves whether this animatable parameter has valid animation data.
		@return Whether this animatable parameter is animated. */
	bool IsAnimated() const;

	/** Copy operator.
		Note: the animation is not copied, right now.
		@param parameter The other parameter.
		@return This parameter. */
	FCDParameterAnimatable& operator= (FCDParameterAnimatable& parameter);

protected:
	/** Requests from the up-class that the correct FCDAnimated helper
		class be created for this parameter.
		This should be handled by the FCDParameterAnimatableT template specializations.
		@return A newly-created FCDAnimated helper object. */
	virtual FCDAnimated* CreateAnimated();
};

/**
	A typed animatable FCollada value parameter.
	@ingroup FUParameter
*/
template <class TYPE, int QUALIFIERS>
class FCOLLADA_EXPORT FCDParameterAnimatableT : public FCDParameterAnimatable
{
private:
	TYPE value;

public:
	/** Constructor.
		@param parent The FCollada object that owns this parameter. */
	FCDParameterAnimatableT(FUParameterizable* parent);

	/** Constructor.
		@param parent The FCollada object that owns this parameter.
		@param defaultValue A default value to assign to this parameter. */
	FCDParameterAnimatableT(FUParameterizable* parent, const TYPE& defaultValue);

	/** Destructor. */
	virtual ~FCDParameterAnimatableT();

	/** Retrieves the value or a reference to the value.
		@return The value or a reference to it. */
	inline operator TYPE&() { return value; }
	inline operator const TYPE&() const { return value; } /**< See above. */
	inline TYPE& operator *() { return value; } /**< See above. */
	inline const TYPE& operator *() const { return value; } /**< See above. */
	inline TYPE* operator->() { return &value; } /**< See above. */
	inline const TYPE* operator->() const { return &value; } /**< See above. */

	/** Assigns to this parameter the given value.
		@param copy A value of the correct type.
		@return A reference to this parameter. */
	FCDParameterAnimatableT<TYPE, QUALIFIERS>& operator= (const TYPE& copy);

protected:
	/** Created the correct FCDAnimated helper object, done by request
		from the parent class.
		@return A newly-created FCDAnimated helper object. */
	virtual FCDAnimated* CreateAnimated();
};

typedef FCDParameterAnimatableT<float, 0> FCDParameterAnimatableFloat; /**< A floating-point value animatable parameter. */
typedef FCDParameterAnimatableT<FMVector2, 0> FCDParameterAnimatableVector2; /**< A 2D vector animatable parameter. */
typedef FCDParameterAnimatableT<FMVector3, 0> FCDParameterAnimatableVector3; /**< A 3D vector animatable parameter. */
typedef FCDParameterAnimatableT<FMVector3, 1> FCDParameterAnimatableColor3; /**< A 3D vector animatable parameter. */
typedef FCDParameterAnimatableT<FMVector4, 0> FCDParameterAnimatableVector4; /**< A 4D vector animatable parameter. */
typedef FCDParameterAnimatableT<FMVector4, 1> FCDParameterAnimatableColor4; /**< A 4D color animatable parameter. */
typedef FCDParameterAnimatableT<FMMatrix44, 0> FCDParameterAnimatableMatrix44; /**< A matrix animatable parameter. */
typedef FCDParameterAnimatableT<FMAngleAxis, 0> FCDParameterAnimatableAngleAxis; /**< An angle-axis rotation animatable parameter. */
typedef FCDParameterAnimatableT<FMLookAt, 0> FCDParameterAnimatableLookAt; /**< A look-at animatable parameter. */
typedef FCDParameterAnimatableT<FMSkew, 0> FCDParameterAnimatableSkew; /**< A skew animatable parameter. */

/**
	An animatable value parameter.
	@ingroup FUParameter
*/
class FCOLLADA_EXPORT FCDParameterListAnimatable
{
private:
	FUParameterizable* parent;

protected:
	/** The list of FCDAnimated helpers.
		This value is marked "protected" because
		OnPotentialSizeChange uses it.. Needs some more thought? */
	FUObjectContainer<FCDAnimated> animateds;

public:
	/** Constructor.
		@param parent The FCollada object that owns this parameter. */
	FCDParameterListAnimatable(FUParameterizable* parent);

	/** Destructor. */
	virtual ~FCDParameterListAnimatable();

	/** Retrieves the parent object for this animatable parameter.
		@return The parent object of the parameter. */
	virtual FUParameterizable* GetParent() { return parent; }

	/** Retrieves the animation helper for an animatable value in this parameter.
		This animation helper will be created if non-existant.
		@param The index of the animatable value.
		@return The animation helper for the value at the given index. */
	FCDAnimated* GetAnimated(size_t index);
	const FCDAnimated* GetAnimated(size_t index) const; /**< See above. */

	/** Retrieves whether this animatable list parameter has valid animation data.
		@param The index of the animatable value within the list parameter.
			If this index is -1, then whether the whole list is returned.
		@return Whether this animatable value is animated. */
	bool IsAnimated(size_t index = ~(size_t)0) const;

	/** Retrieves the list of animated values for this list parameter.
		@return The list of animated values. */
	inline FUObjectContainer<FCDAnimated>& GetAnimatedValues() { return animateds; }
	inline const FUObjectContainer<FCDAnimated>& GetAnimatedValues() const { return animateds; } /**< See above. */

protected:
	/** Requests from the up-class that the correct FCDAnimated helper
		class be created for this parameter.
		This should be handled by the FCDParameterAnimatableT template specializations.
		@param index The element array index of the animatable value.
		@return A newly-created FCDAnimated helper object. */
	virtual FCDAnimated* CreateAnimated(size_t index);

	/** Process a list insertion event.
		The affected array element indices in the FCDAnimated helpers
		will be modified accordingly.
		@param offset The index of where the insertion was done.
		@param count The number of values inserted in the list. */
	void OnInsertion(size_t offset, size_t count);

	/** Process a list removal event.
		The affected FCDAnimated helpers will be released and
		the element indices of the other helpers will be modified accordingly.
		@param offset The index of where the removal was done.
		@param count The number of values removed from the list. */
	void OnRemoval(size_t offset, size_t count);

private:
	size_t BinarySearch(size_t arrayElementIndex) const;
};

/**
	A typed animatable FCollada value parameter.
	@ingroup FUParameter
*/
template <class TYPE, int QUALIFIERS>
class FCOLLADA_EXPORT FCDParameterListAnimatableT : public FCDParameterListAnimatable
{
private:
	fm::vector<TYPE, true> values;

public:
	/** Constructor.
		@param parent The FCollada object that owns this parameter. */
	FCDParameterListAnimatableT(FUParameterizable* parent);

	/** Destructor. */
	virtual ~FCDParameterListAnimatableT();

	/** Retrieves the number of values in this parameter list.
		@return The number of values. */
	inline size_t size() const { return values.size(); } /**< See above. */

	/** Retrieves whether this list parameter contains values.
		@return Whether the list parameter is empty. */
	inline bool empty() const { return values.size() == 0; }

	/** Sets the number of values contained in the list parameter.
		@param count The new number of values contained in the parameter. */
	void resize(size_t count);

	/** Sets the number of values contained in the list parameter.
		@param count The new number of values contained in the parameter.
		@param value The default value to insert in the new slots of the list parameter. */
	void resize(size_t count, const TYPE& value);

	/** Retrieves one value held by this parameter.
		Important: it is intentional that you cannot modify a list value using these function.
		Instead, use the set function.
		@param index The index of the value held by this parameter.
		@return The number of values in this parameter. */
	inline const TYPE& at(size_t index) const { return values[index]; }
	inline const TYPE& operator[](size_t index) const { return values[index]; } /**< See above. */
	template <class INTEGER> inline const TYPE& operator[](INTEGER index) const { return values[index]; } /**< See above. */

	/** Modifies one existing value held by this parameter.
		@param index The index of the value to change.
		@param value The new value. */
	void set(size_t index, const TYPE& value);

	/** Inserts a value to this parameter.
		@param index Where to insert the value.
		@param value The values to insert to this parameter. */
	void insert(size_t index, const TYPE& value);

	/** Inserts one value, multiple times, to this list parameter.
		@param index Where to insert the value.
		@param count The number of times to insert this value.
		@param value The value to insert to this parameter. */
	void insert(size_t index, size_t count, const TYPE& value);

	/** Inserts multiple values to this list parameter.
		@param index Where to insert the values.
		@param values A static list of values.
		@param count The number of values to insert. */
	void insert(size_t index, const TYPE* values, size_t count);

	/** Removes one element from this list parameter.
		@param index The index of the element to remove. */
	void erase(size_t index);

	/** Removes one element from this list parameter.
		@param value The value to remove. */
	void erase(const TYPE& value);

	/** Removes a contiguous series of elements from this list parameter.
		@param start The index of the first element to remove.
		@param end The index past the last element to remove. */
	void erase(size_t start, size_t end);

	/** Removes all the element of the list parameter. */
	void clear();

	/** Retrieves the index of an existing value in this parameter.
		@param value The value to look for.
		@return The index of the given value within the parameter.
			The size of the list is returned if the value is not found. */
	inline size_t find(const TYPE& value) const { return values.find(value) - values.begin(); }

	/** Retrieves whether the list parameter contains a specific value.
		@param value A value.
		@return Whether the given value is contained within the list parameter. */
	inline bool contains(const TYPE& value) const { return values.contains(value); }

	/** Appends one value to this parameter.
		@param value The value to add to this parameter. */
	void push_back(const TYPE& value);

	/** Adds one value at the beginning of this list parameter.
		@param value The value to add to this parameter. */
	void push_front(const TYPE& value);

	/** Removes the value at the end of the list parameter. */
	void pop_back();

	/** Removes the value at the end of the list parameter. */
	void pop_front();

	/** Retrieves the first element from this list parameter.
		@param The first element in the list parameter. */
	inline TYPE& front() { return values.front(); }
	inline const TYPE& front() const { return values.front(); } /**< See above. */

	/** Retrieves the last element from this list parameter.
		@param The last element in the list parameter. */
	inline TYPE& back() { return values.back(); }
	inline const TYPE& back() const { return values.back(); } /**< See above. */

	/** [INTERNAL] Retrieves a pointer to the source data. Not recommended.
		@return A pointer to the source data. */
	inline TYPE** GetDataPtr() { return values.GetDataPtr(); }
	inline const TYPE** GetDataPtr() const { return values.GetDataPtr(); } /**< See above. */

	/** [INTERNAL] Retrieves a reference to the inner value list. Not recommended.
		@return A reference to the value list. */
	inline fm::vector<TYPE, true>& GetDataList() { return values; }
	inline const fm::vector<TYPE, true>& GetDataList() const { return values; } /**< See above. */

	/** Retrieves the number of pre-allocated values reserved by this value list.
		@return The number of pre-allocated values reserved. */
	inline size_t capacity() const { return values.capacity(); }

protected:
	/** Created the correct FCDAnimated helper object, done by request
		from the parent class.
		@return A newly-created FCDAnimated helper object. */
	virtual FCDAnimated* CreateAnimated(size_t index);

private:
	/** Verifies whether a size change was triggered and
		if it is the case, forces the animateds to update their
		value pointers. */
	void OnPotentialSizeChange();
};

typedef FCDParameterListAnimatableT<float, 0> FCDParameterListAnimatableFloat; /**< A floating-point value animatable list parameter. */
typedef FCDParameterListAnimatableT<FMVector2, 0> FCDParameterListAnimatableVector2; /**< A 2D vector animatable list parameter. */
typedef FCDParameterListAnimatableT<FMVector3, 0> FCDParameterListAnimatableVector3; /**< A 3D vector animatable list parameter. */
typedef FCDParameterListAnimatableT<FMVector3, 1> FCDParameterListAnimatableColor3; /**< A 3D color animatable list parameter. */
typedef FCDParameterListAnimatableT<FMVector4, 0> FCDParameterListAnimatableVector4; /**< A 4D vector animatable list parameter. */
typedef FCDParameterListAnimatableT<FMVector4, 1> FCDParameterListAnimatableColor4; /**< A 4D color animatable list parameter. */

/** Declares an animatable parameter for the objects of the current class.
    Use this macro within a class declaration to add an animatable parameter to it.
    The different versions of this macro are used for different parameter types:

    For floats, Booleans, vectors and strings, use DeclareParameterAnimatable.
    For arrays of floats, Booleans, vectors and strings, use DeclareParameterListAnimatable.
    For tracked pointers, use DeclareParameterPtr.
    For contained pointers, use DeclareParameterRef.
    For tracked lists of objects, use DeclareParameterTrackList.
    For containers of objects, use DeclareParameterContainer.
    
    @param type The value type of the animatable parameter. Ex: float, bool, FMVector4 or string.
    @param qual The qualifier for the animatable parameter type. @see FUParameterQualifiers::Qualifiers.
    @param parameterName The member variable name for the animatable parameter.
    @param niceName The Unicode string to identify this animatable parameter to users. */
#define DeclareParameterAnimatable(type, qual, parameterName, niceName) \
	class Parameter_##parameterName : public FCDParameterAnimatableT<type, qual> { \
	public: Parameter_##parameterName(FUParameterizable* parent) : FCDParameterAnimatableT<type, qual>(parent) {} \
	Parameter_##parameterName(FUParameterizable* parent, const type& defaultValue) : FCDParameterAnimatableT<type, qual>(parent, defaultValue) {} \
	virtual ~Parameter_##parameterName() {} \
	Parameter_##parameterName& operator= (const type& copy) { FCDParameterAnimatableT<type, qual>::operator=(copy); return *this; } \
	} parameterName;

/** See above. */
#define DeclareParameterListAnimatable(type, qual, parameterName, niceName) \
	class Parameter_##parameterName : public FCDParameterListAnimatableT<type, qual> { \
	public: Parameter_##parameterName(FUParameterizable* parent) : FCDParameterListAnimatableT<type, qual>(parent) {} \
	virtual ~Parameter_##parameterName() {} \
	} parameterName;

/** Initializes the member variable for an animatable parameter.
    Either one of these macros must be included for each member animatable
    parameter of a class, within that class' constructors.
    The different versions of this macro are used for different contexts:
    
    For a parameter where the default constructor is to be used, use InitializeParameterAnimatableNoArg.
    For a parameter where one or more values should be used in the constructor, use InitializeParameterAnimatable.

    @param parameterName The member variable name of the parameter within the class. */
#define InitializeParameterAnimatableNoArg(parameterName) parameterName(this)
/** See above. */
#define InitializeParameterAnimatable(parameterName, ...) parameterName(this, __VA_ARGS__)

#if defined(__APPLE__) || defined(LINUX)
#include <FCDocument/FCDParameterAnimatable.hpp>
#endif // MAC + LINUX

#endif // _FCD_PARAMETER_ANIMATABLE_H

