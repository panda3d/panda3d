/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file asyncTaskCollection.I
 * @author drose
 * @date 2008-09-16
 */

/**
 *
 */
INLINE AsyncTaskCollection::
~AsyncTaskCollection() {
}

/**
 * Appends the other list onto the end of this one.
 */
INLINE void AsyncTaskCollection::
operator += (const AsyncTaskCollection &other) {
  add_tasks_from(other);
}

/**
 * Returns a AsyncTaskCollection representing the concatenation of the two
 * lists.
 */
INLINE AsyncTaskCollection AsyncTaskCollection::
operator + (const AsyncTaskCollection &other) const {
  AsyncTaskCollection a(*this);
  a += other;
  return a;
}
