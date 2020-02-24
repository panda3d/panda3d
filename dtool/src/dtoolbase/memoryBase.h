/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file memoryBase.h
 * @author drose
 * @date 2006-11-16
 */

#ifndef MEMORYBASE_H
#define MEMORYBASE_H

#include "dtoolbase.h"
#include "memoryHook.h"

// Place this macro within a class definition to define appropriate operator
// new and delete methods that hook into the MemoryInfo class to provide
// memory tracking.  Of course, it is better simply to inherit from
// MemoryBase; this macro is provided to resolve problems with multiple
// inheritance or some such.

#define ALLOC_MEMORY_BASE                                    \
  inline void *operator new(size_t size) RETURNS_ALIGNED(MEMORY_HOOK_ALIGNMENT) { \
    return PANDA_MALLOC_SINGLE(size);                        \
  }                                                          \
  inline void *operator new(size_t size, void *ptr) {        \
    (void) size;                                             \
    return ptr;                                              \
  }                                                          \
  inline void operator delete(void *ptr) {                   \
    PANDA_FREE_SINGLE(ptr);                                  \
  }                                                          \
  inline void operator delete(void *, void *) {              \
  }                                                          \
  inline void *operator new[](size_t size) RETURNS_ALIGNED(MEMORY_HOOK_ALIGNMENT) { \
    return PANDA_MALLOC_ARRAY(size);                         \
  }                                                          \
  inline void *operator new[](size_t size, void *ptr) {      \
    (void) size;                                             \
    return ptr;                                              \
  }                                                          \
  inline void operator delete[](void *ptr) {                 \
    PANDA_FREE_ARRAY(ptr);                                   \
  }                                                          \
  inline void operator delete[](void *, void *) {            \
  }

/**
 * This class is intended to be the base class of all objects in Panda that
 * might be allocated and deleted via the new and delete operators.  It
 * redefines these operators to provide some memory tracking support.
 *
 * We used to try to override the global operator new and delete methods, but
 * that seems to cause problems when including header files for C++-based
 * system libraries (such as are found on OSX).
 */
class EXPCL_DTOOL_DTOOLBASE MemoryBase {
public:
  ALLOC_MEMORY_BASE;
};

#endif
