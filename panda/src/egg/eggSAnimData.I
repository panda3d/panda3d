/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggSAnimData.I
 * @author drose
 * @date 1999-02-19
 */

/**
 *
 */
INLINE EggSAnimData::
EggSAnimData(const std::string &name) : EggAnimData(name) {
}


/**
 *
 */
INLINE EggSAnimData::
EggSAnimData(const EggSAnimData &copy) : EggAnimData(copy) {
}


/**
 *
 */
INLINE EggSAnimData &EggSAnimData::
operator = (const EggSAnimData &copy) {
  EggAnimData::operator = (copy);

  return *this;
}


/**
 * Returns the number of rows in the table.  For an SAnim table, each row has
 * one column.
 */
INLINE int EggSAnimData::
get_num_rows() const {
  return get_size();
}


/**
 * Returns the value at the indicated row.  Row must be in the range 0 <= row
 * < get_num_rows().
 */
INLINE double EggSAnimData::
get_value(int row) const {
  nassertr(row >= 0 && row < get_num_rows(), 0.0);
  return _data[row];
}


/**
 * Changes the value at the indicated row.  Row must be in the range 0 <= row
 * < get_num_rows().
 */
INLINE void EggSAnimData::
set_value(int row, double value) {
  nassertv(row >= 0 && row < get_num_rows());
  _data[row] = value;
}
