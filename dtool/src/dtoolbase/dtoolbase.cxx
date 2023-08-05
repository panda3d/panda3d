/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dtoolbase.cxx
 * @author drose
 * @date 2000-09-12
 */

#include "dtoolbase.h"
#include "memoryHook.h"

#if !defined(CPPPARSER) && !defined(LINK_ALL_STATIC) && !defined(BUILDING_DTOOL_DTOOLBASE)
  #error Buildsystem error: BUILDING_DTOOL_DTOOLBASE not defined
#endif

#if defined(USE_TAU) && defined(_WIN32)
// Hack around tau's lack of DLL export declarations for Profiler class.
bool __tau_shutdown = false;
#endif

MemoryHook default_memory_hook;
MemoryHook *memory_hook = &default_memory_hook;

/**
 * It used to be necessary to call this before using operator new at static
 * init time.  There is no need to call this function anymore.
 */
void
init_memory_hook() {
  assert(memory_hook != nullptr);
}

#if defined(HAVE_THREADS) && defined(SIMPLE_THREADS)

static void
default_thread_yield() {
}
static void
default_thread_consider_yield() {
}
void (*global_thread_yield)() = default_thread_yield;
void (*global_thread_consider_yield)() = default_thread_consider_yield;

#ifdef HAVE_PYTHON
static PyThreadState *
default_thread_state_swap(PyThreadState *state) {
  return nullptr;
}
PyThreadState *(*global_thread_state_swap)(PyThreadState *tstate) = default_thread_state_swap;
#endif  // HAVE_PYTHON

#endif  // HAVE_THREADS && SIMPLE_THREADS
