/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

#ifndef _FU_TRACKER_H_
#define _FU_TRACKER_H_

/**
	@file FUTracker.h
	This file contains the FUTrackable base class and the related tracker template classes.
*/

#ifndef _FU_OBJECT_H_
#include "FUtils/FUObject.h"
#endif // _FU_OBJECT_H_

class FUTracker;

/**
	A trackable object.

	Each object holds a pointer to the trackers that track it.
	This pointer is useful so that the trackers can be notified if the object
	is released.

	@ingroup FUtils
*/
class FCOLLADA_EXPORT FUTrackable : public FUObject
{
private:
	DeclareObjectType(FUObject);

	// The objects tracking this one.
	typedef fm::pvector<FUTracker> FUTrackerList;
	FUTrackerList trackers;

public:
	/** Constructor.
		Although it is not an abstract class, this class is
		not meant to be used directly.  */
	FUTrackable();

	/** Destructor.
		This function informs the trackers of this object's release. */
	virtual ~FUTrackable();

	/** Retrieves the number of tracker tracking the object.
		This can be used as an expensive reference counting mechanism.
		@return The number of trackers tracking the object. */
	size_t GetTrackerCount() const { return trackers.size(); }

protected:
	/** Detaches all the trackers of this object.
		The trackers will be notified that this object has been released.
		It is not recommended to call this function outside of the Release() function. */
	void Detach();

private:
	friend class FUTracker;
	void AddTracker(FUTracker* tracker);
	void RemoveTracker(FUTracker* tracker);
	bool HasTracker(const FUTracker* tracker) const;
};


/**
	An object set
	Each set has access to a list of unique objects.
	When the objects are created/released: they will inform the
	list.
	@ingroup FUtils
*/
class FCOLLADA_EXPORT FUTracker
{
public:
	/** Destructor. */
	virtual ~FUTracker() {}

	/** Callback when an object tracked by this tracker
		is being released.
		@param object A tracked object. */
	virtual void OnObjectReleased(FUTrackable* object) = 0;

	/** Retrieves whether an object is tracked by this tracker.
		@param object An object. */
	virtual bool TracksObject(const FUTrackable* object) const { return object != NULL ? object->HasTracker(this) : false; }

protected:
	/** Adds an object to be tracked.
		@param object The object to track. */
	void TrackObject(FUTrackable* object) { if (object) object->AddTracker(this); }

	/** Stops tracking an object
		@param object The object to stop tracking. */
	void UntrackObject(FUTrackable* object) { if (object) object->RemoveTracker(this); }
};

/**
	A tracked object pointer
	The reverse idea of a smart pointer: if the object pointed
	to by the pointer is released, the pointer will become NULL.
	@ingroup FUtils
*/
template <class ObjectClass = FUTrackable>
class FUTrackedPtr : public FUTracker
{
protected:
	/** The tracked pointer. */
	ObjectClass* ptr;

public:
	/** Copy constructor.
		@param _ptr The object to track. This pointer can be NULL to indicate
			that no object should be tracked at this time. */
	FUTrackedPtr(ObjectClass* _ptr = NULL) : ptr(_ptr)
	{
		if (ptr != NULL) FUTracker::TrackObject((FUTrackable*) ptr);
		ptr = ptr;
	}

	/** Destructor.
		Stops the tracking of the pointer. */
	~FUTrackedPtr()
	{
		if (ptr != NULL) FUTracker::UntrackObject((FUTrackable*) ptr);
		ptr = NULL;
	}

	/** Assigns this tracking pointer a new object to track.
		@param _ptr The new object to track.
		@return This reference. */
	FUTrackedPtr& operator=(ObjectClass* _ptr)
	{
		if (ptr != NULL) FUTracker::UntrackObject((FUTrackable*) ptr);
		ptr = _ptr;
		if (ptr != NULL) FUTracker::TrackObject((FUTrackable*) ptr);
		return *this;
	}
	inline FUTrackedPtr& operator=(const FUTrackedPtr& _ptr) { return operator=(_ptr.ptr); } /**< See above. */

	/** Retrieves whether an object is tracked by this tracker.
		@param object An object. */
	virtual bool TracksObject(const FUTrackable* object) const { return (FUTrackable*) ptr == object; }

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
	virtual void OnObjectReleased(FUTrackable* object)
	{
		FUAssert(TracksObject(object), return);
		ptr = NULL;
	}
};

