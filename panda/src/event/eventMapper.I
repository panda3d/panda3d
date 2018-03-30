/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eventMapper.I
 * @author Mitchell Stokes
 * @date 2018-3-30
 */

/**
 * Returns the singleton EventMapper instance.
 */
INLINE EventMapper *EventMapper::
get_global_ptr() {
  if (_global_ptr != nullptr) {
    return _global_ptr;
  } else {
    make_global_ptr();
    return _global_ptr;
  }
}
