// Filename: textGlyph.h
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

#ifndef TEXTGLYPH_H
#define TEXTGLYPH_H

#include "pandatoolbase.h"

#include "pnmImage.h"

////////////////////////////////////////////////////////////////////
//       Class : TextGlyph
// Description : A single glyph in the font.
////////////////////////////////////////////////////////////////////
class TextGlyph {
public:
  TextGlyph(double advance);
  ~TextGlyph();

  void rescale(double scale_factor);
  int get_advance() const;

  void place(PNMImage &dest_image, int xp, int yp);

private:
  PNMImage _image;
  int _top;
  int _left;
  double _advance;
  int _int_advance;

  friend class TextMaker;
};

#endif
