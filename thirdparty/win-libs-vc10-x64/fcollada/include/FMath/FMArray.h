/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

/**
	@file FMArray.h
	The file contains the vector class, which improves on the standard C++ vector class.
 */

#ifndef _FM_ARRAY_H_
#define _FM_ARRAY_H_

#ifndef _FM_ALLOCATOR_H_
#include "FMath/FMAllocator.h"
#endif // _FM_ALLOCATOR_H_
#ifndef _FM_SORT_H_
#include "FMath/FMSort.h"
#endif // _FM_SORT_H_

#ifdef WIN32
#pragma warning(disable:4127)
#endif //WIN32

/** Namespace that contains the overwritten STL classes. */
namespace fm
{
	/**
		A dynamically-sized array.
		Intentionally has an interface similar to the standard C++ vector class.
		It's implement should be very similar yet more lightweight.
		
		We have also added useful extra functionality, such as a constructor that takes
		in a constant-sized array, comparison with a constant-sized array, erase and find
		functions that take in a value, etc.

		@ingroup FMath
	*/
	template <class T, bool PRIMITIVE=false>
	class vector
	{
	protected:
		size_t reserved; /**< The capacity of the vector. */
		size_t sized; /**< The number of values contained in the vector. */
		T* heapBuffer; /**< The heap buffer that contains the values. */

	public:
		/** The basic list iterator. */
		typedef T* iterator;

		/** The non-modifiable list iterator. */
		typedef const T* const_iterator;

	public:
		/** Default constructor. */
		vector() : reserved(0), sized(0), heapBuffer(NULL) {}

		/** Constructor. Builds a dynamically-sized array of the wanted size.
			@param size The wanted size of the array. */
		vector(size_t size) : reserved(0), sized(0), heapBuffer(NULL)
		{
			resize(size);
		}

		/** Constructor. Builds a dynamically-sized array of the wanted size.
			@param size The wanted size of the array
			@param defaultValue The default value to use for all the entries of the array. */
		vector(size_t size, const T& defaultValue) : reserved(0), sized(0), heapBuffer(NULL)
		{
			resize(size, defaultValue);
		}

		/** Copy constructor.
			@param copy The dynamically-sized array to copy the values from. */
		vector(const fm::vector<T,PRIMITIVE>& copy) : reserved(0), sized(0), heapBuffer(NULL)
		{
			insert(heapBuffer, copy.begin(), copy.size());
		}

		/** Constructor. Builds a dynamically-sized array from a constant-sized array.
			@param values A constant-sized array of floating-point values.
			@param count The size of the constant-sized array. */
		vector(const T* values, size_t count) : reserved(0), sized(0), heapBuffer(NULL)
		{
			insert(heapBuffer, values, count);
		}

		/** Destructor. */
		~vector()
		{
			if (!PRIMITIVE)
			{
				for (intptr_t i = sized - 1; i >= 0; --i)
				{
					heapBuffer[i].~T();
				}
			}
			if (heapBuffer != NULL)
			{
				fm::Release(heapBuffer);
			}
		}

		/** Retrieves a pointer to our internal buffer.
			Be very careful when using this.
			@return A pointer to our internal buffer. */
		inline T** GetDataPtr() { return &heapBuffer; }
		inline const T** GetDataPtr() const { return (const T**) &heapBuffer; } /**< See above. */

		/** Retrieves an iterator for a given element.
			@param value The value, contained within the list, to search for.
			@return An iterator to this element. The end() iterator
				is returned if the value is not found. */
		template <class Type2> iterator find(const Type2& value)
		{
			T* i = begin(),* e = end();
			for (; i != e; ++i) if ((*i) == value) break;
			return i;
		}
		template <class Type2> const_iterator find(const Type2& value) const
		{
			const T* i = begin(),* e = end();
			for (; i != e; ++i) if ((*i) == value) break;
			return i;
		} /**< See above. */

		/** Sorts this vector using the default comparisons operator. */
		inline void sort()
		{
			comparator<T> comp;
			comp.sort(heapBuffer, sized);
		}

		/** Sorts this vector using the given fm::comparator.
			@param comparator The comparator used to sort the vector.*/
		inline void sort(comparator<T>& comp)
		{
			comp.sort(heapBuffer, sized);
		}

		/** Removes the value at the given position within the list.
			@param it The list position for the value to remove. */
		iterator erase(iterator it)
		{
			FUAssert(it >= begin() && it < end(), return it);
			if (!PRIMITIVE) (*it).~T();
			if (end() - it - 1 > 0) memmove(it, it+1, (end() - it - 1) * sizeof(T));
			--sized;
			return it;
		}

