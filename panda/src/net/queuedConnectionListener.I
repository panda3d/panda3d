/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file queuedConnectionListener.I
 * @author drose
 * @date 2000-05-17
 */

/**
 *
 */
INLINE bool ConnectionListenerData::
operator == (const ConnectionListenerData &) const {
  return true;
}

/**
 *
 */
INLINE bool ConnectionListenerData::
operator != (const ConnectionListenerData &) const {
  return false;
}

/**
 *
 */
INLINE bool ConnectionListenerData::
operator < (const ConnectionListenerData &) const {
  return false;
}
