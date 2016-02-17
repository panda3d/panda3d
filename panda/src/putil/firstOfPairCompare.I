/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file firstOfPairCompare.I
 * @author drose
 * @date 2000-04-04
 */

/**
 *
 */
template<class ObjectType, class Compare>
INLINE FirstOfPairCompare<ObjectType, Compare>::
FirstOfPairCompare(Compare compare) : _compare(compare) {
}

/**
 * Returns true if a sorts before b, false otherwise.
 */
template<class ObjectType, class Compare>
INLINE bool FirstOfPairCompare<ObjectType, Compare>::
operator () (const ObjectType &a, const ObjectType &b) const {
  return _compare(a.first, b.first);
}
