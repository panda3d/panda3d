// Filename: fltPackedColor.h
// Created by:  drose (25Aug00)
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

#ifndef FLTPACKEDCOLOR_H
#define FLTPACKEDCOLOR_H

#include "pandatoolbase.h"

#include "luse.h"
#include <math.h>

class FltRecordReader;
class FltRecordWriter;

////////////////////////////////////////////////////////////////////
//       Class : FltPackedColor
// Description : A packed color record, A, B, G, R.  This appears, for
//               instance, within a face bead.
////////////////////////////////////////////////////////////////////
class FltPackedColor {
public:
  INLINE FltPackedColor();

  INLINE Colorf get_color() const;
  INLINE RGBColorf get_rgb() const;
  INLINE void set_color(const Colorf &color);
  INLINE void set_rgb(const RGBColorf &rgb);

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



