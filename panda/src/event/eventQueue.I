/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eventQueue.I
 * @author drose
 * @date 2000-05-05
 */

/**
 * Returns a pointer to the one global EventQueue object.  If the global
 * object has not yet been created, this will create it.
 */
INLINE EventQueue *EventQueue::
get_global_event_queue() {
  if (_global_event_queue == nullptr) {
    make_global_event_queue();
  }
  return _global_event_queue;
}
