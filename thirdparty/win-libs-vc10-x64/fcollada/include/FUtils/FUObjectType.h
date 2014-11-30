/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/
/*
	We use references to static objects so that the order of initialization shouldn't matter.
*/

/**
	@file FUObjectType.h
	This file contains the FUObjectType class.
*/

#ifndef _FU_OBJECT_TYPE_H_
#define _FU_OBJECT_TYPE_H_

/**
	An object type.

	Used for RTTI-purpose and to easily allow up-classing of objects.
	Every object class should have one object type has a static member
	and available through the virtual FUObject::GetObjectType function.
	All FUObject up-classes should use the DeclareObjectType macro.

	@ingroup FUtils
*/
class FCOLLADA_EXPORT FUObjectType
{
private:
	const FUObjectType* parent;
	const char* typeName;

public:
	/** [INTERNAL] Constructor: do not use directly.
		Only the FUObject class should use this constructor.
		All other class object types should use the constructor below.
		Use neither constructor directly: use the ImplementObjectType macro.
		@param typeName The name of type.
			Used only in debug builds for debugging purposes. */
	FUObjectType(const char* typeName);

	/** [INTERNAL] Constructor: do not use directly.
		Use the ImplementObjectType macro.
		@param parent The parent class object type.
		@param typeName The name of type.
			Used only in debug builds for debugging purposes. */
	FUObjectType(const FUObjectType& parent, const char* typeName);

	/** Retrieves the type of the down-class.
		@return The type of the down-class. The FUObject class
			will return itself as the parent, so be careful when looping down.*/
	const FUObjectType& GetParent() const { return (parent != NULL) ? *parent : *this; }

	/** Retrieves whether this object type includes a given object type.
		This function allows you to verify if this object type is
		of a given object type or some up-class of the given object type.
		You should use this comparison function before up-casting an object.
		@param otherType A second object type.
		@return Whether this object type includes the given object type. */
	bool Includes(const FUObjectType& otherType) const;

	/** Retrieves whether a given object type is equivalent to this object type.
		@param otherType A second object type.
		@return Whether the two object types are equivalent. */
	inline bool operator==(const FUObjectType& otherType) const { return &otherType == this; }

	/** Retrieves whether a given object type is different from this object type.
		@param otherType A second object type.
		@return Whether the two object type are different. */
	inline bool operator!=(const FUObjectType& otherType) const { return &otherType != this; }

	/** Retrieves the object type name.
		@return The object type name. */
	inline const char* GetTypeName() const { return typeName; }
};

/**
	Declares the object type for an object class.
	Use this macro inside the class declarations of up-classes of the FUObject class
	to easily implement RTTI.
	@param _ParentClass The class name for the parent class.
*/
#define DeclareObjectType(_ParentClass) \
private: \
	static class FUObjectType __classType; \
	typedef _ParentClass Parent; \
public: \
	static const FUObjectType& GetClassType() { return __classType; } \
	virtual const FUObjectType& GetObjectType() const { return __classType; } \
	virtual void Release(); \
private:

/**
	Implements the object type for an object class.
	Use this macro inside your code files only to create the objects
	necessary to support RTTI in your up-classes of the FUObject class.
	@param ClassName The name of the class.
*/
#define ImplementObjectType(ClassName) \
	FUObjectType ClassName::__classType(Parent::GetClassType(), #ClassName); \
	void ClassName::Release() { Detach(); delete this; }

/** See above. */
#define ImplementObjectTypeT(ClassName) \
	template <> \
	FUObjectType ClassName::__classType(Parent::GetClassType(), #ClassName); \
	template <> \
	void ClassName::Release() { Detach(); delete this; }

/**
	Implements the object type for an object class, but without the Release() function.
	Use this macro inside your code files only to create the objects
	necessary to support RTTI in your up-classes of the FUObject class.
	You will need to manually implement the Release() function, without declaring it.
	The default implementation of the Release() function is the following:
	Release() { delete this; }.
	@param ClassName The name of the class.
*/
#define ImplementObjectType_NoDefaultRelease(ClassName) \
	FUObjectType ClassName::__classType(Parent::GetClassType(), #ClassName);

#endif // _FU_OBJECT_TYPE_H_
