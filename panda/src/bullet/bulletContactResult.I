/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletContactResult.I
 * @author enn0x
 * @date 2010-03-08
 */

/**
 *
 */
INLINE PandaNode *BulletContact::
get_node0() const {

  return _node0;
}

/**
 *
 */
INLINE PandaNode *BulletContact::
get_node1() const {

  return _node1;
}

/**
 *
 */
INLINE BulletManifoldPoint &BulletContact::
get_manifold_point() {

  return _mp;
}

/**
 *
 */
INLINE int BulletContact::
get_idx0() const {

  return _idx0;
}

/**
 *
 */
INLINE int BulletContact::
get_idx1() const {

  return _idx1;
}

/**
 *
 */
INLINE int BulletContact::
get_part_id0() const {

  return _part_id0;
}

/**
 *
 */
INLINE int BulletContact::
get_part_id1() const {

  return _part_id1;
}

/**
 *
 */
INLINE int BulletContactResult::
get_num_contacts() const {

  return _contacts.size();
}

/**
 *
 */
INLINE BulletContact BulletContactResult::
get_contact(int idx) {

  nassertr(idx >= 0 && idx < (int)_contacts.size(), _empty);
  return _contacts[idx];
}
