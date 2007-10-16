// Filename: parabola_src.h
// Created by:  drose (10Oct07)
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
//       Class : Parabola
// Description : An abstract mathematical description of a parabola,
//               particularly useful for describing arcs of
//               projectiles.
//
//               The parabolic equation, given parametrically here, is
//               P = At^2 + Bt + C.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_MATHUTIL FLOATNAME(Parabola) {
PUBLISHED:
  INLINE_MATHUTIL FLOATNAME(Parabola)();
  INLINE_MATHUTIL FLOATNAME(Parabola)(const FLOATNAME(LVecBase3) &a, 
                                      const FLOATNAME(LVecBase3) &b,
                                      const FLOATNAME(LVecBase3) &c);
  INLINE_MATHUTIL FLOATNAME(Parabola)(const FLOATNAME(Parabola) &copy);
  INLINE_MATHUTIL void operator = (const FLOATNAME(Parabola) &copy);
  INLINE_MATHUTIL ~FLOATNAME(Parabola)();

  void xform(const FLOATNAME(LMatrix4) &mat);

  INLINE_MATHUTIL const FLOATNAME(LVecBase3) &get_a() const;
  INLINE_MATHUTIL const FLOATNAME(LVecBase3) &get_b() const;
  INLINE_MATHUTIL const FLOATNAME(LVecBase3) &get_c() const;

  INLINE_MATHUTIL FLOATNAME(LPoint3) calc_point(FLOATTYPE t) const;

  void output(ostream &out) const;
  void write(ostream &out, int indent_level = 0) const;

public:
  void write_datagram(Datagram &destination) const;
  void read_datagram(DatagramIterator &source);

private:
  FLOATNAME(LVecBase3) _a, _b, _c;
};

inline ostream &
operator << (ostream &out, const FLOATNAME(Parabola) &p) {
  p.output(out);
  return out;
}

#include "parabola_src.I"
