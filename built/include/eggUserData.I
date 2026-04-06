/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggUserData.I
 * @author drose
 * @date 2003-06-03
 */

/**
 *
 */
INLINE EggUserData::
EggUserData() {
}


/**
 *
 */
INLINE EggUserData::
EggUserData(const EggUserData &copy) : TypedReferenceCount(copy) {
}


/**
 *
 */
INLINE EggUserData &EggUserData::
operator = (const EggUserData &copy) {
  TypedReferenceCount::operator = (copy);
  return *this;
}
