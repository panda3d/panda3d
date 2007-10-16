// Filename: collisionParabola.I
// Created by:  drose (11Oct07)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: CollisionParabola::Default Constructor
//       Access: Published
//  Description: Creates an invalid parabola.
////////////////////////////////////////////////////////////////////
INLINE CollisionParabola::
CollisionParabola() :
  _t1(0.0f), _t2(0.0f)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionParabola::Constructor
//       Access: Published
//  Description: Creates a parabola with the endpoints between t1 and
//               t2 in the parametric space of the parabola.
////////////////////////////////////////////////////////////////////
INLINE CollisionParabola::
CollisionParabola(const Parabolaf &parabola, float t1, float t2) :
  _parabola(parabola),
  _t1(t1), _t2(t2)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionParabola::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE CollisionParabola::
CollisionParabola(const CollisionParabola &copy) :
  _parabola(copy._parabola),
  _t1(copy._t1), _t2(copy._t2)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionParabola::set_parabola
//       Access: Public
//  Description: Replaces the parabola specified by this solid.
////////////////////////////////////////////////////////////////////
INLINE void CollisionParabola::
set_parabola(const Parabolaf &parabola) {
  _parabola = parabola;
  mark_internal_bounds_stale();
  mark_viz_stale();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionParabola::get_parabola
//       Access: Public
//  Description: Returns the parabola specified by this solid.
////////////////////////////////////////////////////////////////////
INLINE const Parabolaf &CollisionParabola::
get_parabola() const {
  return _parabola;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionParabola::set_t1
//       Access: Public
//  Description: Changes the starting point on the parabola.
////////////////////////////////////////////////////////////////////
INLINE void CollisionParabola::
set_t1(float t1) {
  _t1 = t1;
  mark_internal_bounds_stale();
  mark_viz_stale();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionParabola::get_t1
//       Access: Public
//  Description: Returns the starting point on the parabola.
////////////////////////////////////////////////////////////////////
INLINE float CollisionParabola::
get_t1() const {
  return _t1;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionParabola::set_t2
//       Access: Public
//  Description: Changes the ending point on the parabola.
////////////////////////////////////////////////////////////////////
INLINE void CollisionParabola::
set_t2(float t2) {
  _t2 = t2;
  mark_internal_bounds_stale();
  mark_viz_stale();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionParabola::get_t2
//       Access: Public
//  Description: Returns the ending point on the parabola.
////////////////////////////////////////////////////////////////////
INLINE float CollisionParabola::
get_t2() const {
  return _t2;
}
