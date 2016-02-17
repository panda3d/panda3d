/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file rigidBodyCombiner.I
 * @author drose
 * @date 2007-02-22
 */

/**
 *
 */
INLINE RigidBodyCombiner::VDUnifier::
VDUnifier(const VertexTransform *transform, const GeomVertexData *orig) :
  _transform(transform),
  _orig(orig)
{
}

/**
 *
 */
INLINE bool RigidBodyCombiner::VDUnifier::
operator < (const RigidBodyCombiner::VDUnifier &other) const {
  if (_transform != other._transform) {
    return _transform < other._transform;
  }
  return _orig < other._orig;
}
