/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file movieTypeRegistry.I
 * @author rdb
 * @date 2013-08-24
 */

/**
 * Returns a pointer to the global MovieTypeRegistry instance.
 */
INLINE MovieTypeRegistry *MovieTypeRegistry::
get_global_ptr() {
  if (_global_ptr == nullptr) {
    _global_ptr = new MovieTypeRegistry;
  }

  return _global_ptr;
}
