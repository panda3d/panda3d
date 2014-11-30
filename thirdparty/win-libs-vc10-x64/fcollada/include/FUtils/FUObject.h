/*
   Copyright (C) 2005-2007 Feeling Software Inc.
   Portions of the code are:
   Copyright (C) 2005-2007 Sony Computer Entertainment America
   
   MIT License: http://www.opensource.org/licenses/mit-license.php
*/

/**
	@file FUObject.h
	This file contains the FUObject class, the FUObjectOwner interface,
	the FUObjectRef template class and FUObjectContainer template class.
*/

#ifndef _FU_OBJECT_H_
#define _FU_OBJECT_H_

#ifndef _FU_OBJECT_TYPE_H_
#include "FUtils/FUObjectType.h"
#endif // _FU_OBJECT_TYPE_H_

class FUObjectOwner;

/**
	A basic object.
	Each up-class of this basic object class hold an object type
	that acts just like RTTI to provide a safe way to up-cast.
	@ingroup FUtils
*/
class FCOLLADA_EXPORT FUObject
{
private:
	static class FUObjectType* baseObjectType;

	FUObjectOwner* objectOwner;

protected:
	/** [INTERNAL] Necessary, in order for the
		flag macros to work on up-classes. */
	static const uint32 nextAvailableBit = 0;

public:
	/** Constructor.
		Although it is not an abstract class, this class is
		not meant to be used directly.  */
	FUObject();

	/** Destructor. */
	virtual ~FUObject();

	/** Releases this object.
		This function essentially calls the destructor.
		This function is virtual and is always
		overwritten when using the ImplementObjectType macro. */
	virtual void Release();

	/** Retrieves the type of the base object class.
		@return The type of the base object class. */
	static const FUObjectType& GetClassType() { return *baseObjectType; }

	/** Retrieves the type of the object class.
		@return The type of the base object class. */
	virtual const FUObjectType& GetObjectType() const { return *baseObjectType; }

	/** Retrieves whether this object has exactly the given type.
		@param _type A given class type.
		@return Whether this object is exactly of the given type. */
	inline bool IsType(const FUObjectType& _type) const { return GetObjectType() == _type; }

	/** Retrieves whether this object is exactly or inherits the given type.
		@param _type A given class type.
		@return Whether this object is exactly or inherits the given type. */
	inline bool HasType(const FUObjectType& _type) const { return GetObjectType().Includes(_type); }

protected:
	/** Detaches this object from its owner.
		Mainly notifies the owner before the destructor is called. */
	void Detach();

private:
	friend class FUObjectOwner;

	/** Sets the owner for this object.
		@param owner The new owner. */
	inline void SetObjectOwner(FUObjectOwner* owner)
	{
		// Are you attempting to transfer ownership or do multiple-containment?
		// (owner == NULL) is used to avoid the notification when the container
		// knows its about to release this object.
		FUAssert(objectOwner == NULL || owner == NULL, return);
		objectOwner = owner;
	}
};

/**
	An object owner.
	This interface allows containment classes to be notified when a
	contained object is released.
	@ingroup FUtils
*/
class FUObjectOwner
{
protected:
	/** Owns the given object.
		@param object The object to own. */
	inline void AttachObject(FUObject* object) { object->SetObjectOwner(this); }

	/** Detaches the given object.
		Use this function right before releasing the object to avoid calling the
		notification function (OnOwnedObjectReleased).
		@param object The object to detach. */
	inline void DetachObject(FUObject* object)
	{
		// This assert verifies that we are, indeed, the owner.
		FUAssert(object->objectOwner == this, return);
		object->SetObjectOwner(NULL);
	}

public:
	/** Destructor. */
	virtual ~FUObjectOwner() {}

	/** Notification from the object to its owner, when it is released.
		@param object The object being released. */
	virtual void OnOwnedObjectReleased(FUObject* object) = 0;
};

/**
	Macros used to dynamically case a given FUObject pointer to some higher
	class object.
	@ingroup FUtils
*/
template <class HigherClassType>
inline HigherClassType* DynamicCast(FUObject* object) { return object->HasType(HigherClassType::GetClassType()) ? (HigherClassType*) object : NULL; }

/**
	An object reference
	On top of the tracked object pointer, when this reference
	is released: the tracked object is released.
 
	This template is very complex for a reference.
	You get reduced compilation times when compared to simple containment.
	
	@ingroup FUtils
*/
template <class ObjectClass = FUObject>
class FUObjectRef : public FUObjectOwner
{
private:
	typedef fm::pvector<ObjectClass> Parent;

