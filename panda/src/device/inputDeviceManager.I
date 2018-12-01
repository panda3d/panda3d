/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file inputDeviceManager.I
 * @author rdb
 * @date 2015-12-09
 */

/**
 * Returns the singleton InputDeviceManager instance.
 */
INLINE InputDeviceManager *InputDeviceManager::
get_global_ptr() {
  if (_global_ptr != nullptr) {
    return _global_ptr;
  } else {
    make_global_ptr();
    return _global_ptr;
  }
}
