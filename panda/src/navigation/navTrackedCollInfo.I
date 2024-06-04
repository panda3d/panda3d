/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file navTrackedCollInfo.I
 * @author Maxwell175
 * @date 2022-07-23
 */

INLINE NavTrackedCollInfo::
NavTrackedCollInfo(NodePath _node, BitMask32 _mask) :
node(_node), mask(_mask)
{}

/**
 * Returns the NodePath that will be searched for CollisionNodes.
 */
INLINE NodePath NavTrackedCollInfo::
get_node() const {
  return node;
}

/**
 * Returns the bit mask that the CollisionNodes will be matched against.
 */
INLINE BitMask32 NavTrackedCollInfo::
get_mask() const {
  return mask;
}

INLINE bool NavTrackedCollInfo::
operator==(const NavTrackedCollInfo &other) const {
  return std::tie(node, mask) ==
    std::tie(other.node, other.mask);
}

INLINE bool NavTrackedCollInfo::
operator>(const NavTrackedCollInfo &other) const {
  return std::tie(node, mask) >
         std::tie(other.node, other.mask);
}

INLINE bool NavTrackedCollInfo::
operator<(const NavTrackedCollInfo &other) const {
  return std::tie(node, mask) <
         std::tie(other.node, other.mask);
}
