/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggTable.I
 * @author drose
 * @date 1999-02-19
 */

/**
 *
 */
INLINE EggTable::
EggTable(const std::string &name) : EggGroupNode(name) {
  _type = TT_table;
}


/**
 *
 */
INLINE EggTable::
EggTable(const EggTable &copy) : EggGroupNode(copy), _type(copy._type) {
}


/**
 *
 */
INLINE EggTable &EggTable::
operator = (const EggTable &copy) {
  EggGroupNode::operator = (copy);
  _type = copy._type;

  return *this;
}


/**
 *
 */
INLINE void EggTable::
set_table_type(TableType type) {
  _type = type;
}


/**
 *
 */
INLINE EggTable::TableType EggTable::
get_table_type() const {
  return _type;
}
