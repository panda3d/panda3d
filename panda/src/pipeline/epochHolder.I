/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file epochHolder.I
 * @author Maxwell Dreytser
 * @date 2026-06-01
 */

ALWAYS_INLINE EpochHolder::
EpochHolder(Thread *current_thread) {
#ifdef THREADED_PIPELINE
  _current_thread = current_thread;
  _current_thread->epoch_enter();
#else
  (void)current_thread;
#endif
}

ALWAYS_INLINE EpochHolder::
~EpochHolder() {
#ifdef THREADED_PIPELINE
  _current_thread->epoch_leave();
#endif
}
