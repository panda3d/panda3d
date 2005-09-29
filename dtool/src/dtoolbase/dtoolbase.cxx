// Filename: dtoolbase.cxx
// Created by:  drose (12Sep00)
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

#include "dtoolbase.h"

/////////////////////////////////////////////////////////////////////
//
// Memory manager: DLMALLOC
//
// This is Doug Lea's memory manager.  It is very fast,
// but it is not thread-safe.
//
/////////////////////////////////////////////////////////////////////

#if defined(USE_MEMORY_DLMALLOC)

#define USE_DL_PREFIX 1
#define NO_MALLINFO 1
#include "dlmalloc.h"
#include "dlmalloc.c"

void *default_operator_new(size_t size) {
  void *ptr = dlmalloc(size);
  if (ptr == (void *)NULL) {
    cerr << "Out of memory!\n";
    abort();
  }
  return ptr;
}

void default_operator_delete(void *ptr) {
  dlfree(ptr);
}

void *(*global_operator_new)(size_t size) = &default_operator_new;
void (*global_operator_delete)(void *ptr) = &default_operator_delete;

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

#elif defined(USE_MEMORY_PTMALLOC2)

#define USE_DL_PREFIX 1
#include "ptmalloc2_smp.c"

void *default_operator_new(size_t size) {
  void *ptr = dlmalloc(size);
  if (ptr == (void *)NULL) {
    cerr << "Out of memory!\n";
    abort();
  }
  return ptr;
}

void default_operator_delete(void *ptr) {
  dlfree(ptr);
}

void *(*global_operator_new)(size_t size) = &default_operator_new;
void (*global_operator_delete)(void *ptr) = &default_operator_delete;

/////////////////////////////////////////////////////////////////////
//
// Memory manager: NONE
//
// This option uses the built-in system allocator.  This is a good
// choice on linux, but it's a terrible choice on windows.
//
/////////////////////////////////////////////////////////////////////

#else

void *default_operator_new(size_t size) {
  void *ptr = malloc(size);
  if (ptr == (void *)NULL) {
    cerr << "Out of memory!\n";
    abort();
  }
  return ptr;
}

void default_operator_delete(void *ptr) {
  free(ptr);
}

void *(*global_operator_new)(size_t size) = &default_operator_new;
void (*global_operator_delete)(void *ptr) = &default_operator_delete;

#endif
