/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

#ifndef _FU_PARAMETER_H_
#define _FU_PARAMETER_H_

/**
	@file FUParameter.h
	This file contains the FUParameter interface and its many sub-interfaces.
*/

/** @defgroup FUParameter The generic FCollada parameter interface. */

#ifdef WIN32
#pragma warning(disable:4355) // 'this' : used in base member initializer list.
#endif // WIN32

/** Contains the supported qualifiers for parameter types. */
namespace FUParameterQualifiers
{
    /** Qualifiers for parameters types. */
	enum Qualifiers
	{
		SIMPLE = 0, /**< Qualifies any self-defined parameter type. */
		VECTOR = 0, /**< Qualifies a vector parameter (X, Y, Z, W). */
		COLOR = 1 /**< Qualifies a color parameter (R, G, B, A). */
	};
};


/**
	An interface to a generic FCollada parameter.
	Encapsulates one generic value.
	
	In PREMIUM FCollada: this value may be animatable,
	a list or a complex object, as described
	by higher-level classes. Useful for user interface,
	undo/redo and such advanced features.

	@ingroup FUParameter
*/
template <class TYPE>
class FUParameterT
{
private:
	TYPE value;

public:
    /** Default constructor. */
	FUParameterT() {}
    /** Copy constructor. @param defaultValue The value to copy into this parameter. */
	FUParameterT(const TYPE& defaultValue) : value(defaultValue) {}
    /** Destructor. */
	virtual ~FUParameterT() {}
    /** Retrieves the value of this parameter. */
	inline operator TYPE&() { return value; }
	inline operator const TYPE&() const { return value; } /**< See above. */
	inline TYPE& operator *() { return value; } /**< See above. */
	inline const TYPE& operator *() const { return value; } /**< See above. */
	inline TYPE* operator->() { return &value; } /**< See above. */
	inline const TYPE* operator->() const { return &value; } /**< See above. */
    /** Replaces the value of this parameter with a given value. @param copy The value to copy into this parameter. */
	FUParameterT<TYPE>& operator= (const TYPE& copy) { value = copy; return *this; }
};

typedef FUParameterT<bool> FUParameterBoolean; /**< A simple Boolean value parameter. */
typedef FUParameterT<float> FUParameterFloat; /**< A simple floating-point value parameter. */
typedef FUParameterT<FMVector2> FUParameterVector2; /**< A 2D vector parameter. */
typedef FUParameterT<FMVector3> FUParameterVector3; /**< A 3D vector parameter. */
typedef FUParameterT<FMVector3> FUParameterColor3; /**< A 3D color parameter. */
typedef FUParameterT<FMVector4> FUParameterVector4; /**< A 4D vector parameter. */
typedef FUParameterT<FMVector4> FUParameterColor4; /**< A 4D color parameter. */
typedef FUParameterT<FMMatrix44> FUParameterMatrix44; /**< A matrix parameter. */
typedef FUParameterT<int32> FUParameterInt32; /**< An integer value parameter. */
typedef FUParameterT<uint32> FUParameterUInt32; /**< An unsigned integer or enumerated-type value parameter. */
typedef FUParameterT<fm::string> FUParameterString; /**< A UTF8 string parameter. */
typedef FUParameterT<fstring> FUParameterFString; /**< A Unicode string parameter. */

typedef fm::vector<float, true> FUParameterFloatList; /**< A simple floating-point value list parameter. */
typedef fm::vector<FMVector2, true> FUParameterVector2List; /**< A 2D vector list parameter. */
typedef fm::vector<FMVector3, true> FUParameterVector3List; /**< A 3D vector list parameter. */
typedef fm::vector<FMVector3, true> FUParameterColor3List; /**< A 3D vector list parameter. */
typedef fm::vector<FMVector4, true> FUParameterVector4List; /**< A 4D vector list parameter. */
typedef fm::vector<FMVector4, true> FUParameterColor4List; /**< A 4D vector list parameter. */
typedef fm::vector<FMMatrix44, true> FUParameterMatrix44List; /**< A matrix list parameter. */
typedef fm::vector<int32, true> FUParameterInt32List; /**< An integer type list parameter. */
typedef fm::vector<uint32, true> FUParameterUInt32List; /**< An unsigned integer or enumerated-type list parameter. */
typedef fm::vector<fm::string, false> FUParameterStringList; /**< A UTF8 string list parameter. */
typedef fm::vector<fstring, false> FUParameterFStringList; /**< A Unicode string list parameter. */

