// Filename: charBitmap.h
// Created by:  drose (16Feb01)
// 
////////////////////////////////////////////////////////////////////


#ifndef CHARBITMAP_H
#define CHARBITMAP_H

#include <pandatoolbase.h>

#include <vector>

////////////////////////////////////////////////////////////////////
// 	 Class : CharBitmap
// Description : This defines a single character read from the PK
//               file.  It stores the kerning information as well as
//               the character's decoded bitmap.
////////////////////////////////////////////////////////////////////
class CharBitmap {
public:
  typedef vector<char> Row;
  typedef vector<Row> Block;

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

// An STL function object to sort the characters in order from tallest
// to shortest.  This provides a more optimal packing into the
// resulting image.
class SortCharBitmap {
public:
  INLINE bool operator() (const CharBitmap *c1, const CharBitmap *c2) const;
};

#include "charBitmap.I"

#endif
