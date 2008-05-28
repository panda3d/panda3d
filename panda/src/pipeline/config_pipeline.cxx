// Filename: config_pipeline.cxx
// Created by:  drose (28Mar06)
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

#include "config_pipeline.h"
#include "mainThread.h"
#include "externalThread.h"
#include "thread.h"
#include "pythonThread.h"
#include "pandaSystem.h"

#include "dconfig.h"

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

ConfigVariableInt thread_stack_size
("thread-stack-size", 4194304,
 PRC_DESC("Specifies the minimum size, in bytes, of the stack that will be "
          "created for each newly-created thread.  Not all thread "
          "implementations respect this value."));

////////////////////////////////////////////////////////////////////
//     Function: init_libpipeline
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_libpipeline() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  MainThread::init_type();
  ExternalThread::init_type();
  Thread::init_type();
#ifdef HAVE_PYTHON
  PythonThread::init_type();
#endif  // HAVE_PYTHON

#ifdef HAVE_THREADS
 {
  PandaSystem *ps = PandaSystem::get_global_ptr();
  ps->add_system("threads");
  }
#endif  // HAVE_THREADS
}
