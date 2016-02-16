/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mouseInterfaceNode.I
 * @author drose
 * @date 2004-06-11
 */

/**
 * Returns true if the indicated button (which must have been specified in a
 * previous call to watch_button()) is known to be held down, false otherwise.
 */
INLINE bool MouseInterfaceNode::
is_down(ButtonHandle button) const {
  return _current_button_state.is_down(button);
}