		/** Removes a range of values from the list. The range is determined as every value
			between and including the first value, up to the last value, but not including the
			last value.
			@param first An iterator pointing to the first list value to remove.
			@param last An iterator past the last list value to remove. */
		void erase(iterator first, iterator last)
		{
			FUAssert(first >= begin() && first < end(), return);
			FUAssert(last > begin() && last <= end(), return);
			if (!PRIMITIVE) for (iterator it = first; it != last; ++it) (*it).~T();
			if (end() - last > 0) memmove(first, last, (end() - last) * sizeof(T));
			sized -= last - first;
		}

		/** Removes a range of values from the list.
			@param first The index of the first value to remove.
			@param last The index just passed the last value to remove. */
		inline void erase(size_t first, size_t last) { erase(begin() + first, begin() + last); }

		/** Removes a value contained within the list, once.
			@param value The value, contained within the list, to erase from it.
			@return Whether the value was found and erased from the list. */
		inline bool erase(const T& value) { iterator it = find(value); if (it != end()) { erase(it); return true; } return false; }

		/** Removes an indexed value contained within the list.
			@param index The index of the value to erase. */
		inline void erase(size_t index) { erase(begin() + index); }

		/** Retrieves whether the list contains a given value.
			@param value A value that could be contained in the list.
			@return Whether the list contains this value. */
		inline bool contains(const T& value) const { const_iterator it = find(value); return it != end(); }

		/** Replaces all cases of one value into another value.
			@param start The iterator at which to start replacing values.
			@param end The iterator at which to end replacing values.
			@param oldValue The value to replace.
			@param newValue The value which replaces the old value. */
		template <class V2, class V3>
		inline void replace(const V2& oldValue, const V3& newValue)
		{
			for (iterator start = begin(), stop = end(); start != stop; ++start)
			{
				if (*start == (T) oldValue) *start = newValue;
			}
		}

		/** Replaces the instances of one value into another value
			within a given segment of the list.
			@param start The iterator at which to start replacing values.
			@param end The iterator at which to end replacing values.
			@param oldValue The value to replace.
			@param newValue The value which replaces the old value. */
		template <class V2, class V3>
		inline void replace(iterator start, iterator end, const V2& oldValue, const V3& newValue)
		{
			while (start != end)
			{
				if (*start == (T) oldValue) *start = newValue;
				++start;
			}
		}

		/** Retrieves the number of values contained in the list.
			@return The number of values contained in the list. */
		inline size_t size() const { return sized; }

		/** Retrieves whether there are any elements in the list. */
		inline bool empty() const { return sized == 0; }

		/** Sets the number of values contained in the list.
			@param count The new number of values contained in the list. */
		void resize(size_t count)
		{
			reserve(count);

			if (!PRIMITIVE)
			{
				T* it = end();
				for (; sized < count; ++sized)
				{
					// For non-primitive types, make sure we call the constructors of all the values.
					fm::Construct(it++);
				}
			}
			else
			{
				sized = reserved;
			}
		}

		/** Sets the number of values contained in the list.
			@param count The new number of values contained in the list.
			@param value The value to assign to the new entries in the list. */
		void resize(size_t count, const T& value)
		{
			reserve(count);
			T* it = end();

			for (; sized < count; ++sized)
			{
				// For non-primitive types, make sure we call the constructors of all the values.
				if (!PRIMITIVE)
				{
					fm::Construct(it++, value);
				}
				else
				{
					*(it++) = value;
				}
			}
		}

		/** Removes all the element in the list. */
		inline void clear() { reserve(0); }

		/** Pre-allocate the list to a certain number of values.
			You can use reserve zero values in order to clear the memory
			used by this list. This function is useful when optimizing.
			@param count The new number of values pre-allocated in the list. */
		void reserve(size_t count)
		{
			// Basic check for stupidly large allocations;
			// basically all this is for is to catch (size_t)-1 calls
			// in debug mode. Ensure release will optimize out this call!
			FUAssert(count < INT_MAX, ;);
			if (reserved != count)
			{
				// For non-primitives, make sure we destroy all the values individually.
				if (PRIMITIVE)
				{
					if (sized > count) sized = count;
				}
				else
				{
					while (sized > count) pop_back();
				}

				// If we are reserving data, re-allocate a new buffer.
				T* newValues;
				if (count > 0)
				{
					newValues = (T*) fm::Allocate(count * sizeof(T));
					if (sized > 0)
					{
						memcpy(newValues, heapBuffer, sized * sizeof(T));
					}
				}
				else newValues = NULL;

				// Free the old buffer.
				if (heapBuffer != NULL)
				{
					fm::Release(heapBuffer);
				}
				heapBuffer = newValues;
				reserved = count;
			}
		}

