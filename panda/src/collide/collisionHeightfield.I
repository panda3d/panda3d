/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file collisionHeightfield.I
 * @author hecris
 * @date 2019-07-01
 */

/**
 *
 */
INLINE CollisionHeightfield::
CollisionHeightfield() {
}

/**
 *
 */
INLINE CollisionHeightfield::
CollisionHeightfield(const CollisionHeightfield &copy) {
}

/**
 *
 */
INLINE CollisionHeightfield::
~CollisionHeightfield() {
  delete[] _nodes;
 }

/**
 *
 */
INLINE void CollisionHeightfield::
flush_level() {
  _volume_pcollector.flush_level();
  _test_pcollector.flush_level();
}

/**
 *
 */
INLINE PNMImage CollisionHeightfield::
get_heightfield() {
  return _heightfield;
}

/**
 *
 */
INLINE void CollisionHeightfield::
set_heightfield(PNMImage heightfield) {
  int r = _heightfield.get_x_size();
  int c = _heightfield.get_y_size();
  _heightfield = heightfield;

  if (_heightfield.get_x_size() == r &&
      _heightfield.get_y_size() == c) {
    fill_quadtree_heights();
  } else {
    fill_quadtree_areas();
    fill_quadtree_heights();
  }
}

/**
 *
 */
INLINE PN_stdfloat CollisionHeightfield::
get_max_height() {
  return _max_height;
}

/**
 *
 */
INLINE void CollisionHeightfield::
set_max_height(PN_stdfloat max_height) {
  _max_height = max_height;
  fill_quadtree_heights();
}

/**
 *
 */
INLINE int CollisionHeightfield::
get_num_subdivisions() {
  return _num_subdivisions;
}

/**
 *
 */
INLINE PN_stdfloat CollisionHeightfield::
get_height(int x, int y) const {
  return _heightfield.get_gray(x, y) * _max_height;
}