	/** The owned object. */
	ObjectClass* ptr;

public:
	/** Copy constructor.
		@param _ptr The object to reference. This pointer can be NULL to indicate
			that no object should be referenced at this time. */
	FUObjectRef(ObjectClass* _ptr = NULL)
	:	ptr(_ptr)
	{
		if (_ptr != NULL) AttachObject((FUObject*) ptr);
	}

	/** Destructor.
		The object referenced will be released. */
	~FUObjectRef()
	{
		if (ptr != NULL)
		{
			DetachObject((FUObject*) ptr);
			((FUObject*) ptr)->Release();
#ifdef _DEBUG

			ptr = NULL;
#endif // _DEBUG

		}
	}

	/** Assigns this reference to own a new object.
		@param _ptr The new object to own.
		@return This reference. */
	FUObjectRef<ObjectClass>& operator=(ObjectClass* _ptr)
	{
		if (ptr != NULL) ((FUObject*) ptr)->Release();
		FUAssert(ptr == NULL, return *this);
		ptr = _ptr;
		if (_ptr != NULL) AttachObject((FUObject*) ptr);
		return *this;
	}

	/** Exchanges the reference from one object to another.
		If this reference already points to an object, it will be released.
		If the other reference points to an object, that object will
		now be owned by this reference and the other reference will point to NULL.
		@param _ptr The other reference.
		@return This reference. */
	FUObjectRef<ObjectClass>& operator=(FUObjectRef<ObjectClass>& _ptr)
	{
		operator=(_ptr.ptr);
		_ptr.ptr = NULL;
		return *this;
	}

	/** Accesses the tracked object.
		@return The tracked object. */
	inline ObjectClass& operator*() { FUAssert(ptr != NULL, return *ptr); return *ptr; }
	inline const ObjectClass& operator*() const { FUAssert(ptr != NULL, return *ptr); return *ptr; } /**< See above. */
	inline ObjectClass* operator->() { return ptr; } /**< See above. */
	inline const ObjectClass* operator->() const { return ptr; } /**< See above. */
	inline operator ObjectClass*() { return ptr; } /**< See above. */
	inline operator const ObjectClass*() const { return ptr; } /**< See above. */

protected:
	/** Callback when an object tracked by this tracker
		is being released.
		@param object A contained object. */
	virtual void OnOwnedObjectReleased(FUObject* object)
	{
		FUAssert((size_t) object == (size_t) ptr, return);
		ptr = NULL;
	}
};


/**
	A contained object list.
	When this list is released, the contained objects are also released.
	Each object should have only one owner.
	@ingroup FUtils
*/
template <typename ObjectClass = FUObject>
class FUObjectContainer : private fm::pvector<ObjectClass>, public FUObjectOwner
{
private:
	typedef fm::pvector<ObjectClass> Parent;
public:
	typedef ObjectClass** iterator;
	typedef const ObjectClass** const_iterator;

public:
	/** Destructor.
		Releases all the objects contained within this container. */
	virtual ~FUObjectContainer() { clear(); }

	/** Clears and releases the object tracked by this object list. */
	void clear()
	{
		while (size() > 0)
		{
			FUObject* last = (FUObject*) Parent::back();
			Parent::pop_back();
			DetachObject(last);
			last->Release();
		}
	}

	/** Retrieves the number of elements in the container.
		@return The number of elements in the container. */
	inline size_t size() const { return Parent::size(); }

	/** Retrieves whether there are values in this container.
		@return Whether there are values in the container. */
	inline bool empty() const { return Parent::empty(); }

	/** Retrieves the first element of the container.
		@return The first element in the container. */
	ObjectClass*& front() { return (ObjectClass*&) Parent::front(); }
	const ObjectClass*& front() const { return (const ObjectClass*&) Parent::front(); } /**< See above. */

	/** Retrieves the last element of the container.
		@return The last element in the container. */
	ObjectClass*& back() { return (ObjectClass*&) Parent::back(); }
	const ObjectClass*& back() const { return (const ObjectClass*&) Parent::back(); } /**< See above. */

