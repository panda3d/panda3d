/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file collisionParabola.I
 * @author drose
 * @date 2007-10-11
 */

/**
 * Creates an invalid parabola.
 */
INLINE CollisionParabola::
CollisionParabola() :
  _t1(0.0f), _t2(0.0f)
{
}

/**
 * Creates a parabola with the endpoints between t1 and t2 in the parametric
 * space of the parabola.
 */
INLINE CollisionParabola::
CollisionParabola(const LParabola &parabola, PN_stdfloat t1, PN_stdfloat t2) :
  _parabola(parabola),
  _t1(t1), _t2(t2)
{
}

/**
 *
 */
INLINE CollisionParabola::
CollisionParabola(const CollisionParabola &copy) :
  _parabola(copy._parabola),
  _t1(copy._t1), _t2(copy._t2)
{
}

/**
 * Replaces the parabola specified by this solid.
 */
INLINE void CollisionParabola::
set_parabola(const LParabola &parabola) {
  _parabola = parabola;
  mark_internal_bounds_stale();
  mark_viz_stale();
}

/**
 * Returns the parabola specified by this solid.
 */
INLINE const LParabola &CollisionParabola::
get_parabola() const {
  return _parabola;
}

/**
 * Changes the starting point on the parabola.
 */
INLINE void CollisionParabola::
set_t1(PN_stdfloat t1) {
  _t1 = t1;
  mark_internal_bounds_stale();
  mark_viz_stale();
}

/**
 * Returns the starting point on the parabola.
 */
INLINE PN_stdfloat CollisionParabola::
get_t1() const {
  return _t1;
}

/**
 * Changes the ending point on the parabola.
 */
INLINE void CollisionParabola::
set_t2(PN_stdfloat t2) {
  _t2 = t2;
  mark_internal_bounds_stale();
  mark_viz_stale();
}

/**
 * Returns the ending point on the parabola.
 */
INLINE PN_stdfloat CollisionParabola::
get_t2() const {
  return _t2;
}
