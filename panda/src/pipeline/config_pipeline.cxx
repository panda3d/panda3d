/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_pipeline.cxx
 * @author drose
 * @date 2006-03-28
 */

#include "config_pipeline.h"
#include "cycleData.h"
#include "mainThread.h"
#include "externalThread.h"
#include "genericThread.h"
#include "thread.h"
#include "pandaSystem.h"

#include "dconfig.h"

#if !defined(CPPPARSER) && !defined(LINK_ALL_STATIC) && !defined(BUILDING_PANDA_PIPELINE)
  #error Buildsystem error: BUILDING_PANDA_PIPELINE not defined
#endif

ConfigureDef(config_pipeline);
NotifyCategoryDef(pipeline, "");
NotifyCategoryDef(thread, "");

ConfigureFn(config_pipeline) {
  init_libpipeline();
}

ConfigVariableBool support_threads
("support-threads", true,
 PRC_DESC("Set this false to disallow the creation of threads using Panda's "
          "Thread interface, even if threading support is compiled in.  This "
          "does not affect the operation of mutexes and other synchronization "
          "primitives, just the creation of threads."));

ConfigVariableBool name_deleted_mutexes
("name-deleted-mutexes", false,
 PRC_DESC("Set this true to allocate a name to each Mutex object that "
          "destructs, so if the Mutex is locked after destruction, we can "
          "print out its name to aid debugging.  This is only available "
          "when compiled with DEBUG_THREADS.  Enabling this variable will "
          "cause a memory leak, so you should only enable it when you are "
          "specifically tracking down an operation on a deleted Mutex.  "
          "It is not guaranteed to work, of course, because the memory "
          "for a deleted Mutex may become reused for some other purpose."));

ConfigVariableInt thread_stack_size
("thread-stack-size", 4194304,
 PRC_DESC("Specifies the minimum size, in bytes, of the stack that will be "
          "created for each newly-created thread.  Not all thread "
          "implementations respect this value."));

/**
 * Initializes the library.  This must be called at least once before any of
 * the functions or classes in this library can be used.  Normally it will be
 * called by the static initializers and need not be called explicitly, but
 * special cases exist.
 */
void
init_libpipeline() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

#ifdef DO_PIPELINING
  CycleData::init_type();
#endif

  MainThread::init_type();
  ExternalThread::init_type();
  GenericThread::init_type();
  Thread::init_type();

#ifdef HAVE_THREADS
 {
  PandaSystem *ps = PandaSystem::get_global_ptr();
  ps->add_system("threads");
  }
#endif  // HAVE_THREADS
}
