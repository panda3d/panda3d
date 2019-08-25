/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file stringStreamBuf.I
 * @author drose
 * @date 2007-07-03
 */

/**
 * Swaps the indicated buffer for the contents of the internal buffer.  Does
 * not affect the ppos or gpos, or the iostream buffer.
 */
INLINE void StringStreamBuf::
swap_data(vector_uchar &data) {
  _data.swap(data);
}

/**
 * Returns a reference to the contents of the internal buffer, without any of
 * the iostream buffer.
 */
INLINE const vector_uchar &StringStreamBuf::
get_data() const {
  return _data;
}