		/** Retrieves the maximum size the array can grow to without allocating memory
			size is always less than or equal to capacity
			@return The number of values the array currently has memory allocated for */
		inline size_t capacity() { return reserved; }
		inline size_t capacity() const { return reserved; } /**< See above. */

		/** Retrieves the iterator for the first value in the list.
			@return The iterator for the first value in the list. */
		inline iterator begin() { return heapBuffer; }
		inline const_iterator begin() const { return heapBuffer; } /**< See above. */

		/** Retrieves the iterator just past the last value in the list.
			@return The iterator for just past the last value in the list. */
		inline iterator end() { return heapBuffer + sized; }
		inline const_iterator end() const { return heapBuffer + sized; } /**< See above. */

		/** Inserts a new item at a given position within the list.
			@param it An iterator pointing to where to insert the item.
			@param item The item to insert.
			@return An iterator pointing to the inserted item within the list. */
		iterator insert(iterator it, const T& item)
		{
			FUAssert(it >= begin() && it <= end(), return it);
			if (sized == reserved)
			{
				size_t offset = it - begin();
				reserve(sized + (sized > 31 ? 32 : (sized+1)));
				it = begin() + offset;
			}
			if (it < end())
			{
				memmove(it + 1, it, (end() - it) * sizeof(T));
			}
			if (!PRIMITIVE)
			{
				fm::Construct(it, item);
			}
			else
			{
				*it = item;
			}
			++sized;
			return it;
		}

		/** Inserts a new item at a given position within the list.
			@param index Where to insert the given value.
			@param item The item to insert. */
		inline void insert(size_t index, const T& item) { insert(begin() + index, item); }
		
		/** Inserts a new item at the end of the list.
			@param item The item to insert. */
		inline void push_back(const T& item) { insert(end(), item); }
		
		/** Inserts a new item at the front of the list.
			This operation is very expansive and not recommended
			for real-time operations.
			@param item The item to insert. */
		inline void push_front(const T& item) { insert(begin(), item); }

		/** Removes the last item from a list. */
		void pop_back()
		{
			FUAssert(sized > 0, return);
			if (!PRIMITIVE) (*(heapBuffer + sized - 1)).~T();
			--sized;
		}
		
		/** Removes the first item from a list.
			This operation is very expansive and not recommended
			for real-time operations. */
		inline void pop_front()
		{
			erase(begin());
		}

		/** Inserts multiple items at a given position within the list.
			@param it An iterator pointing to where to insert the items.
			@param first An iterator pointing to the first item to insert.
			@param last An iterator past the last item to insert. */
		template <typename _IT> inline void insert(iterator it, _IT first, _IT last)
		{
			size_t count = last - first;
			insert(it, first, count);
		}

		/** Inserts one item, multiple times at a given position within the list.
			@param it An iterator pointing to where to insert the item.
			@param count The number of times to insert the item.
			@param item The item to insert. */
		inline void insert(iterator it, size_t count, const T& item, bool noInit=false)
		{
			if (count > 0)
			{
				FUAssert(it >= begin() && it <= end(), return);
				if (sized + count > reserved)
				{
					size_t offset = it - begin();
					reserve(sized + count);
					it = begin() + offset;
				}
				if (it < end())
				{
					memmove(it + count, it, (end() - it) * sizeof(T));
				}
				sized += count;

				if (!noInit)
				{
					if (!PRIMITIVE)
					{
						do
						{
							fm::Construct(it++, item);
						}
						while (--count > 0);
					}
					else
					{
						do
						{
							*(it++) = item;
						}
						while (--count > 0);
					}
				}
			}
		}

		/** Inserts one value, multiple times, to this list.
			@param index Where to insert the value.
			@param count The number of times to insert this value.
			@param value The value to insert. */
		inline void insert(size_t index, size_t count, const T& value) { insert(begin() + index, count, value); }

