// Filename: charBitmap.cxx
// Created by:  drose (16Feb01)
// 
////////////////////////////////////////////////////////////////////

#include "charBitmap.h"

#include <notify.h>


////////////////////////////////////////////////////////////////////
//     Function: CharBitmap::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
CharBitmap::
CharBitmap(int character, int width, int height, 
           int hoff, int voff, double dx, double dy) {
  _character = character;
  _hoff = hoff;
  _voff = voff;
  _dx = dx;
  _dy = dy;

  for (int y = 0; y < height; y++) {
    _block.push_back(Row(width));
  }

  _x = 0;
  _y = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: CharBitmap::paint
//       Access: Public
//  Description: Paints a string of same-color pixels into the bitmap.
//               This is called repeatedly by the rle decoder.
//               Returns true when the last pixel has been painted,
//               false if there is more to go.
////////////////////////////////////////////////////////////////////
bool CharBitmap::
paint(bool black, int num_pixels, int &repeat) {
  if (_y < _block.size()) {
    while (num_pixels > 0 && _y < _block.size()) {
      nassertr(_x < _block[_y].size(), true);
      _block[_y][_x] = black;
      _x++;
      if (_x >= _block[_y].size()) {
        // End of a row.
        _x = 0;
        _y++;
        while (repeat > 0 && _y < _block.size()) {
          _block[_y] = _block[_y-1];
          _y++;
          repeat--;
        }
      }
      num_pixels--;
    }
  }

  return (_y < _block.size());
}
