// Filename: fltPackedColor.h
// Created by:  drose (25Aug00)
// 
////////////////////////////////////////////////////////////////////

#ifndef FLTPACKEDCOLOR_H
#define FLTPACKEDCOLOR_H

#include <pandatoolbase.h>

#include <luse.h>

class FltRecordReader;
class FltRecordWriter;

////////////////////////////////////////////////////////////////////
// 	 Class : FltPackedColor
// Description : A packed color record, A, B, G, R.  This appears, for
//               instance, within a face bead.
////////////////////////////////////////////////////////////////////
class FltPackedColor {
public:
  INLINE FltPackedColor();

  INLINE Colorf get_color() const;
  INLINE RGBColorf get_rgb() const;

  void output(ostream &out) const;
  bool extract_record(FltRecordReader &reader);
  bool build_record(FltRecordWriter &writer) const;

public:
  int _a;
  int _b;
  int _g;
  int _r;
};

INLINE ostream &operator << (ostream &out, const FltPackedColor &color);

#include "fltPackedColor.I"

#endif


  
