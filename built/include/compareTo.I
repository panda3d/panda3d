/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file compareTo.I
 * @author drose
 * @date 2002-02-22
 */

/**
 * Returns true if a sorts before b, false otherwise.
 */
template<class ObjectType>
INLINE bool CompareTo<ObjectType>::
operator () (const ObjectType &a, const ObjectType &b) const {
  return (a.compare_to(b) < 0);
}
