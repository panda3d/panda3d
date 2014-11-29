/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

/**
	@file FMSort.h
	The file contains the fm::comparator and fm::pcomparator classes.
 */

#ifndef _FM_SORT_H_
#define _FM_SORT_H_

namespace fm
{
	/** A utility function to swap data.*/
	template <class T>
	inline void swap(T& a, T& b)
	{
		T temp = a;
		a = b;
		b = temp;
	}

	/** A utility to sort arrays.*/
	template <class T>
	class comparator
	{
	public:
		/** Destructor. */
		virtual ~comparator() {}

		/** Compares two elements in ascending order, using operator<.
			Override this function to define your custom comparator.
			@param a The first element.
			@param b The second element.
			@return True if a is smaller than b, false otherwise.*/
		virtual bool compare(const T& a, const T& b)
		{
			return (a < b);
		}

		/** Sorts the given array using the quick sort algorithm.
			@param data The data array to sort.
			@param count The number of elements in the array.*/
		void sort(T* data, size_t count)
		{
			if (data == NULL) return;
			quicksort(data, 0, count);
		}

	private:
		void quicksort(T* data, intptr_t beg, intptr_t end)
		{
			if (end > beg + 1)
			{
				intptr_t l = beg + 1, r = end;
				T& pivot = data[beg];
				while (l < r)
				{
					if (compare(data[l], pivot))
						l++;
					else
						swap(data[l], data[--r]);
				}
				swap(data[--l], data[beg]);
				quicksort(data, beg, l);
				quicksort(data, r, end);
			}
		}
	};

	/** A utility to sort an array in descending order, using operator<.*/
	template <class T>
	class icomparator : public comparator<T>
	{
	public:
		/** Destructor. */
		virtual ~icomparator() {}

		/** Compares two elements in descending order, using operator<.
			Override this function to define your custom comparator.
			@param a The first element.
			@param b The second element.
			@return True if a is greater than b, false otherwise.*/
		virtual bool compare(const T& a, const T& b)
		{
			return (b < a);
		}
	};

	/** A utility to sort arrays of pointer elements.*/
	template <class T>
	class pcomparator : public comparator<const void*>
	{
	public:
		typedef const void* valueType;

		/** Destructor. */
		virtual ~pcomparator() {}

		/** Compares two pointers.
			Override this function to define your own pcomparator.
			@param a Element a.
			@param b Element b.
			@return True if a is smaller than b, false otherwise.*/
		virtual bool compare(const T* a, const T* b)
		{
			return (a < b);
		}

		/** [INTERNAL] Override from comparator<T>::compare().
			Do not override this function.
			@param a (void*) representation of an element.
			@param b (void*) representation of another element.
			@return The comparison casting a and b to (T*).*/
		virtual bool compare(const valueType& a, const valueType& b)
		{
			return compare((const T*)a, (const T*)b);
		}
	};
}

#endif // _FM_SORT_H_
