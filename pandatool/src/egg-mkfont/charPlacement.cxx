// Filename: charPlacement.cxx
// Created by:  drose (16Feb01)
// 
////////////////////////////////////////////////////////////////////

#include "charPlacement.h"
#include "charBitmap.h"


////////////////////////////////////////////////////////////////////
//     Function: CharPlacement::intersects
//       Access: Public
//  Description: Returns true if the particular position this char
//               has been assigned to overlaps the rectangle whose
//               top left corner is at x, y and whose size is given by
//               x_size, y_size, or false otherwise.
////////////////////////////////////////////////////////////////////
bool CharPlacement::
intersects(int x, int y, int x_size, int y_size) const {
  int hright = x + x_size;
  int hbot = y + y_size;

  int mright = _x + _width;
  int mbot = _y + _height;
  
  return !(x >= mright || hright <= _x || 
	   y >= mbot || hbot <= _y);
}
