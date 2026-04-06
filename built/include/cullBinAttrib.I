/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cullBinAttrib.I
 * @author drose
 * @date 2002-03-01
 */

/**
 * Use CullBinAttrib::make() to construct a new CullBinAttrib object.
 */
INLINE CullBinAttrib::
CullBinAttrib() {
  _draw_order = 0;
}

/**
 * Returns the name of the bin this attribute specifies.  If this is the empty
 * string, it refers to the default bin.
 */
INLINE const std::string &CullBinAttrib::
get_bin_name() const {
  return _bin_name;
}

/**
 * Returns the draw order this attribute specifies.  Some bins (in particular,
 * CullBinFixed bins) use this to further specify the order in which objects
 * should be rendered.
 */
INLINE int CullBinAttrib::
get_draw_order() const {
  return _draw_order;
}
