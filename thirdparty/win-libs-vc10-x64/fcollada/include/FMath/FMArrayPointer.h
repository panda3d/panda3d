/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

/**
	@file FMArrayPointer.h
	The file contains the pvector class, which implemented the FCollada
	specialized vector class for faster and smaller builds.
 */

#ifndef _FM_ARRAY_POINTER_H_
#define _FM_ARRAY_POINTER_H_

#ifndef _FM_ARRAY_H_
#include "FMath/FMArray.h"
#endif // _FM_ARRAY_H_

// Already documented in FMArray.h.
namespace fm
{
	/**
		A dynamically-sized array of pointers.
		Built on top of the FCollada specialized vector class, this class improves on the compilation time
		and the library size by re-using the <void*> template only once for all pointer vectors.

		@ingroup FMath
	*/
	template <class T>
	class pvector : public fm::vector<const void*, true>
	{
	private:
		/** Use this pointer to get typed information in the debugger.
			The few bytes that we loose are nothing compared to the annoyance of trying to debug in these. */
		T*** first; 
		
	public:
		typedef typename fm::vector<const void*, true> Parent; /**< Defines the parent class to the pointer array. */
		typedef T* item; /**< Defines the item pointer type contained by the pointer array. */
		typedef const T* const_item; /**< Defines the constant-version of the item pointer type contained by the pointer array. */
		typedef item* iterator; /**< Defines the item pointer iterator for the pointer array. */
		typedef const_item* const_iterator; /**< Defines the constant-version of the item pointer iterator for the pointer array. */
		
		/** Default constructor. */
		pvector() : Parent()
		{
			first = (T***) (size_t) &heapBuffer;
		}

		/** Constructor. Builds a dynamically-sized pointer array of the desired size.
			@param size The desired size of the array. */
		pvector(size_t size) : Parent(size, NULL)
		{
			first = (T***) (size_t) &heapBuffer;
		}

		/** Constructor. Builds a dynamically-sized pointer array of the desired size.
			@param size The desired size of the pointer array
			@param defaultValue The default value to use for all the pointers of the array. */
		pvector(size_t size, const T& defaultValue) : Parent(size, defaultValue)
		{
			first = (T***) &heapBuffer;
		}

		/** Copy constructor.
			@param copy The dynamically-sized pointer array to copy the values from. */
		pvector(const pvector<T>& copy) : Parent(copy)
		{
			first = (T***) (size_t) &heapBuffer;
		}

		/** Constructor. Builds a dynamically-sized pointer array from a constant-sized array.
			@param values A constant-sized array of floating-point values.
			@param count The size of the constant-sized array. */
		pvector(const T** values, size_t count) : Parent()
		{
			first = (T***) (size_t) &heapBuffer;
			resize(count);
			memcpy(&front(), values, count * sizeof(void*));
		}

		/** Destructor. */
		~pvector()
		{
#ifdef _DEBUG
			Parent::clear();
			Parent::push_back((void*) (size_t) 0xFFFFFFFF);
			first = (T***) (size_t) 0xDEADDEAD;
#endif // _DEBUG
		}

		/** Retrieves the first element of the pointer array.
			@return The first element in the pointer array. */
		inline T*& front() { return (T*&) Parent::front(); }
		inline const T*& front() const { return (const T*&) Parent::front(); } /**< See above. */

		/** Retrieves the last element of the pointer array.
			@return The last element in the pointer array. */
		inline T*& back() { return (T*&) Parent::back(); }
		inline const T*& back() const { return (const T*&) Parent::back(); } /**< See above. */

		/** Retrieves an indexed object in the pointer array.
			@param index An index.
			@return The given object. */
		inline T*& at(size_t index) { return (T*&) Parent::at(index); }
		inline const T*& at(size_t index) const { return (const T*&) Parent::at(index); } /**< See above. */
		template <class INTEGER> inline T*& operator[](INTEGER index) { return (T*&) Parent::at(index); } /**< See above. */
		template <class INTEGER> inline const T*& operator[](INTEGER index) const { return (const T*&) Parent::at(index); } /**< See above. */

		/** Retrieves an iterator for the first element in the pointer array.
			@return an iterator for the first element in the pointer array. */
		inline iterator begin() { return (!empty()) ? &front() : NULL; }
		inline const_iterator begin() const { return (!empty()) ? &front() : NULL; } /**< See above. */
		
