// Filename: charPlacement.h
// Created by:  drose (16Feb01)
// 
////////////////////////////////////////////////////////////////////

#ifndef CHARPLACEMENT_H
#define CHARPLACEMENT_H

#include <pandatoolbase.h>

#include "charBitmap.h"

////////////////////////////////////////////////////////////////////
// 	 Class : CharPlacement
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
