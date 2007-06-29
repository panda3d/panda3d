// Filename: memoryHook.cxx
// Created by:  drose (28Jun07)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "memoryHook.h"

#ifdef WIN32

// Windows case.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#else

// Posix case.
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>

#endif  // WIN32


#if defined(USE_MEMORY_DLMALLOC)

/////////////////////////////////////////////////////////////////////
//
// Memory manager: DLMALLOC
//
// This is Doug Lea's memory manager.  It is very fast,
// but it is not thread-safe.
//
/////////////////////////////////////////////////////////////////////

#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)
#error Cannot use dlmalloc library with threading enabled!
#endif

#define USE_DL_PREFIX 1
#define NO_MALLINFO 1
#include "dlmalloc.h"
#include "dlmalloc_src.cxx"

#define call_malloc dlmalloc
#define call_realloc dlrealloc
#define call_free dlfree

#elif defined(USE_MEMORY_PTMALLOC2) && !defined(linux)
// This doesn't appear to work in Linux; perhaps it is clashing with
// the system library.  On Linux, fall through to the next case
// instead.

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
#include "ptmalloc2_smp_src.cxx"

#define call_malloc dlmalloc
#define call_realloc dlrealloc
#define call_free dlfree

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
  _page_size = getpagesize();

#endif  // WIN32

#ifdef DO_MEMORY_USAGE
  _total_heap_single_size = 0;
  _total_heap_array_size = 0;
  _requested_heap_size = 0;
  _total_mmap_size = 0;
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
#endif
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
#ifdef DO_MEMORY_USAGE
  // In the DO_MEMORY_USAGE case, we want to track the total size of
  // allocated bytes on the heap.
  AtomicAdjust::add(_total_heap_single_size, (PN_int32)size);
#endif  // DO_MEMORY_USAGE

  void *alloc = call_malloc(inflate_size(size));
  if (alloc == (void *)NULL) {
    cerr << "Out of memory!\n";
    abort();
  }

  return alloc_to_ptr(alloc, size);
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
  assert(size <= _total_heap_single_size);
  AtomicAdjust::add(_total_heap_single_size, -(PN_int32)size);
#endif  // DO_MEMORY_USAGE

  call_free(alloc);
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
#ifdef DO_MEMORY_USAGE
  // In the DO_MEMORY_USAGE case, we want to track the total size of
  // allocated bytes on the heap.
  AtomicAdjust::add(_total_heap_array_size, (PN_int32)size);
#endif  // DO_MEMORY_USAGE

  void *alloc = call_malloc(inflate_size(size));
  if (alloc == (void *)NULL) {
    cerr << "Out of memory!\n";
    abort();
  }

  return alloc_to_ptr(alloc, size);
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
  assert(orig_size <= _total_heap_array_size);
  AtomicAdjust::add(_total_heap_array_size, (PN_int32)size-(PN_int32)orig_size);
#endif  // DO_MEMORY_USAGE

  alloc = call_realloc(alloc, inflate_size(size));
  if (alloc == (void *)NULL) {
    cerr << "Out of memory!\n";
    abort();
  }

  return alloc_to_ptr(alloc, size);
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
  assert(size <= _total_heap_array_size);
  AtomicAdjust::add(_total_heap_array_size, -(PN_int32)size);
#endif  // DO_MEMORY_USAGE

  call_free(alloc);
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
#if defined(USE_MEMORY_DLMALLOC) || (defined(USE_MEMORY_PTMALLOC2) && !defined(linux))
  return dlmalloc_trim(pad);
#else
  // Since malloc_trim() isn't standard C, we can't be sure it exists
  // on a given platform.
  return 0;
#endif
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
  assert(size <= _total_mmap_size);
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
