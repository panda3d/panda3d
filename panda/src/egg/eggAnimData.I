/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggAnimData.I
 * @author drose
 * @date 1999-02-19
 */

#include "pnotify.h"

/**
 *
 */
INLINE EggAnimData::
EggAnimData(const std::string &name) : EggNode(name) {
  _has_fps = false;
}


/**
 *
 */
INLINE EggAnimData::
EggAnimData(const EggAnimData &copy) :
  EggNode(copy), _data(copy._data),
  _fps(copy._fps), _has_fps(copy._has_fps) {
}


/**
 *
 */
INLINE EggAnimData &EggAnimData::
operator = (const EggAnimData &copy) {
  EggNode::operator = (copy);
  _data = copy._data;
  _fps = copy._fps;
  _has_fps = copy._has_fps;

  return *this;
}


/**
 *
 */
INLINE void EggAnimData::
set_fps(double fps) {
  _fps = fps;
  _has_fps = true;
}


/**
 *
 */
INLINE void EggAnimData::
clear_fps() {
  _has_fps = false;
}

/**
 *
 */
INLINE bool EggAnimData::
has_fps() const {
  return _has_fps;
}

/**
 * This is only valid if has_fps() returns true.
 */
INLINE double EggAnimData::
get_fps() const {
  nassertr(has_fps(), 0.0);
  return _fps;
}


/**
 * Removes all data and empties the table.
 */
INLINE void EggAnimData::
clear_data() {
  _data.clear();
}

/**
 * Adds a single element to the table.
 */
INLINE void EggAnimData::
add_data(double value) {
  _data.push_back(value);
}



/**
 * Returns the number of elements in the table.
 */
INLINE int EggAnimData::
get_size() const {
  return _data.size();
}


/**
 * Returns the entire table of data.
 */
INLINE PTA_double EggAnimData::
get_data() const {
  return _data;
}


/**
 * Replaces the entire table of data.
 */
INLINE void EggAnimData::
set_data(const PTA_double &data) {
  _data = data;
}
