/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pnmTextGlyph.h
 * @author drose
 * @date 2002-04-03
 */

#ifndef PNMTEXTGLYPH_H
#define PNMTEXTGLYPH_H

#include "pandabase.h"

#include "pnmImage.h"
#include "vector_int.h"

/**
 * A single glyph in a PNMTextMaker.
 */
class EXPCL_PANDA_PNMTEXT PNMTextGlyph {
PUBLISHED:
  explicit PNMTextGlyph(double advance);
  ~PNMTextGlyph();

  INLINE int get_advance() const;

  void place(PNMImage &dest_image, int xp, int yp,
             const LColor &fg);
  void place(PNMImage &dest_image, int xp, int yp,
             const LColor &fg, const LColor &interior);

  INLINE int get_left() const;
  INLINE int get_right() const;
  INLINE int get_bottom() const;
  INLINE int get_top() const;

  INLINE int get_height() const;
  INLINE int get_width() const;
  INLINE double get_value(int x, int y) const;
  INLINE bool get_interior_flag(int x, int y) const;

private:
  void determine_interior();
  void scan_interior(int x, int y, xelval new_code, bool neighbor_dark,
                     int recurse_level);
  void rescale(double scale_factor);

  PNMImage _image;
  int _top;
  int _left;
  double _advance;
  int _int_advance;
  vector_int _scan_interior_points;

  friend class PNMTextMaker;
};

#include "pnmTextGlyph.I"

#endif
