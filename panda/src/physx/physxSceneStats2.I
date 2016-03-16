/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxSceneStats2.I
 * @author enn0x
 * @date 2009-10-20
 */

/**
 *
 */
INLINE unsigned int PhysxSceneStats2::
get_num_stats() const {

  return _ptr->numStats;
}

/**
 *
 */
INLINE int PhysxSceneStats2::
get_cur_value(unsigned int index) const {

  nassertr(index < _ptr->numStats, -1);
  return _ptr->stats[index].curValue;
}

/**
 *
 */
INLINE int PhysxSceneStats2::
get_max_value(unsigned int index) const {

  nassertr(index < _ptr->numStats, -1);
  return _ptr->stats[index].maxValue;
}

/**
 *
 */
INLINE const char * PhysxSceneStats2::
get_name(unsigned int index) const {

  nassertr(index < _ptr->numStats, "");
  return _ptr->stats[index].name;
}

/**
 *
 */
INLINE unsigned int PhysxSceneStats2::
get_parent(unsigned int index) const {

  nassertr(index < _ptr->numStats, 0);
  return _ptr->stats[index].parent;
}
