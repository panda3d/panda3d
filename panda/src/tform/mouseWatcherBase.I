/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mouseWatcherBase.I
 * @author rdb
 * @date 2018-06-12
 */

/**
 * Sorts all the regions in this group into pointer order.
 */
INLINE void MouseWatcherBase::
sort_regions() {
  LightMutexHolder holder(_lock);
  if (!_sorted) {
    do_sort_regions();
  }
}

/**
 * Returns true if the group has already been sorted, false otherwise.
 */
INLINE bool MouseWatcherBase::
is_sorted() const {
  LightMutexHolder holder(_lock);
  return _sorted;
}
