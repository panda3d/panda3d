// Filename: memoryHook.cxx
// Created by:  drose (28Jun07)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "memoryHook.h"
#include "deletedBufferChain.h"
#include <stdlib.h>

#ifdef WIN32

// Windows case.
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#include <windows.h>

#else

// Posix case.
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>

#ifndef MAP_ANON
#define MAP_ANON 0x1000
#endif

#endif  // WIN32


#if defined(USE_MEMORY_DLMALLOC)

/////////////////////////////////////////////////////////////////////
//
// Memory manager: DLMALLOC
//
// This is Doug Lea's memory manager.  It is very fast, but it is not
// thread-safe.  However, we provide thread locking within MemoryHook.
//
/////////////////////////////////////////////////////////////////////

#define USE_DL_PREFIX 1
#define NO_MALLINFO 1
#ifdef _DEBUG
  #define DEBUG 1
#endif
#ifdef assert
  // dlmalloc defines its own assert, which clashes.
  #undef assert
#endif
#include "dlmalloc.h"
#include "dlmalloc_src.cxx"

#define call_malloc dlmalloc
#define call_realloc dlrealloc
#define call_free dlfree
#define MEMORY_HOOK_MALLOC_LOCK 1

#elif defined(USE_MEMORY_PTMALLOC2)
// This doesn't appear to work in Linux; perhaps it is clashing with
// the system library.  It also doesn't appear to be thread-safe on
// OSX.

/////////////////////////////////////////////////////////////////////
//
// Memory manager: PTMALLOC2
//
// Ptmalloc2 is a derivative of Doug Lea's memory manager that was
// made thread-safe by Wolfram Gloger, then was ported to windows by
// Niall Douglas.  It is not quite as fast as dlmalloc (because the
// thread-safety constructs take a certain amount of CPU time), but
// it's still much faster than the windows allocator.
//
/////////////////////////////////////////////////////////////////////

#define USE_DL_PREFIX 1
#define NO_MALLINFO 1
#ifdef _DEBUG
  #define MALLOC_DEBUG 2
#endif
#include "ptmalloc2_smp_src.cxx"

#define call_malloc dlmalloc
#define call_realloc dlrealloc
#define call_free dlfree
#undef MEMORY_HOOK_MALLOC_LOCK

#else

/////////////////////////////////////////////////////////////////////
//
// Memory manager: MALLOC
//
// This option uses the built-in system allocator.  This is a good
// choice on linux, but it's a terrible choice on windows.
//
/////////////////////////////////////////////////////////////////////

#define call_malloc malloc
#define call_realloc realloc
#define call_free free
#undef MEMORY_HOOK_MALLOC_LOCK

#endif  // USE_MEMORY_*

