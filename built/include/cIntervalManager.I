/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cIntervalManager.I
 * @author drose
 * @date 2002-09-10
 */

/**
 * Specifies a custom event queue to be used for throwing done events from
 * intervals as they finish.  If this is not specified, the global event queue
 * is used.
 *
 * The caller maintains ownership of the EventQueue object; it is the caller's
 * responsibility to ensure that the supplied EventQueue does not destruct
 * during the lifetime of the CIntervalManager.
 */
INLINE void CIntervalManager::
set_event_queue(EventQueue *event_queue) {
  _event_queue = event_queue;
}

/**
 * Returns the custom event queue to be used for throwing done events from
 * intervals as they finish.
 */
INLINE EventQueue *CIntervalManager::
get_event_queue() const {
  return _event_queue;
}

INLINE std::ostream &
operator << (std::ostream &out, const CIntervalManager &ival_mgr) {
  ival_mgr.output(out);
  return out;
}