/**
	An object list.
	Based on top of our modified version of the STL vector class,
	this contained object list holds pointers to some FUTrackable derived class
	and automatically removes objects when they are deleted.
	@ingroup FUtils
*/
template <class ObjectClass = FUTrackable>
class FUTrackedList : private fm::pvector<ObjectClass>, FUTracker
{
public:
	typedef fm::pvector<ObjectClass> Parent;
	typedef ObjectClass* item;
	typedef const ObjectClass* const_item;
	typedef item* iterator;
	typedef const_item* const_iterator;

	/** Destructor. */
	virtual ~FUTrackedList() { clear(); }

	/** Clears the object tracked by this object list. */
	void clear()
	{
		for (iterator it = begin(); it != end(); ++it)
		{
			FUTracker::UntrackObject((FUTrackable*) (*it));
		}
		Parent::clear();
	}
	
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
		FUTracker::TrackObject((FUTrackable*) object);
		Parent::push_back(object);
	}

	/** Inserts an object in the container's containment list.
		@param _iterator The iterator after which to insert the object.
		@param object An object to insert.
		@return The iterator to the inserted object. */
	iterator insert(iterator _iterator, ObjectClass* object)
	{
		FUTracker::TrackObject(object);
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
				FUTracker::TrackObject(const_cast<FUTrackable*>((const FUTrackable*) (*_startIterator)));
			}
		}
	}

	/** Removes the last value of the tracked object list. */
	void pop_back()
	{
		if (!Parent::empty())
		{
			FUTracker::UntrackObject(back());
			Parent::pop_back();
		}
	}
	
	/** Removes the value at the given position within the list.
		@param _it The list position for the value to remove. */
	iterator erase(iterator _it)
	{
		FUTracker::UntrackObject((FUTrackable*) *_it);
		return (iterator) Parent::erase(_it);
	}

	/** Removes a range of values from the list.
		@param first The list position of the first value to remove.
		@param last The list position just passed the last value to remove. */
	inline void erase(iterator first, iterator last)
	{
		for (iterator it = first; it != last; ++it) FUTracker::UntrackObject((FUTrackable*) *it);
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
		iterator it = Parent::find(value);
		if (it != Parent::end())
		{
			FUTracker::UntrackObject((FUTrackable*) *it);
			Parent::erase(it);
			return true;
		}
		return false;
	}

	/** Removes an indexed value contained within the list.
		@param index The index of the value to erase. */
	inline void erase(size_t index) { erase(begin() + index); }

	/** Retrieves whether an object is contained by this container.
		@param object An object. */
	virtual bool TracksObject(const FUTrackable* object) const { return Parent::contains((ObjectClass*) object); }

	/** Clones a list of tracked objects.
		This list will stop tracking all its current tracked objects
		and will start tracking the objects within the other list.
		@param other A second list of tracked objects.
		@return This list. */
	FUTrackedList<ObjectClass>& operator= (const FUTrackedList<ObjectClass>& other) { clear(); insert(end(), other.begin(), other.end()); return *this; }

	inline bool empty() const { return Parent::empty(); } /**< Inherited from pvector. */
	inline size_t size() const { return Parent::size(); } /**< Inherited from pvector. */
	void reserve(size_t count) { Parent::reserve(count); } /**< Inherited from pvector. */
	inline bool contains(const ObjectClass* value) const { return Parent::contains(value); } /**< Inherited from pvector. */

	/** Releases a value contained within a list.
		Use this function only if there is no duplicate pointers within the list.
		@param value The value, contained within the list, to release.
		@return Whether the value was found and released. */
	inline bool release(const ObjectClass* value)
	{
		ObjectClass** it = find(value);
		if (it != Parent::end()) { erase(it); ((FUTrackable*) value)->Release(); return true; }
		return false;
	}

	/** Removes the first value of the tracked object list. */
	void pop_front()
	{
		if (!Parent::empty())
		{
			FUTracker::UntrackObject(front());
			Parent::pop_front();
		}
	}

protected:
	/** Removes an object from the container's containment list.
		@param object A contained object. */
	virtual void OnObjectReleased(FUTrackable* object)
	{
		FUAssert(TracksObject(object), return);
		Parent::erase((ObjectClass*) object);
	}
};

#endif // _FU_TRACKER_H_
