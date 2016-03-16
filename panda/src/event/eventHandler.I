/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eventHandler.I
 * @author skyler
 * @date 2004-01-27
 */

/**
 * Returns a pointer to the one global EventHandler object.  If the global
 * object has not yet been created, this will create it.
 */
INLINE EventHandler *EventHandler::
get_global_event_handler(EventQueue *queue) {
  // The event queue parameter is present for now, for backward compatibility,
  // but it is ignored.
  if (_global_event_handler == 0) {
    make_global_event_handler();
  }
  return _global_event_handler;
}
