/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggXfmSAnim.I
 * @author drose
 * @date 1999-02-19
 */

/**
 *
 */
INLINE EggXfmSAnim::
EggXfmSAnim(const std::string &name, CoordinateSystem cs) : EggGroupNode(name) {
  _has_fps = false;
  _coordsys = cs;
}


/**
 *
 */
INLINE EggXfmSAnim::
EggXfmSAnim(const EggXfmSAnim &copy)
  : EggGroupNode(copy),
    _fps(copy._fps),
    _has_fps(copy._has_fps),
    _order(copy._order),
    _coordsys(copy._coordsys) {
}


/**
 *
 */
INLINE EggXfmSAnim &EggXfmSAnim::
operator = (const EggXfmSAnim &copy) {
  EggGroupNode::operator = (copy);
  _fps = copy._fps;
  _has_fps = copy._has_fps;
  _order = copy._order;
  _coordsys = copy._coordsys;

  return *this;
}


/**
 *
 */
INLINE void EggXfmSAnim::
set_fps(double fps) {
  _fps = fps;
  _has_fps = true;
}


/**
 *
 */
INLINE void EggXfmSAnim::
clear_fps() {
  _has_fps = false;
}

/**
 *
 */
INLINE bool EggXfmSAnim::
has_fps() const {
  return _has_fps;
}

/**
 * This is only valid if has_fps() returns true.
 */
INLINE double EggXfmSAnim::
get_fps() const {
  nassertr(has_fps(), 0.0);
  return _fps;
}

/**
 *
 */
INLINE void EggXfmSAnim::
set_order(const std::string &order) {
  _order = order;
}

/**
 *
 */
INLINE void EggXfmSAnim::
clear_order() {
  _order = "";
}

/**
 *
 */
INLINE bool EggXfmSAnim::
has_order() const {
  return !_order.empty();
}

/**
 *
 */
INLINE const std::string &EggXfmSAnim::
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
INLINE const std::string &EggXfmSAnim::
get_standard_order() {
  return _standard_order;
}

/**
 * Returns the coordinate system this table believes it is defined within.
 * This should always match the coordinate system of the EggData structure
 * that owns it.  It is necessary to store it here because the meaning of the
 * h, p, and r columns depends on the coordinate system.
 */
INLINE CoordinateSystem EggXfmSAnim::
get_coordinate_system() const {
  return _coordsys;
}

/**
 * Removes all data from the table.  It does this by removing all of its
 * children.
 */
INLINE void EggXfmSAnim::
clear_data() {
  EggGroupNode::clear();
}
