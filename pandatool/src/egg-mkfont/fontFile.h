// Filename: fontFile.h
// Created by:  drose (18Feb01)
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
// 	 Class : FontFile
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

  typedef vector<CharBitmap *> Chars;
  Chars _chars;
};

#include "fontFile.I"

#endif