////////////////////////////////////////////////////////////////////
//     Function: MemoryHook::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
MemoryHook::
MemoryHook() {
#ifdef WIN32

  // Windows case.
  SYSTEM_INFO sysinfo;
  GetSystemInfo(&sysinfo);

  _page_size = (size_t)sysinfo.dwPageSize;

#else

  // Posix case.
  _page_size = sysconf(_SC_PAGESIZE);

#endif  // WIN32

#ifdef DO_MEMORY_USAGE
  _total_heap_single_size = 0;
  _total_heap_array_size = 0;
  _requested_heap_size = 0;
  _total_mmap_size = 0;
  _max_heap_size = ~(size_t)0;
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryHook::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
MemoryHook::
MemoryHook(const MemoryHook &copy) :
  _page_size(copy._page_size)
{
#ifdef DO_MEMORY_USAGE
  _total_heap_single_size = copy._total_heap_single_size;
  _total_heap_array_size = copy._total_heap_array_size;
  _requested_heap_size = copy._requested_heap_size;
  _total_mmap_size = copy._total_mmap_size;
  _max_heap_size = copy._max_heap_size;
#endif

  ((MutexImpl &)copy._lock).acquire();
  _deleted_chains = copy._deleted_chains;
  ((MutexImpl &)copy._lock).release();
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryHook::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
MemoryHook::
~MemoryHook() {
  // Really, we only have this destructor to shut up gcc about the
  // virtual functions warning.
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryHook::heap_alloc_single
//       Access: Public, Virtual
//  Description: Allocates a block of memory from the heap, similar to
//               malloc().  This will never return NULL; it will abort
//               instead if memory is not available.
//
//               This particular function should be used to allocate
//               memory for a single object, as opposed to an array.
//               The only difference is in the bookkeeping.
////////////////////////////////////////////////////////////////////
void *MemoryHook::
heap_alloc_single(size_t size) {
  size_t inflated_size = inflate_size(size);

#ifdef MEMORY_HOOK_MALLOC_LOCK
  _lock.acquire();
  void *alloc = call_malloc(inflated_size);
  _lock.release();
#else
  void *alloc = call_malloc(inflated_size);
#endif

  while (alloc == (void *)NULL) {
    alloc_fail(inflated_size);
#ifdef MEMORY_HOOK_MALLOC_LOCK
    _lock.acquire();
    alloc = call_malloc(inflated_size);
    _lock.release();
#else
    alloc = call_malloc(inflated_size);
#endif
  }

#ifdef DO_MEMORY_USAGE
  // In the DO_MEMORY_USAGE case, we want to track the total size of
  // allocated bytes on the heap.
  AtomicAdjust::add(_total_heap_single_size, (AtomicAdjust::Integer)size);
  if ((size_t)AtomicAdjust::get(_total_heap_single_size) + 
      (size_t)AtomicAdjust::get(_total_heap_array_size) >
      _max_heap_size) {
    overflow_heap_size();
  }
#endif  // DO_MEMORY_USAGE

  void *ptr = alloc_to_ptr(alloc, size);
  assert(ptr >= alloc && (char *)ptr + size <= (char *)alloc + inflated_size);
  return ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryHook::heap_free_single
//       Access: Public, Virtual
//  Description: Releases a block of memory previously allocated via
//               heap_alloc_single.
////////////////////////////////////////////////////////////////////
void MemoryHook::
heap_free_single(void *ptr) {
  size_t size;
  void *alloc = ptr_to_alloc(ptr, size);

#ifdef DO_MEMORY_USAGE
  assert((int)size <= _total_heap_single_size);
  AtomicAdjust::add(_total_heap_single_size, -(AtomicAdjust::Integer)size);
#endif  // DO_MEMORY_USAGE

#ifdef MEMORY_HOOK_MALLOC_LOCK
  _lock.acquire();
  call_free(alloc);
  _lock.release();
#else
  call_free(alloc);
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryHook::heap_alloc_array
//       Access: Public, Virtual
//  Description: Allocates a block of memory from the heap, similar to
//               malloc().  This will never return NULL; it will abort
//               instead if memory is not available.
//
//               This particular function should be used to allocate
//               memory for an array of objects, as opposed to a
//               single object.  The only difference is in the
//               bookkeeping.
////////////////////////////////////////////////////////////////////
void *MemoryHook::
heap_alloc_array(size_t size) {
  size_t inflated_size = inflate_size(size);

#ifdef MEMORY_HOOK_MALLOC_LOCK
  _lock.acquire();
  void *alloc = call_malloc(inflated_size);
  _lock.release();
#else
  void *alloc = call_malloc(inflated_size);
#endif

  while (alloc == (void *)NULL) {
    alloc_fail(inflated_size);
#ifdef MEMORY_HOOK_MALLOC_LOCK
    _lock.acquire();
    alloc = call_malloc(inflated_size);
    _lock.release();
#else
    alloc = call_malloc(inflated_size);
#endif
  }

#ifdef DO_MEMORY_USAGE
  // In the DO_MEMORY_USAGE case, we want to track the total size of
  // allocated bytes on the heap.
  AtomicAdjust::add(_total_heap_array_size, (AtomicAdjust::Integer)size);
  if ((size_t)AtomicAdjust::get(_total_heap_single_size) + 
      (size_t)AtomicAdjust::get(_total_heap_array_size) >
      _max_heap_size) {
    overflow_heap_size();
  }
#endif  // DO_MEMORY_USAGE

  void *ptr = alloc_to_ptr(alloc, size);
  assert(ptr >= alloc && (char *)ptr + size <= (char *)alloc + inflated_size);
  return ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryHook::heap_realloc_array
//       Access: Public, Virtual
//  Description: Resizes a block of memory previously returned from
//               heap_alloc_array.
////////////////////////////////////////////////////////////////////
void *MemoryHook::
heap_realloc_array(void *ptr, size_t size) {
  size_t orig_size;
  void *alloc = ptr_to_alloc(ptr, orig_size);

#ifdef DO_MEMORY_USAGE
  assert((AtomicAdjust::Integer)orig_size <= _total_heap_array_size);
  AtomicAdjust::add(_total_heap_array_size, (AtomicAdjust::Integer)size-(AtomicAdjust::Integer)orig_size);
#endif  // DO_MEMORY_USAGE

  size_t inflated_size = inflate_size(size);

  void *alloc1 = alloc;
#ifdef MEMORY_HOOK_MALLOC_LOCK
  _lock.acquire();
  alloc1 = call_realloc(alloc1, inflated_size);
  _lock.release();
#else
  alloc1 = call_realloc(alloc1, inflated_size);
#endif

  while (alloc1 == (void *)NULL) {
    alloc_fail(inflated_size);
    
    // Recover the original pointer.
    alloc1 = alloc;

#ifdef MEMORY_HOOK_MALLOC_LOCK
    _lock.acquire();
    alloc1 = call_realloc(alloc1, inflated_size);
    _lock.release();
#else
    alloc1 = call_realloc(alloc1, inflated_size);
#endif
  }

  void *ptr1 = alloc_to_ptr(alloc1, size);
  assert(ptr1 >= alloc1 && (char *)ptr1 + size <= (char *)alloc1 + inflated_size);
#if defined(MEMORY_HOOK_DO_ALIGN)
  // We might have to shift the memory to account for the new offset
  // due to the alignment.
  size_t orig_delta = (char *)ptr - (char *)alloc;
  size_t new_delta = (char *)ptr1 - (char *)alloc1;
  if (orig_delta != new_delta) {
    memmove((char *)alloc1 + new_delta, (char *)alloc1 + orig_delta, min(size, orig_size));
  }
#endif  // MEMORY_HOOK_DO_ALIGN
  return ptr1;
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryHook::heap_free_array
//       Access: Public, Virtual
//  Description: Releases a block of memory previously allocated via
//               heap_alloc_array.
////////////////////////////////////////////////////////////////////
void MemoryHook::
heap_free_array(void *ptr) {
  size_t size;
  void *alloc = ptr_to_alloc(ptr, size);

#ifdef DO_MEMORY_USAGE
  assert((int)size <= _total_heap_array_size);
  AtomicAdjust::add(_total_heap_array_size, -(AtomicAdjust::Integer)size);
#endif  // DO_MEMORY_USAGE

#ifdef MEMORY_HOOK_MALLOC_LOCK
  _lock.acquire();
  call_free(alloc);
  _lock.release();
#else
  call_free(alloc);
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryHook::heap_trim
//       Access: Public
//  Description: Attempts to release memory back to the system, if
//               possible.  The pad argument is the minimum amount of
//               unused memory to keep in the heap (against future
//               allocations).  Any memory above that may be released
//               to the system, reducing the memory size of this
//               process.  There is no guarantee that any memory may
//               be released.
//
//               Returns true if any memory was actually released,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool MemoryHook::
heap_trim(size_t pad) {
  bool trimmed = false;

#if defined(USE_MEMORY_DLMALLOC) || defined(USE_MEMORY_PTMALLOC2)
  // Since malloc_trim() isn't standard C, we can't be sure it exists
  // on a given platform.  But if we're using dlmalloc, we know we
  // have dlmalloc_trim.
  _lock.acquire();
  if (dlmalloc_trim(pad)) {
    trimmed = true;
  }
  _lock.release();
#endif

#ifdef WIN32
  // Also, on Windows we have _heapmin().
  if (_heapmin() == 0) {
    trimmed = true;
  }
#endif

  return trimmed;
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryHook::mmap_alloc
//       Access: Public, Virtual
//  Description: Allocates a raw page or pages of memory directly from
//               the OS.  This will be in a different address space
//               from the memory allocated by heap_alloc(), and so it
//               won't contribute to fragmentation of that memory.
//
//               The allocation size must be an integer multiple of
//               the page size.  Use round_to_page_size() if there is
//               any doubt.
//
//               If allow_exec is true, the memory will be flagged so
//               that it is legal to execute code that has been
//               written to this memory.
////////////////////////////////////////////////////////////////////
void *MemoryHook::
mmap_alloc(size_t size, bool allow_exec) {
  assert((size % _page_size) == 0);

#ifdef DO_MEMORY_USAGE
  _total_mmap_size += size;
#endif

#ifdef WIN32

  // Windows case.
  void *ptr = VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE,
                           allow_exec ? PAGE_EXECUTE_READWRITE : PAGE_READWRITE);
  if (ptr == (void *)NULL) {
    DWORD err = GetLastError();
    cerr << "Couldn't allocate memory page of size " << size << ": ";

    PVOID buffer;
    DWORD length = 
      FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                    NULL, err, 0, (LPTSTR)&buffer, 0, NULL);
    if (length != 0) {
      cerr << (char *)buffer << "\n";
    } else {
      cerr << "Error code " << err << "\n";
    }
    LocalFree(buffer);
    abort();
  }

  return ptr;

#else

  // Posix case.
  int prot = PROT_READ | PROT_WRITE;
  if (allow_exec) {
    prot |= PROT_EXEC;
  }
  void *ptr = mmap(NULL, size, prot, MAP_PRIVATE | MAP_ANON, -1, 0);
  if (ptr == (void *)-1) {
    perror("mmap");
    abort();
  }

  return ptr;

#endif  // WIN32
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryHook::mmap_free
//       Access: Public, Virtual
//  Description: Frees a block of memory previously allocated via
//               mmap_alloc().  You must know how large the block was.
////////////////////////////////////////////////////////////////////
void MemoryHook::
mmap_free(void *ptr, size_t size) {
  assert((size % _page_size) == 0);

#ifdef DO_MEMORY_USAGE
  assert((int)size <= _total_mmap_size);
  _total_mmap_size -= size;
#endif

#ifdef WIN32
  VirtualFree(ptr, 0, MEM_RELEASE);
#else  
  munmap(ptr, size);
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryHook::mark_pointer
//       Access: Public, Virtual
//  Description: This special method exists only to provide a callback
//               hook into MemoryUsage.  It indicates that the
//               indicated pointer, allocated from somewhere other
//               than a call to heap_alloc(), now contains a pointer
//               to the indicated ReferenceCount object.  If orig_size
//               is 0, it indicates that the ReferenceCount object has
//               been destroyed.
////////////////////////////////////////////////////////////////////
void MemoryHook::
mark_pointer(void *, size_t, ReferenceCount *) {
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryHook::get_deleted_chain
//       Access: Public
//  Description: Returns a pointer to a global DeletedBufferChain
//               object suitable for allocating arrays of the
//               indicated size.  There is one unique
//               DeletedBufferChain object for every different size.
////////////////////////////////////////////////////////////////////
DeletedBufferChain *MemoryHook::
get_deleted_chain(size_t buffer_size) {
  DeletedBufferChain *chain;

  _lock.acquire();
  DeletedChains::iterator dci = _deleted_chains.find(buffer_size);
  if (dci != _deleted_chains.end()) {
    chain = (*dci).second;
  } else {
    // Once allocated, this DeletedBufferChain object is never deleted.
    chain = new DeletedBufferChain(buffer_size);
    _deleted_chains.insert(DeletedChains::value_type(buffer_size, chain));
  }
  
  _lock.release();
  return chain;
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryHook::alloc_fail
//       Access: Protected, Virtual
//  Description: This callback method is called whenever a low-level
//               call to call_malloc() has returned NULL, indicating
//               failure.
//
//               Since this method is called very low-level, and may
//               be in the middle of any number of critical sections,
//               it will be difficult for this callback initiate any
//               emergency high-level operation to make more memory
//               available.  However, this module is set up to assume
//               that that's what this method does, and will make
//               another alloc attempt after it returns.  Probably the
//               only sensible thing this method can do, however, is
//               just to display a message and abort.
////////////////////////////////////////////////////////////////////
void MemoryHook::
alloc_fail(size_t attempted_size) {
  cerr << "Out of memory allocating " << attempted_size << " bytes\n";
  abort();
}

#ifdef DO_MEMORY_USAGE
////////////////////////////////////////////////////////////////////
//     Function: MemoryHook::overflow_heap_size
//       Access: Protected, Virtual
//  Description: This callback method is called whenever the total
//               allocated heap size exceeds _max_heap_size.  It's
//               mainly intended for reporting memory leaks, on the
//               assumption that once we cross some specified
//               threshold, we're just leaking memory.
//
//               The implementation for this method is in MemoryUsage.
////////////////////////////////////////////////////////////////////
void MemoryHook::
overflow_heap_size() {
  _max_heap_size = ~(size_t)0;
}
#endif  // DO_MEMORY_USAGE
