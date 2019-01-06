/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file ppInstance.I
 * @author drose
 * @date 2009-06-19
 */

/**
 * Returns the NPP object corresponding to this particular instance.
 */
inline NPP PPInstance::
get_npp_instance() const {
  return _npp_instance;
}

/**
 * Returns the current window parameters.
 */
const NPWindow *PPInstance::
get_window() const {
  if (_got_window) {
    return &_window;
  }
  return nullptr;
}
