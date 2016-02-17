/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file scissorEffect.I
 * @author drose
 * @date 2008-07-30
 */

/**
 * Returns true if the ScissorEffect is a screen-based effect, meaning
 * get_frame() has a meaningful value, but get_a() and get_b() do not.
 */
INLINE bool ScissorEffect::
is_screen() const {
  return _screen;
}

/**
 * If is_screen() returns true, this method may be called to query the screen-
 * based scissor frame.  This is a series of left, right, bottom, top,
 * representing the scissor frame relative to the current DisplayRegion.  See
 * ScissorAttrib.
 */
INLINE const LVecBase4 &ScissorEffect::
get_frame() const {
  return _frame;
}

/**
 * Returns the number of node-based scissor points.  See get_point().
 */
INLINE int ScissorEffect::
get_num_points() const {
  return (int)_points.size();
}

/**
 * If is_screen() returns false, then get_num_points() and get_point() may be
 * called to query the node-based scissor frame.  These return n points (at
 * least two), which are understood to be in the space of this node, and which
 * define any opposite corners of the scissor frame.
 */
INLINE const LPoint3 &ScissorEffect::
get_point(int n) const {
  nassertr(n >= 0 && n < (int)_points.size(), LPoint3::zero());
  return _points[n]._p;
}

/**
 * Returns the node to which the nth point is relative, or empty NodePath to
 * indicate the current node.
 */
INLINE NodePath ScissorEffect::
get_node(int n) const {
  nassertr(n >= 0 && n < (int)_points.size(), NodePath());
  return _points[n]._node;
}

/**
 * Returns true if this ScissorEffect actually enables scissoring, or false if
 * it culls only.
 */
INLINE bool ScissorEffect::
get_clip() const {
  return _clip;
}
