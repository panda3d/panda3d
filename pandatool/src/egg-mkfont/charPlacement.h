// Filename: charPlacement.h
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

#ifndef CHARPLACEMENT_H
#define CHARPLACEMENT_H

#include <pandatoolbase.h>

#include "charBitmap.h"

////////////////////////////////////////////////////////////////////
//       Class : CharPlacement
// Description : This specifies where a particular character will be
//               placed on the working bitmap.  An array of these is
//               built up to lay out all the characters in the bitmap,
//               and then when the layout is suitable, the bitmap is
//               generated.
////////////////////////////////////////////////////////////////////
class CharPlacement {
public:
  INLINE CharPlacement(const CharBitmap *bm, int x, int y,
                       int width, int height);

  bool intersects(int x, int y, int x_size, int y_size) const;

  const CharBitmap *_bm;
  int _x, _y;
  int _width, _height;
};

#include "charPlacement.I"

#endif
