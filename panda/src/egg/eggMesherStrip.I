// Filename: eggMesherStrip.I
// Created by:  drose (13Mar05)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: EggMesherStrip::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE EggMesherStrip::
EggMesherStrip(const EggMesherStrip &copy) :
  _prims(copy._prims),
  _edges(copy._edges),
  _verts(copy._verts),
  _type(copy._type),
  _index(copy._index),
  _status(copy._status),
  _planar(copy._planar),
  _plane_normal(copy._plane_normal),
  _plane_offset(copy._plane_offset),
  _row_id(copy._row_id),
  _flat_shaded(copy._flat_shaded)
{
}

////////////////////////////////////////////////////////////////////
//     Function: EggMesherStrip::is_coplanar_with
//       Access: Public
//  Description: Returns true if the strip and the other strip are
//               coplanar, within the indicated threshold.  See
//               coplanarity().
////////////////////////////////////////////////////////////////////
INLINE bool EggMesherStrip::
is_coplanar_with(const EggMesherStrip &other, PN_stdfloat threshold) const {
  return (coplanarity(other) <= threshold);
}

////////////////////////////////////////////////////////////////////
//     Function: EggMesherStrip::coplanarity
//       Access: Public
//  Description: Returns the degree to which the two strips are
//               coplanar.  0.0 is exactly coplanar; numbers somewhat
//               larger than zero indicate less coplanar.  1.0 is
//               at right angles; 2.0 is exactly backfacing.  If
//               either strip is not itself planar, 3.0 is returned.
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat EggMesherStrip::
coplanarity(const EggMesherStrip &other) const {
  if (_planar && other._planar) {
    return 1.0 - dot(_plane_normal, other._plane_normal);
  }
  return 3.0;
}

////////////////////////////////////////////////////////////////////
//     Function: EggMesherStrip::type_category
//       Access: Public
//  Description: Returns an integer which gives a heuristic about the
//               similarity of different strip types.  In general,
//               closer numbers are more similar.
////////////////////////////////////////////////////////////////////
INLINE int EggMesherStrip::
type_category() const {
  switch (_type) {
  case PT_tri:
    return 1;

  case PT_tristrip:
    return 2;

  case PT_quad:
  case PT_quadstrip:
    return 5;

  default:
    return 10;
  }
}


////////////////////////////////////////////////////////////////////
//     Function: EggMesherStrip::rotate_forward
//       Access: Public
//  Description: Rotates a triangle or quad by bringing its first
//               vertex to the back.
////////////////////////////////////////////////////////////////////
INLINE void EggMesherStrip::
rotate_forward() {
  _verts.push_back(_verts.front());
  _verts.pop_front();
}

////////////////////////////////////////////////////////////////////
//     Function: EggMesherStrip::rotate_back
//       Access: Public
//  Description: Rotates a triangle or quad by bringing its last
//               vertex to the front.
////////////////////////////////////////////////////////////////////
INLINE void EggMesherStrip::
rotate_back() {
  _verts.push_front(_verts.back());
  _verts.pop_back();
}


////////////////////////////////////////////////////////////////////
//     Function: EggMesherStrip::get_head_edge
//       Access: Public
//  Description: Returns an EggMesherEdge which represents the leading
//               edge in the quadstrip or tristrip.  This
//               EggMesherEdge will not have pointer equality with any
//               shared EggMesherEdge.
////////////////////////////////////////////////////////////////////
INLINE EggMesherEdge EggMesherStrip::
get_head_edge() const {
  Verts::const_iterator vi = _verts.begin();
  nassertr(vi != _verts.end(), EggMesherEdge(0, 0));
  ++vi;
  return EggMesherEdge(_verts.front(), *vi);
}

////////////////////////////////////////////////////////////////////
//     Function: EggMesherStrip::get_tail_edge
//       Access: Public
//  Description: Returns an EggMesherEdge which represents the
//               trailing edge in the quadstrip or tristrip.  This
//               EggMesherEdge will not have pointer equality with any
//               shared EggMesherEdge.
////////////////////////////////////////////////////////////////////
INLINE EggMesherEdge EggMesherStrip::
get_tail_edge() const {
  Verts::const_reverse_iterator vi = _verts.rbegin();
  nassertr(vi != _verts.rend(), EggMesherEdge(0, 0));
  ++vi;
  return EggMesherEdge(*vi, _verts.back());
}

////////////////////////////////////////////////////////////////////
//     Function: EggMesherStrip::operator ==
//       Access: Public
//  Description: Defines equality for strips.  This actually tests
//               only pointer equality; it's used only when removing a
//               strip from the list.
////////////////////////////////////////////////////////////////////
INLINE bool EggMesherStrip::
operator == (const EggMesherStrip &other) const {
  return this == &other;
}

////////////////////////////////////////////////////////////////////
//     Function: EggMesherStrip::operator !=
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool EggMesherStrip::
operator != (const EggMesherStrip &other) const {
  return !operator == (other);
}
