/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

/**
	@file FMAllocator.h
	The file contains the simple_allocator class and some export memory management functions.
 */

#ifndef _FM_ALLOCATOR_H_
#define _FM_ALLOCATOR_H_

#include <new>

#ifdef new
#define _OLD_NEW new
#undef new
#endif // new

namespace fm
{
	/** An allocation function type.
		@param size The size, in bytes, to allocate.
		@return The allocated buffer or NULL if not enough memory is available.*/
	typedef void* (*AllocateFunc)(size_t size);

	/** A deallocation function type.
		@param buffer The memory buffer to deallocate.*/
	typedef void (*FreeFunc)(void* buffer);

	/** Sets the FCollada memory allocation / deallocation functions.
		@param a The Allocation function. Defaults to malloc.
		@param f The Deallocation function. Defaults to free.*/
	FCOLLADA_EXPORT void SetAllocationFunctions(AllocateFunc a, FreeFunc f);

	/** Allocates a requested amount of memory.
		@param byteCount The amount of memory to allocate, in bytes.
		@return A pointer to the memory address. This pointer will be NULL if there is not
			enough memory to allocate. */
	FCOLLADA_EXPORT void* Allocate(size_t byteCount);

	/** Releases a memory buffer.
		@param buffer The memory buffer to release. */
	FCOLLADA_EXPORT void Release(void* buffer);

	/** Construct the object at a given pointer.
		@param o A pointer to the object. */
	template <class Type1>
	inline void Construct(Type1* o)
	{
		::new (o) Type1;
	}

	/** Construct the object at a given pointer.
		@param o A pointer to the object.
		@param value The value to copy. */
	template <class Type1, class Type2>
	inline void Construct(Type1* o, const Type2& value)
	{
		::new (o) Type1(value);
	}
};

#ifdef _OLD_NEW
#define new _OLD_NEW
#undef _OLD_NEW
#endif // _OLD_NEW

#endif // _FM_ALLOCATOR_H_
