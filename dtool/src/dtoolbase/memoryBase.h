// Filename: memoryBase.h
// Created by:  drose (16Nov06)
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

#ifndef MEMORYBASE_H
#define MEMORYBASE_H

#include "dtoolbase.h"

// Place this macro within a class definition to define appropriate
// operator new and delete methods that hook into the MemoryInfo class
// to provide memory tracking.  Of course, it is better simply to
// inherit from MemoryBase; this macro is provided to resolve problems
// with multiple inheritance or some such.

#ifndef USE_MEMORY_NOWRAPPERS

#define ALLOC_MEMORY_BASE                                    \
  inline void *operator new(size_t size) {                   \
    return (*global_operator_new)(size);                     \
  }                                                          \
  inline void *operator new(size_t size, void *ptr) {        \
    return ptr;                                              \
  }                                                          \
  inline void operator delete(void *ptr) {                   \
    (*global_operator_delete)(ptr);                          \
  }                                                          \
  inline void operator delete(void *ptr, void *) {           \
  }                                                          \
  inline void *operator new[](size_t size) {                 \
    return (*global_operator_new)(size);                     \
  }                                                          \
  inline void *operator new[](size_t size, void *ptr) {      \
    return ptr;                                              \
  }                                                          \
  inline void operator delete[](void *ptr) {                 \
    (*global_operator_delete)(ptr);                          \
  }                                                          \
  inline void operator delete[](void *, void *) {            \
  }

#else   // USE_MEMORY_NOWRAPPERS

#define ALLOC_MEMORY_BASE

#endif  // USE_MEMORY_NOWRAPPERS

////////////////////////////////////////////////////////////////////
//       Class : MemoryBase
// Description : This class is intended to be the base class of all
//               objects in Panda that might be allocated and deleted
//               via the new and delete operators.  It redefines these
//               operators to provide some memory tracking support.
//
//               We used to try to override the global operator new
//               and delete methods, but that seems to cause problems
//               when including header files for C++-based system
//               libraries (such as are found on OSX).
////////////////////////////////////////////////////////////////////
class EXPCL_DTOOL MemoryBase {
public:
  ALLOC_MEMORY_BASE;
};

#endif


