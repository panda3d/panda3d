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
class EggVertex;
class FontFile;

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
  EggVertex *make_vertex(const LPoint2d &xy);

  void copy_character(const CharPlacement &pl);

  bool consider_scale_factor(double scale_factor);
  void choose_scale_factor(double too_small, double too_large);
  bool choose_scale_factor();
  void choose_image_size();

  void unsmooth_rgb(PNMImage &image);
  string expand_hyphen(const string &str);

public:
  void run();

private:
  Filename _output_image_filename;
  Filename _input_font_filename;
  bool _got_output_size;
  Colord _fg, _bg;
  bool _use_alpha;
  int _output_xsize, _output_ysize, _output_zsize;
  double _buffer_pixels;
  double _poly_pixels;
  double _scale_factor;
  double _gaussian_radius;
  double _ds;
  double _scale;
  double _ppu;
  bool _get_all;
  string _only_chars;
  bool _small_caps;
  double _small_caps_scale;

  FontFile *_font;
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

