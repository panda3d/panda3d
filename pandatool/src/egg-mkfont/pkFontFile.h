// Filename: pkFontFile.h
// Created by:  drose (18Feb01)
// 
////////////////////////////////////////////////////////////////////

#ifndef PKFONTFILE_H
#define PKFONTFILE_H

#include <pandatoolbase.h>

#include "fontFile.h"

////////////////////////////////////////////////////////////////////
// 	 Class : PkFontFile
// Description : A specialization on FontFile for reading TeX-style
//               .pk fonts.
////////////////////////////////////////////////////////////////////
class PkFontFile : public FontFile {
public:
  PkFontFile();

  virtual bool read(const Filename &filename,
		    bool extract_all, const string &extract_only);

private:
  unsigned int fetch_nibble();
  unsigned int fetch_packed_int();
  unsigned int fetch_byte();
  unsigned int fetch_int(int n = 4);
  int fetch_signed_int(int n = 4);
  bool do_character(int flag_byte);
  void do_xxx(int num_bytes);
  void do_yyy();
  void do_post();
  void do_pre();
  bool read_pk();

  bool _post;
  bool _post_warning;
  int _p;
  bool _high;
  int _dyn_f;
  int _repeat_count;

  bool _extract_all;
  string _extract_only;

  vector<unsigned char> _pk;
};

#endif
