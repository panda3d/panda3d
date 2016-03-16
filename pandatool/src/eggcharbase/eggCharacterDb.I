/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggCharacterDb.I
 * @author drose
 * @date 2006-10-05
 */

/**
 *
 */
INLINE EggCharacterDb::Key::
Key(const EggJointPointer *joint, TableType table_type, int frame) :
  _joint(joint),
  _table_type(table_type),
  _frame(frame)
{
}

/**
 * Provides an arbitrary unique ordering for all keys.
 */
INLINE bool EggCharacterDb::Key::
operator < (const EggCharacterDb::Key &other) const {
  if (_joint != other._joint) {
    return _joint < other._joint;
  }
  if (_table_type != other._table_type) {
    return _table_type < other._table_type;
  }
  return _frame < other._frame;
}
