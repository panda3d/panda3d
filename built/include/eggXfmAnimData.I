/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggXfmAnimData.I
 * @author drose
 * @date 1999-02-19
 */

/**
 *
 */
INLINE EggXfmAnimData::
EggXfmAnimData(const std::string &name, CoordinateSystem cs) : EggAnimData(name) {
  _coordsys = cs;
}


/**
 *
 */
INLINE EggXfmAnimData::
EggXfmAnimData(const EggXfmAnimData &copy)
  : EggAnimData(copy),
    _order(copy._order),
    _contents(copy._contents),
    _coordsys(copy._coordsys) {
}


/**
 *
 */
INLINE EggXfmAnimData &EggXfmAnimData::
operator = (const EggXfmAnimData &copy) {
  EggAnimData::operator = (copy);
  _order = copy._order;
  _contents = copy._contents;
  _coordsys = copy._coordsys;

  return *this;
}


/**
 *
 */
INLINE void EggXfmAnimData::
set_order(const std::string &order) {
  _order = order;
}

/**
 *
 */
INLINE void EggXfmAnimData::
clear_order() {
  _order = "";
}

/**
 *
 */
INLINE bool EggXfmAnimData::
has_order() const {
  return !_order.empty();
}

/**
 *
 */
INLINE const std::string &EggXfmAnimData::
get_order() const {
  if (has_order()) {
    return _order;
  } else {
    return get_standard_order();
  }
}

/**
 * Returns the standard order of matrix component composition.  This is what
 * the order string must be set to in order to use set_value() or add_data()
 * successfully.
 */
INLINE const std::string &EggXfmAnimData::
get_standard_order() {
  return EggXfmSAnim::get_standard_order();
}


/**
 *
 */
INLINE void EggXfmAnimData::
set_contents(const std::string &contents) {
  _contents = contents;
}

/**
 *
 */
INLINE void EggXfmAnimData::
clear_contents() {
  _contents = "";
}

/**
 *
 */
INLINE bool EggXfmAnimData::
has_contents() const {
  return !_contents.empty();
}

/**
 *
 */
INLINE const std::string &EggXfmAnimData::
get_contents() const {
  return _contents;
}

/**
 * Returns the coordinate system this table believes it is defined within.
 * This should always match the coordinate system of the EggData structure
 * that owns it.  It is necessary to store it here because the meaning of the
 * h, p, and r columns depends on the coordinate system.
 */
INLINE CoordinateSystem EggXfmAnimData::
get_coordinate_system() const {
  return _coordsys;
}


/**
 * Returns the number of rows in the table.
 */
INLINE int EggXfmAnimData::
get_num_rows() const {
  if (get_num_cols() == 0) {
    return 0;
  }
  return get_size() / get_num_cols();
}

/**
 * Returns the number of columns in the table.  This is set according to the
 * "contents" string, which defines the meaning of each column.
 */
INLINE int EggXfmAnimData::
get_num_cols() const {
  return _contents.length();
}


/**
 * Returns the value at the indicated row.  Row must be in the range 0 <= row
 * < get_num_rows(); col must be in the range 0 <= col < get_num_cols().
 */
INLINE double EggXfmAnimData::
get_value(int row, int col) const {
  nassertr(get_num_cols() != 0, 0.0);
  nassertr(row >= 0 && row < get_num_rows(), 0.0);
  nassertr(col >= 0 && col < get_num_cols(), 0.0);
  return _data[row * get_num_cols() + col];
}
