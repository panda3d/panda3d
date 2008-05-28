// Filename: dtoolbase.cxx
// Created by:  drose (12Sep00)
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

#include "dtoolbase.h"
#include "memoryHook.h"

#if defined(USE_TAU) && defined(WIN32)
// Hack around tau's lack of DLL export declarations for Profiler class.
bool __tau_shutdown = false;
#endif

MemoryHook *memory_hook;

////////////////////////////////////////////////////////////////////
//     Function: init_memory_hook
//  Description: Any code that might need to use PANDA_MALLOC or
//               PANDA_FREE, or any methods of the global memory_hook
//               object, at static init time, should ensure that it
//               calls init_memory_hook() first to ensure that the
//               pointer has been properly initialized.  There is no
//               harm in calling this function more than once.
//
//               There is no need to call this function other than at
//               static init time.
////////////////////////////////////////////////////////////////////
void
init_memory_hook() {
  if (memory_hook == NULL) {
    memory_hook = new MemoryHook;
  }
}

// Here's a quick way to ensure the above function is called at least
// once at static init time.
class InitMemoryHook {
public:
  InitMemoryHook() {
    init_memory_hook();
  }
};
static InitMemoryHook _imh_object;

#if defined(HAVE_THREADS) && defined(SIMPLE_THREADS)

static void
default_thread_yield() {
}
static void
default_thread_consider_yield() {
}
void (*global_thread_yield)() = default_thread_yield;
void (*global_thread_consider_yield)() = default_thread_consider_yield;

#endif  // HAVE_THREADS && SIMPLE_THREADS
