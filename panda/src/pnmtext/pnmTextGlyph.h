// Filename: pnmTextGlyph.h
// Created by:  drose (03Apr02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef PNMTEXTGLYPH_H
#define PNMTEXTGLYPH_H

#include "pandabase.h"

#include "pnmImage.h"

////////////////////////////////////////////////////////////////////
//       Class : PNMTextGlyph
// Description : A single glyph in a PNMTextMaker.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA PNMTextGlyph {
public:
  PNMTextGlyph(double advance);
  ~PNMTextGlyph();

  void rescale(double scale_factor);
  INLINE int get_advance() const;

  void place(PNMImage &dest_image, int xp, int yp, 
             const Colorf &fg = Colorf(0.0f, 0.0f, 0.0f, 1.0f));

  INLINE int get_left() const;
  INLINE int get_right() const;
  INLINE int get_bottom() const;
  INLINE int get_top() const;

  INLINE int get_height() const;
  INLINE int get_width() const;
  INLINE double get_value(int x, int y) const;

private:
  PNMImage _image;
  int _top;
  int _left;
  double _advance;
  int _int_advance;

  friend class PNMTextMaker;
};

#include "pnmTextGlyph.I"

#endif
