// Filename: fontFile.h
// Created by:  drose (18Feb01)
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

#ifndef FONTFILE_H
#define FONTFILE_H

#include <pandatoolbase.h>

#include <filename.h>
#include <notify.h>

#include <math.h>

class CharBitmap;

////////////////////////////////////////////////////////////////////
//       Class : FontFile
// Description : This is an abstract base class that provides the
//               interface to reading and/or rasterizing a particular
//               font file.
////////////////////////////////////////////////////////////////////
class FontFile {
public:
  FontFile();
  virtual ~FontFile();

  virtual bool read(const Filename &filename,
                    bool extract_all, const string &extract_only)=0;

  INLINE double get_ds() const;
  INLINE double get_dpi() const;
  INLINE double get_vppp() const;
  INLINE double get_hppp() const;

  void sort_chars_by_height();
  INLINE int get_num_chars() const;
  INLINE CharBitmap *get_char(int n) const;

protected:
  double _ds;
  double _vppp;
  double _hppp;

  typedef pvector<CharBitmap *> Chars;
  Chars _chars;
};

#include "fontFile.I"

#endif
