/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file parabola_src.h
 * @author drose
 * @date 2007-10-10
 */

/**
 * An abstract mathematical description of a parabola, particularly useful for
 * describing arcs of projectiles.
 *
 * The parabolic equation, given parametrically here, is P = At^2 + Bt + C.
 */
class EXPCL_PANDA_MATHUTIL FLOATNAME(LParabola) {
PUBLISHED:
  INLINE_MATHUTIL FLOATNAME(LParabola)();
  INLINE_MATHUTIL FLOATNAME(LParabola)(const FLOATNAME(LVecBase3) &a,
                                      const FLOATNAME(LVecBase3) &b,
                                      const FLOATNAME(LVecBase3) &c);
  INLINE_MATHUTIL FLOATNAME(LParabola)(const FLOATNAME(LParabola) &copy);
  INLINE_MATHUTIL void operator = (const FLOATNAME(LParabola) &copy);
  INLINE_MATHUTIL ~FLOATNAME(LParabola)();

  void xform(const FLOATNAME(LMatrix4) &mat);

  INLINE_MATHUTIL const FLOATNAME(LVecBase3) &get_a() const;
  INLINE_MATHUTIL const FLOATNAME(LVecBase3) &get_b() const;
  INLINE_MATHUTIL const FLOATNAME(LVecBase3) &get_c() const;

  INLINE_MATHUTIL FLOATNAME(LPoint3) calc_point(FLOATTYPE t) const;

  void output(std::ostream &out) const;
  void write(std::ostream &out, int indent_level = 0) const;

  void write_datagram_fixed(Datagram &destination) const;
  void read_datagram_fixed(DatagramIterator &source);
  void write_datagram(Datagram &destination) const;
  void read_datagram(DatagramIterator &source);

private:
  FLOATNAME(LVecBase3) _a, _b, _c;
};

inline std::ostream &
operator << (std::ostream &out, const FLOATNAME(LParabola) &p) {
  p.output(out);
  return out;
}

#include "parabola_src.I"
