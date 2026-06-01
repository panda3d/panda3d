/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file epochHolder.h
 * @author Maxwell Dreytser
 * @date 2026-06-01
 */

#ifndef EPOCHHOLDER_H
#define EPOCHHOLDER_H

#include "pandabase.h"
#include "thread.h"

#ifndef CPPPARSER

/**
 * Scoped RAII guard that holds the calling thread inside an EBR critical
 * section (see Thread::epoch_enter) for its lifetime.
 *
 * Compiles to nothing when the pipeline is not threaded.
 */
class EpochHolder {
public:
  ALWAYS_INLINE explicit EpochHolder(Thread *current_thread = Thread::get_current_thread());
  ALWAYS_INLINE ~EpochHolder();
  EpochHolder(const EpochHolder &copy) = delete;
  EpochHolder &operator = (const EpochHolder &copy) = delete;

#ifdef THREADED_PIPELINE
private:
  Thread *_current_thread;
#endif
};

#include "epochHolder.I"

#endif  // CPPPARSER

#endif  // EPOCHHOLDER_H
