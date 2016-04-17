/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggGroupNode.I
 * @author drose
 * @date 1999-01-16
 */

/**
 * Returns true if the 2-d v1 is to the right of v2.
 */
INLINE bool EggGroupNode::
is_right(const LVector2d &v1, const LVector2d &v2) {
  return (v1[0] * v2[1] - v1[1] * v2[0]) > 0;
}

/**
 *
 */
bool EggGroupNode::TBNVertexValue::
operator < (const TBNVertexValue &other) const {
  if (_facing != other._facing) {
    return (int)_facing < (int)other._facing;
  }
  int compare = _pos.compare_to(other._pos);
  if (compare != 0) {
    return compare < 0;
  }
  compare = _normal.compare_to(other._normal);
  if (compare != 0) {
    return compare < 0;
  }
  compare = _uv.compare_to(other._uv);
  if (compare != 0) {
    return compare < 0;
  }
  return _uv_name < other._uv_name;
}
