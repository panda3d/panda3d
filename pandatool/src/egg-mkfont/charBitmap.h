// Filename: charBitmap.h
// Created by:  drose (16Feb01)
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


#ifndef CHARBITMAP_H
#define CHARBITMAP_H

#include <pandatoolbase.h>

#include "pvector.h"

////////////////////////////////////////////////////////////////////
//       Class : CharBitmap
// Description : This defines a single character read from the PK
//               file.  It stores the kerning information as well as
//               the character's decoded bitmap.
////////////////////////////////////////////////////////////////////
class CharBitmap {
public:
  typedef pvector<char> Row;
  typedef pvector<Row> Block;

  CharBitmap(int character, int width, int height,
             int hoff, int voff, double dx, double dy);

  bool paint(bool black, int num_pixels, int &repeat);

  INLINE int get_width() const;
  INLINE int get_height() const;

  int _character;
  int _hoff, _voff;
  double _dx, _dy;

  Block _block;
  unsigned int _x, _y;
};

#include "charBitmap.I"

#endif