		/** Retrieves an iterator for the element after the last element in the pointer array.
			@return an iterator for the element after the last element in the pointer array. */
		inline iterator end() { return (!empty()) ? (&back()) + 1 : NULL; }
		inline const_iterator end() const { return (!empty()) ? (&back()) + 1 : NULL; } /**< See above. */

		/** Retrieves an iterator for a given element in the pointer array.
			@param item An item of the pointer array.
			@return An iterator for the given item. If the item is not
				found in the pointer array, the end() iterator is returned. */
		inline iterator find(const T* item) { Parent::iterator f = Parent::find(item); return begin() + (f - Parent::begin()); }
		inline const_iterator find(const T* item) const { Parent::const_iterator f = Parent::find(item); return begin() + (f - Parent::begin()); } /**< See above. */

		/** Inserts an object in the container's containment pointer array.
			@param _iterator The iterator after which to insert the object.
			@param object An object to insert.
			@return The iterator to the inserted object. */
		inline iterator insert(iterator _iterator, T* object)
		{
			iterator originalStart = begin();
			Parent::iterator newIt = Parent::insert(Parent::begin() + (_iterator - originalStart), object);
			return begin() + (newIt - Parent::begin());
		}

		/** Adds a given number of NULL pointers at a given position in the pointer array.
			@param _iterator The iterator after which to insert the object.
			@param count The number of NULL pointers to add. */
		inline void insert(iterator _iterator, size_t count)
		{
			Parent::iterator it = Parent::begin() + (_iterator - begin());
			Parent::insert(it, count, NULL);
		}

		/** Inserts a list of pointers in the pointer array.
			@param _where The iterator after which to insert the pointers.
			@param _startIterator The iterator for the first pointer to insert.
			@param _endIterator The iterator for the last pointer.
				This pointer will not be inserted. */
		template <class _It>
		inline void insert(iterator _where, _It _startIterator, _It _endIterator)
		{
			if (_startIterator < _endIterator)
			{
				size_t relativeWhere = _where - begin();
				size_t count = _endIterator - _startIterator;
				Parent::insert(Parent::begin() + relativeWhere, count, (T*) 0);
				_where = begin() + relativeWhere;

				memcpy(_where, _startIterator, sizeof(void*) * count);
			}
		}
		
		/** Removes the value at the given position within the pointer array.
			@param _it The position for the pointer to remove. */
		inline iterator erase(iterator _it)
		{
			Parent::iterator it = Parent::begin() + (_it - begin());
			it = Parent::erase(it);
			return begin() + (it - Parent::begin());
		}

		/** Removes a given pointer from the pointer array.
			@param value The pointer to remove from the pointer array.
			@return Whether the given pointer existed within the pointer array. */
		inline bool erase(const T* value)
		{
			Parent::iterator it = Parent::find(value);
			if (it != Parent::end()) { Parent::erase(it); return true; }
			return false;
		}

		/** Removes the value at the given position within the pointer array.
			@param first The start position for the pointers to remove.
			@param last The end position for the pointers to remove. */
		inline void erase(iterator first, iterator last)
		{
			Parent::erase(Parent::begin() + (first - begin()), Parent::begin() + (last - begin()));
		}

		/** Removes an indexed value contained within the list.
			@param index The index of the value to erase. */
		inline void erase(size_t index) { Parent::erase(Parent::begin() + index); }

		/** Releases a value contained within a list.
			Use this function only if there is no duplicate pointers within the list.
			@param value The value, contained within the list, to release.
			@return Whether the value was found and released. */
		inline bool release(const T* value)
		{
			Parent::iterator it = Parent::find(value);
			if (it != Parent::end()) { Parent::erase(it); delete value; return true; }
			return false;
		}

		/** Copy constructor.
			Overwrites the current data of the pointer array with the data of the given pointer array.
			@param other The pointer array to copy.
			@return The copied pointer array. */
		pvector<T>& operator= (const pvector<T>& other) { clear(); insert(end(), other.begin(), other.end()); return *this; }

		/** Resizes the pointer array to the given amount.
			It is intentional that the default value is NULL.
			@param count The desired size for the pointer array. */
		inline void resize(size_t count) { Parent::resize(count, NULL); }
	};
};

#endif // _FM_ARRAY_POINTER_H_
