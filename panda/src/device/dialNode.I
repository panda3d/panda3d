/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dialNode.I
 * @author drose
 * @date 2002-03-12
 */

/**
 * Returns true if the DialNode is valid and connected to a server, false
 * otherwise.
 */
INLINE bool DialNode::
is_valid() const {
  return (_dial != nullptr) && _dial->is_connected();
}

/**
 * Returns the number of dial dials known to the DialNode.  This number may
 * change as more dials are discovered.
 */
INLINE int DialNode::
get_num_dials() const {
  return _dial->get_num_dials();
}

/**
 * Returns the number of complete revolutions of the dial since the last time
 * read_dial() was called.  This is a destructive operation; it is not
 * possible to read the dial without resetting the counter.
 */
INLINE double DialNode::
read_dial(int index) {
  return _dial->read_dial(index);
}

/**
 * Returns true if the state of the indicated dial dial is known, or false if
 * we have never heard anything about this particular dial.
 */
INLINE bool DialNode::
is_dial_known(int index) const {
  return _dial->is_dial_known(index);
}
