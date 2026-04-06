/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file logicOpAttrib.I
 * @author rdb
 * @date 2016-03-24
 */

/**
 * Use LogicOpAttrib::make() to construct a new LogicOpAttrib object.
 */
INLINE LogicOpAttrib::
LogicOpAttrib(LogicOpAttrib::Operation op) :
  _op(op)
{
}

/**
 * Returns the logic operation specified by this attribute.
 */
INLINE LogicOpAttrib::Operation LogicOpAttrib::
get_operation() const {
  return _op;
}
