// Filename: charPlacement.I
// Created by:  drose (16Feb01)
// 
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: CharPlacement::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE CharPlacement::
CharPlacement(const CharBitmap *bm, int x, int y, 
              int width, int height) :
  _bm(bm), 
  _x(x),
  _y(y),
  _width(width),
  _height(height)
{
}
