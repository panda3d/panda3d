// Filename: eggMakeFont.h
// Created by:  drose (16Feb01)
// 
////////////////////////////////////////////////////////////////////

#ifndef EGGMAKEFONT_H
#define EGGMAKEFONT_H

#include <pandatoolbase.h>

#include "charLayout.h"

#include <eggWriter.h>
#include <luse.h>
#include <pnmImage.h>

class CharPlacement;
class CharBitmap;
class EggGroup;
class EggTexture;
class EggVertexPool;

////////////////////////////////////////////////////////////////////
// 	 Class : EggMakeFont
// Description : This program reads a rasterized font stored in a
//               Metafont/TeX pk file format, and generates an egg
//               file and texture map that can be used with TextNode
//               to render text using the font.
////////////////////////////////////////////////////////////////////
class EggMakeFont : public EggWriter {
public:
  EggMakeFont();

protected:
  virtual bool handle_args(Args &args);
  static bool dispatch_dimensions(const string &opt, const string &arg, void *data);
  bool ns_dispatch_dimensions(const string &opt, const string &arg);

private:
  TexCoordd get_uv(double x, double y);
  LPoint2d get_xy(double x, double y);

  void copy_character(const CharPlacement &pl);

  bool consider_scale_factor(double scale_factor);
  void choose_scale_factor(double too_small, double too_large);
  bool choose_scale_factor();
  void choose_image_size();

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
  void read_pk();

  string expand_hyphen(const string &str);

public:
  void run();

private:
  Filename _output_image_filename;
  Filename _input_pk_filename;
  bool _got_output_size;
  int _output_xsize, _output_ysize, _output_zsize;
  double _buffer_pixels;
  double _poly_pixels;
  double _scale_factor;
  double _gaussian_radius;
  double _ppu;
  bool _get_all;
  string _only_chars;

  double _ds;
  double _vppp;
  double _hppp;

  bool _post;
  bool _post_warning;
  int _p;
  bool _high;
  int _dyn_f;
  int _repeat_count;
  vector<unsigned char> _pk;
  typedef vector<CharBitmap *> Chars;
  Chars _chars;
  typedef map<int, PT(EggGroup)> EggDefs;
  EggDefs _egg_defs;

  CharLayout _layout;
  int _working_xsize, _working_ysize;
  int _working_buffer_pixels;
  double _working_poly_pixels;

  PNMImage _output_image;

  EggVertexPool *_vpool;
  EggGroup *_group;
  EggTexture *_tref;
};


#endif

