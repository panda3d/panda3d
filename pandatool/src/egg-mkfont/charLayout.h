// Filename: charLayout.h
// Created by:  drose (16Feb01)
// 
////////////////////////////////////////////////////////////////////

#ifndef CHARLAYOUT_H
#define CHARLAYOUT_H

#include <pandatoolbase.h>

#include "charPlacement.h"

#include <vector>

class CharPlacement;

////////////////////////////////////////////////////////////////////
//       Class : CharLayout
// Description : This represents the arrangement of all characters on
//               a working bitmap of a given size.  Either all
//               characters fit or they don't.
////////////////////////////////////////////////////////////////////
class CharLayout {
public:
  void reset(int working_xsize, int working_ysize, 
             int working_buffer_pixels);

  bool place_character(const CharBitmap *bm);

  typedef vector<CharPlacement> Placements;
  Placements _placements;

  int _working_xsize, _working_ysize;
  int _working_buffer_pixels;
  int _cx, _cy, _nexty;

private:
  bool find_hole(int &x, int &y, int x_size, int y_size) const;
  const CharPlacement *find_overlap(int x, int y, int x_size, int y_size) const;
};

#endif
