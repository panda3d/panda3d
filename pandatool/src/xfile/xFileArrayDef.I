/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file xFileArrayDef.I
 * @author drose
 * @date 2004-10-03
 */

/**
 *
 */
INLINE XFileArrayDef::
XFileArrayDef(int fixed_size) :
  _fixed_size(fixed_size),
  _dynamic_size(nullptr)
{
}

/**
 *
 */
INLINE XFileArrayDef::
XFileArrayDef(XFileDataDef *dynamic_size) :
  _fixed_size(0),
  _dynamic_size(dynamic_size)
{
}

/**
 * Returns true if this array definition specifies a const-size array, false
 * if it is a dynamic-size array.
 */
INLINE bool XFileArrayDef::
is_fixed_size() const {
  return (_dynamic_size == nullptr);
}

/**
 * Returns the const size of the array, if is_fixed_size() returned true.
 */
INLINE int XFileArrayDef::
get_fixed_size() const {
  nassertr(is_fixed_size(), 0);
  return _fixed_size;
}

/**
 * Returns the data element that names the dynamic size of the array, if
 * is_fixed_size() returned false.
 */
INLINE XFileDataDef *XFileArrayDef::
get_dynamic_size() const {
  nassertr(!is_fixed_size(), nullptr);
  return _dynamic_size;
}
