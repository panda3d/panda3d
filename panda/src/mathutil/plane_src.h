// Filename: plane_src.h
// Created by:  mike (09Jan97)
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
//       Class : Plane
// Description : An abstract mathematical description of a plane.  A
//               plane is defined by the equation Ax + By + Cz + D = 0.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA FLOATNAME(Plane) : public FLOATNAME(LVecBase4) {
PUBLISHED:
  INLINE_MATHUTIL FLOATNAME(Plane)(void);
  INLINE_MATHUTIL FLOATNAME(Plane)(const FLOATNAME(LVecBase4) &copy);
  INLINE_MATHUTIL FLOATNAME(Plane)(const FLOATNAME(LPoint3) &a, 
                                   const FLOATNAME(LPoint3) &b,
                                   const FLOATNAME(LPoint3) &c);
  INLINE_MATHUTIL FLOATNAME(Plane)(const FLOATNAME(LVector3) &normal,
                                   const FLOATNAME(LPoint3) &point);
  INLINE_MATHUTIL FLOATNAME(Plane)(FLOATTYPE a, FLOATTYPE b,
                                   FLOATTYPE c, FLOATTYPE d);

  INLINE_MATHUTIL FLOATNAME(Plane) operator * (const FLOATNAME(LMatrix3) &mat) const;
  INLINE_MATHUTIL FLOATNAME(Plane) operator * (const FLOATNAME(LMatrix4) &mat) const;
  INLINE_MATHUTIL FLOATNAME(Plane) operator - () const;

  FLOATNAME(LMatrix4) get_reflection_mat(void) const;

  INLINE_MATHUTIL FLOATNAME(LVector3) get_normal() const;
  FLOATNAME(LPoint3) get_point() const;

  INLINE_MATHUTIL FLOATTYPE dist_to_plane(const FLOATNAME(LPoint3) &point) const;
  INLINE_MATHUTIL bool intersects_line(FLOATNAME(LPoint3) &intersection_point,
                              const FLOATNAME(LPoint3) &p1,
                              const FLOATNAME(LPoint3) &p2) const;
  INLINE_MATHUTIL bool intersects_line(FLOATTYPE &t,
                              const FLOATNAME(LPoint3) &from,
                              const FLOATNAME(LVector3) &delta) const;

  bool intersects_plane(FLOATNAME(LPoint3) &from,
                        FLOATNAME(LVector3) &delta,
                        const FLOATNAME(Plane) &other) const;

  void output(ostream &out) const;
  void write(ostream &out, int indent_level = 0) const;
};

INLINE_MATHUTIL ostream &
operator << (ostream &out, const FLOATNAME(Plane) &p);

#include "plane_src.I"