		/** Inserts multiple items at a given position within the list,
			from a static list.
			@param it An iterator pointing to where to insert the items.
			@param first A pointer to the first item to insert.
			@param count The number of element within the static list
				that must be inserted within the list. */
		void insert(iterator it, const T* first, size_t count)
		{
			if (count > 0)
			{
				FUAssert(it >= begin() && it <= end(), return);
				if (sized + count > reserved)
				{
					size_t offset = it - begin();
					reserve((sized + count - reserved) > 32 ? (sized + count) : (reserved + 32));
					it = begin() + offset;
				}
				if (it < end())
				{
					memmove(it + count, it, (end() - it) * sizeof(T));
				}
				sized += count;
				if (!PRIMITIVE)
				{
					do
					{
						fm::Construct(it++, (*first++));
					}
					while (--count > 0);
				}
				else
				{
					memcpy(it, first, count * sizeof(T));
				}
			}
		}

		/** Inserts multiple values to this list.
			@param index Where to insert the values.
			@param values A static list of values.
			@param count The number of values to insert. */
		inline void insert(size_t index, const T* values, size_t count) { insert(begin() + index, values, count); }

		/** Retrieves the first element of the pointer array.
			@return The first element in the pointer array. */
		T& front() { FUAssert(sized > 0,); return (*heapBuffer); }
		const T& front() const { FUAssert(sized > 0,); return (*heapBuffer); } /**< See above. */

		/** Retrieves the last element of the pointer array.
			@return The last element in the pointer array. */
		T& back() { FUAssert(sized > 0,); return *(heapBuffer + sized - 1); }
		const T& back() const { FUAssert(sized > 0,); return *(heapBuffer + sized - 1); } /**< See above. */

		/** Retrieves an indexed object in the pointer array.
			@param index An index.
			@return The given object. */
		T& at(size_t index) { FUAssert(index < sized,); return heapBuffer[index]; }
		const T& at(size_t index) const { FUAssert(index < sized,); return heapBuffer[index]; } /**< See above. */
		template <class INTEGER> inline T& operator[](INTEGER index) { FUAssert((size_t) index < sized,); return heapBuffer[index]; } /**< See above. */
		template <class INTEGER> inline const T& operator[](INTEGER index) const { FUAssert((size_t) index < sized,); return heapBuffer[index]; } /**< See above. */

		/** Retrieves whether two lists are equivalent.
			@param other A second list.
			@return Whether the two lists are equivalent. */
		bool operator==(const fm::vector<T,PRIMITIVE>& other) const
		{
			bool equals = sized == other.size();
			const T* e = end();
			for (const T* it = begin(),* it2 = other.begin(); it != e && equals; ++it, ++it2)
			{
				equals = (*it) == (*it2);
			}
			return equals;
		}

		/** Copy operator. Copies the contents of one vector to another.
			@param rhs The vector to copy (RHS of operation).
			@return A reference to this (LHS of operation). */
		vector<T,PRIMITIVE>& operator =(const fm::vector<T,PRIMITIVE>& rhs)
		{
			if (this != &rhs)
			{
				if (PRIMITIVE)
				{
					resize(rhs.size()); 
					memcpy(begin(), rhs.begin(), sizeof(T) * rhs.size());
				}
				else
				{
					reserve(rhs.size());
					clear();
					for (const_iterator it = rhs.begin(); it != rhs.end(); ++it)
					{
						push_back(*it);
					}
				}
			}
			return *this;
		}
	};
};

/** Returns whether a dynamically-sized array is equivalent to a constant-sized array.
	@param dl A dynamically-sized array.
	@param cl A constant-sized array.
	@param count The size of the constant-sized array.
	@return Whether the two arrays are equivalent. */
template <class T, bool PRIMITIVE>
inline bool IsEquivalent(const fm::vector<T,PRIMITIVE>& dl, const T* cl, size_t count)
{
	if (dl.size() != count) return false;
	bool equivalent = true;
	for (size_t i = 0; i < count && equivalent; ++i)
	{
		 equivalent = IsEquivalent(dl.at(i), cl[i]);
	}
	return equivalent;
}

/** Returns whether two constant-sized arrays are equivalent.
	@param al A first constant-sized array.
	@param acount The number of elements in the first array.
	@param bl A second constant-sized array.
	@param bcount The number of elements in the second array.
	@return Whether the two arrays are equivalent. */
template <class T>
inline bool IsEquivalent(const T* al, size_t acount, const T* bl, size_t bcount)
{
	if (acount != bcount) return false;
	bool equivalent = true;
	for (size_t i = 0; i < acount && equivalent; ++i)
	{
		 equivalent = IsEquivalent(al[i], bl[i]);
	}
	return equivalent;
}

#ifdef WIN32
#pragma warning(default:4127)
#endif //WIN32
#endif // _FM_ARRAY_H_