	/** Retrieves an indexed object in the list.
		@param index An index.
		@return The given object. */
	inline ObjectClass* at(size_t index) { return (ObjectClass*) Parent::at(index); }
	inline const ObjectClass* at(size_t index) const { return (const ObjectClass*) Parent::at(index); } /**< See above. */
	template <class INTEGER> inline ObjectClass* operator[](INTEGER index) { return at(index); } /**< See above. */
	template <class INTEGER> inline const ObjectClass* operator[](INTEGER index) const { return at(index); } /**< See above. */

	/** Retrieves an iterator for the first element in the list.
		@return an iterator for the first element in the list. */
	inline iterator begin() { return (iterator) Parent::begin(); }
	inline const_iterator begin() const { return (const_iterator) Parent::begin(); } /**< See above. */

	/** Retrieves an iterator for the element after the last element in the list.
		@return an iterator for the element after the last element in the list. */
	inline iterator end() { return (iterator) Parent::end(); }
	inline const_iterator end() const { return (const_iterator) Parent::end(); } /**< See above. */

	/** Retrieves an iterator for a given element in the list.
		@param item An item of the list.
		@return An iterator for the given item. If the item is not
			found in the list, the end() iterator is returned. */
	inline iterator find(const ObjectClass* item) { return (iterator) Parent::find(item); }
	inline const_iterator find(const ObjectClass* item) const { return (const_iterator) Parent::find(item); } /**< See above. */

	/** Adds an object to the container's containment list.
		@param object An object to contain. */
	inline void push_back(ObjectClass* object)
	{
		AttachObject((FUObject*) object);
		Parent::push_back(object);
	}

	/** Inserts an object in the container's containment list.
		@param _iterator The iterator after which to insert the object.
		@param object An object to insert.
		@return The iterator to the inserted object. */
	iterator insert(iterator _iterator, ObjectClass* object)
	{
		AttachObject((FUObject*) object);
		return (iterator) Parent::insert(_iterator, object);
	}

	/** Inserts an object in the container's containment list.
		@param index Where to insert the object.
		@param object An object to insert. */
	inline void insert(size_t index, ObjectClass* object) { insert(begin() + index, object); }

	/** Inserts a list of object in the container's containment list.
		@param _where The iterator after which to insert the object.
		@param _startIterator The iterator for the first object to insert.
		@param _endIterator The iterator just passed the last object.
			This object will not be inserted. */
	template <class _It>
	void insert(iterator _where, _It _startIterator, _It _endIterator)
	{
		if (_startIterator < _endIterator)
		{
			size_t relativeWhere = _where - begin();
			size_t count = _endIterator - _startIterator;
			Parent::insert(Parent::begin() + relativeWhere, count);
			_where = begin() + relativeWhere;

			for (; _startIterator != _endIterator; ++_startIterator, ++_where)
			{
				*_where = const_cast<ObjectClass*>((const ObjectClass*)(*_startIterator));
				AttachObject((FUObject*) *_startIterator);
			}
		}
	}

	/** Removes the last value of the tracked object list. */
	void pop_back()
	{
		FUAssert(!Parent::empty(), return);
		FUObject* last = (FUObject*) Parent::back();
		Parent::pop_back();
		DetachObject(last);
		last->Release();
	}

	/** Removes the first value of the object container.
		Warning: this function may result in large memory copies,
		since we use an array to contain objects. */
	void pop_front()
	{
		FUAssert(!Parent::empty(), return);
		FUObject* first = (FUObject*) Parent::front();
		Parent::pop_front();
		DetachObject(first);
		first->Release();
	}

	/** Removes the value at the given position within the list.
		@param _it The list position for the value to remove. */
	iterator erase(iterator _it)
	{
		FUAssert(contains(*_it), return _it);
		FUObject* o = (FUObject*) * _it;
		iterator it = (iterator) Parent::erase(_it);
		DetachObject(o);
		o->Release();
		return it;
	}

	/** Removes a range of values from the list.
		@param first The list position of the first value to remove.
		@param last The list position just passed the last value to remove. */
	inline void erase(iterator first, iterator last)
	{
		for (iterator it = first; it != last; ++it)
		{
			FUObject* o = (FUObject*) * it;
			DetachObject(o);
			o->Release();
		}
		Parent::erase(first, last);
	}

	/** Removes a range of values from the list.
		@param first The index of the first value to remove.
		@param last The index just passed the last value to remove. */
	inline void erase(size_t first, size_t last) { erase(begin() + first, begin() + last); }

