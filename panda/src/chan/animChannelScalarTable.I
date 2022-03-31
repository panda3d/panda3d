/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animChannelScalarTable.I
 * @author drose
 * @date 1999-02-22
 */

/**
 * Returns a pointer to the table's data, if it exists, or NULL if it does
 * not.
 */
INLINE CPTA_stdfloat AnimChannelScalarTable::
get_table() const {
  return _table;
}

/**
 * Returns true if the data table has been assigned.
 */
INLINE bool AnimChannelScalarTable::
has_table() const {
  return _table != nullptr;
}


/**
 * Empties the data table.
 */
INLINE void AnimChannelScalarTable::
clear_table() {
  _table = nullptr;
}
