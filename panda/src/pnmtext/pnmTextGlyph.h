// Filename: pnmTextGlyph.h
// Created by:  drose (03Apr02)
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

#ifndef PNMTEXTGLYPH_H
#define PNMTEXTGLYPH_H

#include "pandabase.h"

#include "pnmImage.h"
#include "vector_int.h"

////////////////////////////////////////////////////////////////////
//       Class : PNMTextGlyph
// Description : A single glyph in a PNMTextMaker.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA PNMTextGlyph {
public:
  PNMTextGlyph(double advance);
  ~PNMTextGlyph();

  INLINE int get_advance() const;

  void place(PNMImage &dest_image, int xp, int yp, 
             const Colorf &fg);
  void place(PNMImage &dest_image, int xp, int yp, 
             const Colorf &fg, const Colorf &interior);

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