/** Declares a parameter for the objects of the current class.
    Use this macro within a class declaration to add a parameter to it.
    The different versions of this macro are used for different parameter types:

    For floats, Booleans, vectors and strings, use DeclareParameter.
    For arrays of floats, Booleans, vectors and strings, use DeclareParameterList.
    For tracked pointers, use DeclareParameterPtr.
    For contained pointers, use DeclareParameterRef.
    For tracked lists of objects, use DeclareParameterTrackList.
    For containers of objects, use DeclareParameterContainer.
    
    @param type The value type of the parameter. Ex: float, bool, FMVector4 or string.
    @param qual The qualifier for the parameter type. @see FUParameterQualifiers::Qualifiers.
    @param parameterName The member variable name for the parameter.
    @param niceName The Unicode string to identify this parameter to users. */
#define DeclareParameter(type, qual, parameterName, niceName) \
	class Parameter_##parameterName : public FUParameterT<type> { \
	public: Parameter_##parameterName() : FUParameterT<type>() {} \
	Parameter_##parameterName(const type& defaultValue) : FUParameterT<type>(defaultValue) {} \
	virtual ~Parameter_##parameterName() {} \
	inline Parameter_##parameterName& operator= (const type& copy) { FUParameterT<type>::operator=(copy); return *this; } \
	} parameterName;

/** See above. */
#define DeclareParameterPtr(type, parameterName, niceName) FUTrackedPtr<type> parameterName;
/** See above. */
#define DeclareParameterRef(type, parameterName, niceName) FUObjectRef<type> parameterName;
/** See above. */
#define DeclareParameterList(list_type, parameterName, niceName) FUParameter##list_type##List parameterName;
/** See above. */
#define DeclareParameterTrackList(type, parameterName, niceName) FUTrackedList<type> parameterName;
/** See above. */
#define DeclareParameterContainer(type, parameterName, niceName) FUObjectContainer<type> parameterName;

/** Generates the code necessary for a parameter.
    These macros should only be used for non-primitive parameter types.
    Multi-dimensional vectors, matrices and strings are considered primitive types.
    The different versions of this macro are used for different contexts:

    For a parameter which requires a custom constructor, use ImplementParameterObject.
    For a parameter which has a default constructor, use ImplementParameterObjectNoArg.
    For a parameter which should never build itself, use ImplementParameterObjectNoCtr.
    When the parameter belongs to a template class, use ImplementParameterObjectT,
    ImplementParameterObjectNoArgT and ImplementParameterObjectNoCtrT.
    
    @param objectClassName The name of the class holding the parameter.
    @param parameterClassName The name of the non-primitive parameter type.
        IMPORTANT: This class must be based on top of FUObject, FUTrackable or FUParameterizable.
    @param parameterName The member variable name of the parameter. */
#define ImplementParameterObject(objectClassName, parameterClassName, parameterName, ...)
/** See above. */
#define ImplementParameterObjectNoArg(objectClassName, parameterClassName, parameterName)
/** See above. */
#define ImplementParameterObjectNoCtr(objectClassName, parameterClassName, parameterName)
/** See above. */
#define ImplementParameterObjectT(objectClassName, parameterClassName, parameterName, ...)
/** See above. */
#define ImplementParameterObjectNoArgT(objectClassName, parameterClassName, parameterName)
/** See above. */
#define ImplementParameterObjectNoCtrT(objectClassName, parameterClassName, parameterName)

/** Initializes the member variable for a parameter.
    Either one of these macros must be included for each member parameter of a class,
    within that class' constructors.
    The different versions of this macro are used for different contexts:
    
    For a parameter where the default constructor is to be used, use InitializeParameterNoArg.
    For a parameter where one or more values should be used in the constructor, use InitializeParameter.

    @param parameterName The member variable name of the parameter within the class. */
#define InitializeParameterNoArg(parameterName) parameterName()
/** See above. */
#define InitializeParameter(parameterName, ...) parameterName(__VA_ARGS__)

#if defined(__APPLE__) || defined(LINUX)
#include "FUtils/FUParameter.hpp"
#endif // __APPLE__

#endif // _FCD_PARAMETER_H

