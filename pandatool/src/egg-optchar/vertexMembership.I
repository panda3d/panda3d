/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vertexMembership.I
 * @author drose
 * @date 2003-07-21
 */

/**
 *
 */
INLINE VertexMembership::
VertexMembership(EggGroup *group, double membership) :
  _group(group),
  _membership(membership)
{
}

/**
 *
 */
INLINE VertexMembership::
VertexMembership(const VertexMembership &copy) :
  _group(copy._group),
  _membership(copy._membership)
{
}

/**
 *
 */
INLINE void VertexMembership::
operator = (const VertexMembership &copy) {
  _group = copy._group;
  _membership = copy._membership;
}

/**
 *
 */
INLINE bool VertexMembership::
operator < (const VertexMembership &other) const {
  if (_membership != other._membership) {
    return _membership < other._membership;
  }
  return _group < other._group;
}
