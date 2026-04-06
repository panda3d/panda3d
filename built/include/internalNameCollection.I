/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file internalNameCollection.I
 * @author drose
 * @date 2002-03-16
 */

/**
 *
 */
INLINE InternalNameCollection::
~InternalNameCollection() {
}

/**
 * Appends the other list onto the end of this one.
 */
INLINE void InternalNameCollection::
operator += (const InternalNameCollection &other) {
  add_names_from(other);
}

/**
 * Returns a InternalNameCollection representing the concatenation of the two
 * lists.
 */
INLINE InternalNameCollection InternalNameCollection::
operator + (const InternalNameCollection &other) const {
  InternalNameCollection a(*this);
  a += other;
  return a;
}