	/** Removes a value contained within the list, once.
		@param value The value, contained within the list, to erase from it.
		@return Whether the value was found and erased from the list. */
	inline bool erase(const ObjectClass* value)
	{
		iterator it = find(value);
		if (it == Parent::end()) return false;
		erase(it);
		return true;
	}

	/** Detaches the value at the given position from the list.
		The object is not released, so after this call: it has no owner
		and must be released manually.
		@param _it The list position for the value to remove. */
	iterator Detach(iterator _it)
	{
		FUAssert(contains(*_it), return _it);
		FUObject* o = (FUObject*) * _it;
		iterator it = (iterator) Parent::erase(_it);
		DetachObject(o);
		return it;
	}

	/** Detaches a range of values from the list.
		The objects are not released, so after this call: they have no owner
		and must be released manually.
		@param first The list position of the first value to remove.
		@param last The list position just passed the last value to remove. */
	inline void Detach(iterator first, iterator last)
	{
		for (iterator it = first; it != last; ++it)
		{
			FUObject* o = (FUObject*) * it;
			DetachObject(o);
		}
		Parent::erase(first, last);
	}

	/** Detaches a range of values from the list.
		The objects are not released, so after this call: they have no owner
		and must be released manually.
		@param first The index of the first value to remove.
		@param last The index just passed the last value to remove. */
	inline void Detach(size_t first, size_t last) { Detach(begin() + first, begin() + last); }

	/** Detaches a given value from the list.
		The object is not released, so after this call: it has no owner
		and must be released manually.
		@param value The value, contained within the list, to detach from it.
		@return Whether the value was found and detached from the list. */
	inline bool Detach(const ObjectClass* value)
	{
		iterator it = find(value);
		if (it == Parent::end()) return false;
		Detach(it);
		return true;
	}

	/** Removes an indexed value contained within the list.
		@param index The index of the value to erase. */
inline void erase(size_t index) { erase(begin() + index); }

	/** Pre-caches the wanted number of pointers.
		Use this function to avoid many memory re-allocations.
		@param count The wanted number of pre-cached pointers. */
	inline void reserve(size_t count) { Parent::reserve(count); }

	/** Retrieves whether this container owns a given object.
		@param value An object.
		@return Whether the object is owned by this container. */
	inline bool contains(const ObjectClass* value) const { return Parent::contains(value); }

	/** Releases a value contained within a list.
		Use this function only if there is no duplicate pointers within the list.
		@deprecated Use FUObject::Release instead.
		@param value The value, contained within the list, to release.
		@return Whether the value was found and released. */
	DEPRECATED(3.05A, "FUObject::Release()") bool release(const ObjectClass* value)
	{
		ObjectClass** it = find(value);
		if (it != Parent::end()) { ((FUObject*) value)->Release(); return true; }
		return false;
	}

	/** Adds a new empty object to the container.
		@return The new empty object. */
	ObjectClass* Add()
	{
		ObjectClass* object = new ObjectClass();
		push_back(object);
		return object;
	}

	/** Adds a new object to the container.
		@param arg1 An constructor argument.
		@return The new object. */
	template <class A1>
	ObjectClass* Add(const A1& arg1)
	{
		ObjectClass* object = new ObjectClass(arg1);
		push_back(object);
		return object;
	}

	/** Adds a new object to the container.
		@param arg1 A first constructor argument.
		@param arg2 A second constructor argument.
		@return The new object. */
	template <class A1, class A2>
	ObjectClass* Add(const A1& arg1, const A2& arg2)
	{
		ObjectClass* object = new ObjectClass(arg1, arg2);
		push_back(object);
		return object;
	}

	/** Adds a new object to the container.
		@param arg1 A first constructor argument.
		@param arg2 A second constructor argument.
		@param arg3 A third constructor argument.
		@return The new object. */
	template <class A1, class A2, class A3>
	ObjectClass* Add(const A1& arg1, const A2& arg2, const A3& arg3)
	{
		ObjectClass* object = new ObjectClass(arg1, arg2, arg3);
		push_back(object);
		return object;
	}

protected:
	/** Removes an object from the container's owned list.
		@param object A contained object. */
	virtual void OnOwnedObjectReleased(FUObject* object)
	{
		FUAssert(Parent::contains(object), return);
		Parent::erase((ObjectClass*) object);
	}
};

#endif // _FU_OBJECT_H_
