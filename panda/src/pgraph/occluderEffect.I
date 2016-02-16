/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file occluderEffect.I
 * @author drose
 * @date 2011-03-17
 */

/**
 * Use OccluderEffect::make() to construct a new OccluderEffect object.
 */
INLINE OccluderEffect::
OccluderEffect() {
}

/**
 * Use OccluderEffect::make() to construct a new OccluderEffect object.  The
 * copy constructor is only defined to facilitate methods like
 * add_on_occluder().
 */
INLINE OccluderEffect::
OccluderEffect(const OccluderEffect &copy) :
  _on_occluders(copy._on_occluders)
{
}

/**
 * Returns the number of occluders that are enabled by the effectute.
 */
INLINE int OccluderEffect::
get_num_on_occluders() const {
  return _on_occluders.size();
}

/**
 * Returns the nth occluder enabled by the effectute, sorted in render order.
 */
INLINE NodePath OccluderEffect::
get_on_occluder(int n) const {
  nassertr(n >= 0 && n < (int)_on_occluders.size(), NodePath::fail());
  return _on_occluders[n];
}

/**
 * Returns true if the indicated occluder is enabled by the effect, false
 * otherwise.
 */
INLINE bool OccluderEffect::
has_on_occluder(const NodePath &occluder) const {
  return _on_occluders.find(occluder) != _on_occluders.end();
}

/**
 * Returns true if this is an identity effect: it does not change the set of
 * occluders in use.
 */
INLINE bool OccluderEffect::
is_identity() const {
  return _on_occluders.empty();
}
