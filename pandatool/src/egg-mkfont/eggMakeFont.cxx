// Filename: eggMakeFont.cxx
// Created by:  drose (16Feb01)
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

#include "eggMakeFont.h"
#include "charBitmap.h"
#include "charPlacement.h"
#include "pkFontFile.h"

#include "string_utils.h"
#include "eggGroup.h"
#include "eggTexture.h"
#include "eggVertexPool.h"
#include "eggPolygon.h"
#include "eggPoint.h"
#include "pointerTo.h"

#include <math.h>

////////////////////////////////////////////////////////////////////
//     Function: EggMakeFont::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
EggMakeFont::
EggMakeFont() : EggWriter(true, false) {
  set_program_description
    ("egg-mkfont reads a rasterized font stored in a "
     "Metafont/TeX pk file format, and generates an egg "
     "file and corresponding texture map that can be used with "
     "Panda's TextNode object to render text in the font.\n\n"

     "The input pk file can come from any of a number of sources.  "
     "It may be a Metafont font that ships with TeX, or it may "
     "have been converted from a PostScript font, or you can use "
     "freetype (see www.freetype.org) to convert a TTF font to pk.\n\n");


  clear_runlines();
  add_runline("[opts] -o output.egg file.pk");
  add_runline("[opts] file.pk output.egg");

  add_option
    ("i", "filename", 0,
     "Name of the texture image to write.  The default if this is omitted "
     "is based on the name of the egg file.",
     &EggMakeFont::dispatch_filename, NULL, &_output_image_filename);

  add_option
    ("c", "num", 0,
     "Specifies the number of channels of the output image.  This should "
     "either 1, 2, 3, or 4.  If the number is 1 or 3 a grayscale image is "
     "generated, with the text in white on black.  If the number is 2 or 4 "
     "a completely white image is generated, with the text in the alpha "
     "channel.  This parameter may also be specified as the third number "
     "on -d, below.  The default is 1.",
     &EggMakeFont::dispatch_int, NULL, &_output_zsize);

  add_option
    ("fg", "r,g,b[,a]", 0,
     "Specifies the foreground color of the generated texture map.  The "
     "default is white: 1,1,1,1, which leads to the most flexibility "
     "as the color can be modulated at runtime to any suitable color.",
     &EggMakeFont::dispatch_color, NULL, &_fg[0]);

  add_option
    ("bg", "r,g,b[,a]", 0,
     "Specifies the background color of the generated texture map.  The "
     "default is transparent black: 0,0,0,0, which allows the text to be "
     "visible against any color background by placing a polygon of a "
     "suitable color behind it.",
     &EggMakeFont::dispatch_color, NULL, &_bg[0]);

  add_option
    ("d", "x,y[,c]", 0,
     "Dimensions in pixels of the texture image, with an optional number of "
     "channels.  Normally, you should not specify this parameter, as "
     "egg-mkfont will choose an image size that yields a scale factor "
     "between 2.5 and 4, which leads to good antialiased letters.  If you "
     "want a larger or smaller image, you could force the image size "
     "with this parameter, but it would probably yield better results if "
     "you re-rasterized the font at a different DPI instead.",
     &EggMakeFont::dispatch_dimensions, &_got_output_size);

  add_option
    ("sf", "factor", 0,

     "The scale factor of the generated image.  This is the factor by which "
     "the font image is generated oversized, then reduced to its final size, "
     "to generate antialiased letters.  Values between 2.5 and 4 are generally "
     "best.  Normally, you should not specify this parameter, as egg-mkfont "
     "will choose a scale factor automatically to best fit the letters in the "
     "space available.  If you do specify a scale factor with -sf, you must "
     "also specify an image size with -d.",
     &EggMakeFont::dispatch_double, &_got_scale_factor, &_scale_factor);

  add_option
    ("nr", "", 0,
     "No reduce.  After the oversized image is generated, rather than reducing "
     "it to its final size, just leave it as it is, and assume the user will "
     "reduce it later.  This may be desireable if you intend to adjust the "
     "letters by hand in some way after the image is generated.",
     &EggMakeFont::dispatch_none, &_no_reduce);

  add_option
    ("g", "radius", 0,
     "The radius of the Gaussian filter used to antialias the letters. [1.2]",
     &EggMakeFont::dispatch_double, NULL, &_gaussian_radius);

  add_option
    ("b", "n", 0,
     "The number of buffer pixels between two adjacent characters in "
     "the palette image. [4.0]",
     &EggMakeFont::dispatch_double, NULL, &_buffer_pixels);

  add_option
    ("B", "n", 0,
     "The number of extra pixels around a single character in the "
     "generated polygon. [1.0]",
     &EggMakeFont::dispatch_double, NULL, &_poly_pixels);

  add_option
    ("ds", "size", 0,
     "Specify the design size of the resulting font.  The design size of "
     "a font is the height of a typical capital letter; it's the approximate "
     "height of a line of text.  This sets the size of the polygons "
     "accordingly.  [1.0]",
     &EggMakeFont::dispatch_double, NULL, &_ds);

  add_option
    ("scale", "size", 0,
     "Specify an additional scale to the font, without changing its design "
     "size.  This makes the letters larger (or smaller) without changing "
     "the spacing between lines.  Usually you should use -ds instead of "
     "-scale to change the size of the text.",
     &EggMakeFont::dispatch_double, NULL, &_scale);

  add_option
    ("all", "", 0,
     "Extract all the characters in the font.  Normally, only the "
     "ASCII characters in the range 33 .. 127 are extracted.",
     &EggMakeFont::dispatch_none, &_get_all);

  add_option
    ("only", "'chars'", 0,
     "Extract *only* the indicated characters from the font.  The parameter "
     "should be a quoted string of letters and symbols that are to be "
     "extracted.  If the hyphen appears, it indicates a range of characters, "
     "e.g. A-Z to extract all the capital letters.  If the hyphen appears "
     "as the first or last character it loses its special meaning.",
     &EggMakeFont::dispatch_string, NULL, &_only_chars);

  add_option
    ("sc", "", 0,
     "Small caps: generate lowercase letters as small capitals.  This "
     "allows the lowercase and capital letters to share the same space "
     "on the texture map.",
     &EggMakeFont::dispatch_none, &_small_caps);

  add_option
    ("scs", "", 0,
     "Small caps scale: the ratio of the size of a lowercase letter to "
     "its uppercase equivalent, when -sc is in effect.  [0.8]",
     &EggMakeFont::dispatch_double, NULL, &_small_caps_scale);

  _fg.set(1.0, 1.0, 1.0, 1.0);
  _bg.set(0.0, 0.0, 0.0, 0.0);
  _output_xsize = 256;
  _output_ysize = 256;
  _output_zsize = 1;
  _buffer_pixels = 4.0;
  _poly_pixels = 1.0;
  _scale_factor = 3.0;
  _gaussian_radius = 1.2;
  _ds = 1.0;
  _scale = 1.0;
  _small_caps_scale = 0.8;
}


////////////////////////////////////////////////////////////////////
//     Function: EggMakeFont::handle_args
//       Access: Protected, Virtual
//  Description: Does something with the additional arguments on the
//               command line (after all the -options have been
//               parsed).  Returns true if the arguments are good,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool EggMakeFont::
handle_args(ProgramBase::Args &args) {
  if (args.empty()) {
    nout << "Must specify name of pk file on command line.\n";
    return false;
  }

  _input_font_filename = args[0];
  args.pop_front();
  return EggWriter::handle_args(args);
}

////////////////////////////////////////////////////////////////////
//     Function: EggMakeFont::dispatch_dimensions
//       Access: Protected, Static
//  Description: Reads the dimensions of the output image and stores
//               them in _output_[xyz]size.
////////////////////////////////////////////////////////////////////
bool EggMakeFont::
dispatch_dimensions(ProgramBase *self, const string &opt, const string &arg, void *) {
  EggSingleBase *base = (EggSingleBase *)self;
  EggMakeFont *me = (EggMakeFont *)base->as_writer();
  return me->ns_dispatch_dimensions(opt, arg);
}

////////////////////////////////////////////////////////////////////
//     Function: EggMakeFont::ns_dispatch_dimensions
//       Access: Protected
//  Description: Reads the dimensions of the output image and stores
//               them in _output_[xyz]size.
////////////////////////////////////////////////////////////////////
bool EggMakeFont::
ns_dispatch_dimensions(const string &opt, const string &arg) {
  vector_string words;
  tokenize(arg, words, ",");

  bool okflag = false;
  if (words.size() == 2) {
    okflag =
      string_to_int(words[0], _output_xsize) &&
      string_to_int(words[1], _output_ysize);

  } else if (words.size() == 3) {
    okflag =
      string_to_int(words[0], _output_xsize) &&
      string_to_int(words[1], _output_ysize) &&
      string_to_int(words[2], _output_zsize);
  }

  if (!okflag) {
    nout << "-" << opt
         << " requires two or three integers separated by commas.\n";
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: EggMakeFont::get_uv
//       Access: Private
//  Description: Given the X, Y coordinates of a particular pixel on
//               the image, return the corresponding UV coordinates.
////////////////////////////////////////////////////////////////////
TexCoordd EggMakeFont::
get_uv(double x, double y) {
  return TexCoordd(x / (double)_working_xsize,
                   ((double)_working_ysize - y) / (double)_working_ysize);
}


////////////////////////////////////////////////////////////////////
//     Function: EggMakeFont::get_xy
//       Access: Private
//  Description: Given X, Y coordinates in pixels, scale to unit
//               coordinates for the character's geometry.
////////////////////////////////////////////////////////////////////
LPoint2d EggMakeFont::
get_xy(double x, double y) {
  return LPoint2d(x / (_font->get_hppp() * _ppu), -y / (_font->get_vppp() * _ppu));
}

////////////////////////////////////////////////////////////////////
//     Function: EggMakeFont::make_vertex
//       Access: Private
//  Description: Allocates and returns a new vertex from the vertex
//               pool representing the indicated 2-d coordinates.
////////////////////////////////////////////////////////////////////
EggVertex *EggMakeFont::
make_vertex(const LPoint2d &xy) {
  return
    _vpool->make_new_vertex(LPoint3d::origin(_coordinate_system) +
                            LVector3d::rfu(xy[0], 0.0, xy[1], _coordinate_system));
}

////////////////////////////////////////////////////////////////////
//     Function: EggMakeFont::copy_character
//       Access: Private
//  Description: Copy the indicated character image to its home on the
//               bitmap and generate egg structures for it.
////////////////////////////////////////////////////////////////////
void EggMakeFont::
copy_character(const CharPlacement &pl) {
  const CharBitmap *bm = pl._bm;
  int xp = pl._x;
  int yp = pl._y;

  int character = bm->_character;
  int hoff = bm->_hoff;
  int voff = bm->_voff;
  double dx = bm->_dx;
  double dy = bm->_dy;
  int width = bm->get_width();
  int height = bm->get_height();

  // Copy the character into the image.
  if (_output_image.has_alpha()) {
    for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++) {
        if (bm->_block[y][x]) {
          _output_image.set_xel(xp + x, yp + y, _fg[0], _fg[1], _fg[2]);
          _output_image.set_alpha(xp + x, yp + y, _fg[3]);
        }
      }
    }
  } else {
    for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++) {
        if (bm->_block[y][x]) {
          _output_image.set_xel(xp + x, yp + y, _fg[0], _fg[1], _fg[2]);
        }
      }
    }
  }

  // Create the polygon that will have the character mapped onto it.

  // b is the number of pixels bigger than the character in each
  // direction the polygon will be.  It needs to be larger than zero
  // just because when we filter the image down, we end up with some
  // antialiasing blur that extends beyond the original borders of the
  // character.  But it shouldn't be too large, because we don't want
  // the neighboring polygons of a word to overlap any more than they
  // need to.

  double b = _working_poly_pixels;

  TexCoordd uv_ul = get_uv(xp - b, yp - b);
  TexCoordd uv_lr = get_uv(xp + width + b, yp + height + b);
  LPoint2d xy_ul = get_xy(-hoff - b, -voff - b);
  LPoint2d xy_lr = get_xy(-hoff + width + b, -voff + height + b);
  LPoint2d dp = get_xy(dx, dy);

  EggVertex *v1 = make_vertex(LPoint2d(xy_ul[0], xy_lr[1]));
  EggVertex *v2 = make_vertex(LPoint2d(xy_lr[0], xy_lr[1]));
  EggVertex *v3 = make_vertex(LPoint2d(xy_lr[0], xy_ul[1]));
  EggVertex *v4 = make_vertex(LPoint2d(xy_ul[0], xy_ul[1]));

  v1->set_uv(TexCoordd(uv_ul[0], uv_lr[1]));
  v2->set_uv(TexCoordd(uv_lr[0], uv_lr[1]));
  v3->set_uv(TexCoordd(uv_lr[0], uv_ul[1]));
  v4->set_uv(TexCoordd(uv_ul[0], uv_ul[1]));

  // Create an egg group to hold the polygon.
  string group_name = format_string(character);
  PT(EggGroup) group = new EggGroup(group_name);
  _egg_defs[character] = group;

  EggPolygon *poly = new EggPolygon();
  group->add_child(poly);
  poly->set_texture(_tref);

  poly->add_vertex(v1);
  poly->add_vertex(v2);
  poly->add_vertex(v3);
  poly->add_vertex(v4);

  // Now create a single point where the origin of the next character
  // will be.

  EggVertex *v0 = make_vertex(dp);
  EggPoint *point = new EggPoint;
  group->add_child(point);
  point->add_vertex(v0);

  if (_small_caps && isupper(character)) {
    // Now create the polygon representing the lowercase letter, for
    // small caps mode.  This uses the same texture on a smaller
    // polygon.
    xy_ul *= _small_caps_scale;
    xy_lr *= _small_caps_scale;
    dp *= _small_caps_scale;
    character = tolower(character);

    EggVertex *v1 = make_vertex(LPoint2d(xy_ul[0], xy_lr[1]));
    EggVertex *v2 = make_vertex(LPoint2d(xy_lr[0], xy_lr[1]));
    EggVertex *v3 = make_vertex(LPoint2d(xy_lr[0], xy_ul[1]));
    EggVertex *v4 = make_vertex(LPoint2d(xy_ul[0], xy_ul[1]));

    v1->set_uv(TexCoordd(uv_ul[0], uv_lr[1]));
    v2->set_uv(TexCoordd(uv_lr[0], uv_lr[1]));
    v3->set_uv(TexCoordd(uv_lr[0], uv_ul[1]));
    v4->set_uv(TexCoordd(uv_ul[0], uv_ul[1]));

    string group_name = format_string(character);
    PT(EggGroup) group = new EggGroup(group_name);
    _egg_defs[character] = group;

    EggPolygon *poly = new EggPolygon();
    group->add_child(poly);
    poly->set_texture(_tref);

    poly->add_vertex(v1);
    poly->add_vertex(v2);
    poly->add_vertex(v3);
    poly->add_vertex(v4);

    EggVertex *v0 = make_vertex(dp);
    EggPoint *point = new EggPoint;
    group->add_child(point);
    point->add_vertex(v0);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggMakeFont::consider_scale_factor
//       Access: Private
//  Description: Attempts to place all of the characters on an image
//               of the indicated size (scaled up from the output
//               image size by scale_factor in each dimension).
//               Returns true if all the characters fit, or false if
//               the image was too small.  In either case, leaves
//               _scale_factor and _working_* set to reflect the
//               chosen scale factor.
////////////////////////////////////////////////////////////////////
bool EggMakeFont::
consider_scale_factor(double scale_factor) {
  _scale_factor = scale_factor;
  _working_xsize = (int)floor(_output_xsize * _scale_factor + 0.5);
  _working_ysize = (int)floor(_output_ysize * _scale_factor + 0.5);
  _working_buffer_pixels = (int)floor(_buffer_pixels * _scale_factor + 0.5);

  _layout.reset(_working_xsize, _working_ysize, _working_buffer_pixels);

  bool ok = true;
  int num_chars = _font->get_num_chars();
  for (int i = 0; i < num_chars; i++) {
    CharBitmap *bm = _font->get_char(i);
    if (!(_small_caps && islower(bm->_character))) {
      ok = _layout.place_character(bm);
      if (!ok) {
        // Out of room.
        return false;
      }
    }
  }

  // They all fit!
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: EggMakeFont::choose_scale_factor
//       Access: Private
//  Description: Binary search on scale factor, given a factor that is
//               known to be too small and one that is known to be too
//               large.
////////////////////////////////////////////////////////////////////
void EggMakeFont::
choose_scale_factor(double too_small, double too_large) {
  if (too_large - too_small < 0.000001) {
    // Close enough.
    consider_scale_factor(too_large);
    return;
  }

  double mid = (too_small + too_large) / 2.0;
  if (consider_scale_factor(mid)) {
    // This midpoint is too large.
    choose_scale_factor(too_small, mid);
  } else {
    // This midpoint is too small.
    choose_scale_factor(mid, too_large);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggMakeFont::choose_scale_factor
//       Access: Private
//  Description: Tries several scale_factors until an optimal one
//               (that is, the smallest one that all letters fit
//               within) is found.  Returns when all characters have
//               been successfully placed on the layout.
//
//               Returns true if successful, or false if it just could
//               not be done.
////////////////////////////////////////////////////////////////////
bool EggMakeFont::
choose_scale_factor() {
  // We need to determine a scale factor that will definitely be too
  // small, and one that will definitely be too large.
  double too_small, too_large;
  int sanity_count = 0;

  double guess = 1.0;
  if (consider_scale_factor(guess)) {
    // This guess is too large.
    do {
      too_large = guess;
      guess = guess / 2.0;
      if (sanity_count++ > 20) {
        return false;
      }
    } while (consider_scale_factor(guess));
    too_small = guess;

  } else {
    // This guess is too small.
    do {
      too_small = guess;
      guess = guess * 2.0;
      if (sanity_count++ > 20) {
        return false;
      }
    } while (!consider_scale_factor(guess));
    too_large = guess;
  }

  choose_scale_factor(too_small, too_large);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: EggMakeFont::choose_image_size
//       Access: Private
//  Description: Chooses a size for the output image that should yield
//               a scale_factor in the range (2.5 .. 4], which will give
//               pretty good antialiased letters.
////////////////////////////////////////////////////////////////////
void EggMakeFont::
choose_image_size() {
  // Start with an arbitrary guess.
  _output_xsize = 256;
  _output_ysize = 256;

  bool sane = choose_scale_factor();

  if (sane && _scale_factor <= 2.5) {
    // The scale factor is too small.  The letters may appear jaggy,
    // and we don't need so much image space.
    do {
      if (_output_ysize < _output_xsize) {
        _output_xsize /= 2;
      } else {
        _output_ysize /= 2;
      }
      sane = choose_scale_factor();
    } while (sane && _scale_factor <= 2.5);

    if (!sane) {
      // Oops, better backpedal.
      _output_xsize *= 2;
    }
    choose_scale_factor();

  } else if (_scale_factor > 4.0) {
    // The scale factor is too large.  The letters will be overly
    // reduced and may be blurry.  We need a larger image.
    do {
      if (_output_ysize < _output_xsize) {
        _output_ysize *= 2;
      } else {
        _output_xsize *= 2;
      }
      sane = choose_scale_factor();
    } while (!sane || _scale_factor > 4.0);
  }
}



////////////////////////////////////////////////////////////////////
//     Function: EggMakeFont::unsmooth_rgb
//       Access: Public
//  Description: Make the image jaggy in RGB (but leave it antialiased
//               in alpha).  This will result in correct antialiasing
//               when the text is loaded in the player.
////////////////////////////////////////////////////////////////////
void EggMakeFont::
unsmooth_rgb(PNMImage &image) {
  for (int y = 0; y < image.get_y_size(); y++) {
    for (int x = 0; x < image.get_x_size(); x++) {
      double alpha = image.get_alpha(x, y);
      if (alpha != 0.0) {
        image.set_xel(x, y, image.get_xel(x, y) / alpha);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggMakeFont::expand_hyphen
//       Access: Public
//  Description: If a hyphen appears in the string anywhere but in the
//               first and last position, replace it with a sequence
//               of characters.  For example, 0-9 becomes 0123456789.
//               Return the new string.
////////////////////////////////////////////////////////////////////
string EggMakeFont::
expand_hyphen(const string &str) {
  string result;
  size_t last = 0;

  size_t hyphen = str.find('-', last + 1);
  while (hyphen < str.length() - 1) {
    size_t ap = hyphen - 1;
    size_t zp = hyphen + 1;
    result += str.substr(last, ap - last);
    char a = str[ap];
    char z = str[zp];

    for (char i = a; i <= z; i++) {
      result += i;
    }

    last = zp + 1;
    hyphen = str.find('-', last + 1);
  }

  result += str.substr(last);
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: EggMakeFont::run
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void EggMakeFont::
run() {
  if (_output_image_filename.empty() && has_output_filename()) {
    _output_image_filename = get_output_filename();
    _output_image_filename.set_extension("rgb");
  }

  if (_output_image_filename.empty()) {
    nout << "No output image filename given.\n";
    exit(1);
  }

  if (!_only_chars.empty()) {
    _only_chars = expand_hyphen(_only_chars);
    nout << "Extracting only characters: " << _only_chars << "\n";
    if (_small_caps) {
      _only_chars = upcase(_only_chars);
    }
  }

  _font = new PkFontFile();
  if (!_font->read(_input_font_filename, _get_all, _only_chars)) {
    nout << "Unable to read " << _input_font_filename << ".\n";
    exit(1);
  }

  nout << "Placing " << _font->get_num_chars() << " letters.\n";

  // Now that we've collected all the characters, sort them in order
  // from tallest to shortest so we will hopefully get a more optimal
  // packing.
  _font->sort_chars_by_height();


  // Choose a suitable image size and/or scale factor.
  if (_got_scale_factor && _got_output_size) {
    // The user specified both; we accept the scale factor.
    if (!consider_scale_factor(_scale_factor)) {
      nout << "Ran out of room on font image; try increasing the image "
        "size or the scale factor.\n";
      exit(1);
    }

  } else if (_got_output_size) {
    // The user specified an output size, but not a scale factor.
    choose_scale_factor();

  } else if (_got_scale_factor) {
    // The user specified a scale factor, but not an output size.
    // This is really an error.
    nout << "It is meaningless to specify a scale factor (-sf) without "
      "also specifying an image size (-d).  Ignoring scale factor.\n";
    choose_image_size();

  } else {
    // The user did not specify anything.  This is really preferred.
    // We'll decide what's best.
    choose_image_size();
  }

  _working_poly_pixels = _poly_pixels * _scale_factor;
  _output_image.clear(_working_xsize, _working_ysize, _output_zsize);

  // If the user specified 1.0 for both foreground and background
  // alpha, we don't really want to use alpha.
  _use_alpha = (_output_zsize != 3) && (_fg[3] != 1.0 || _bg[3] != 1.0);
  if (_use_alpha && _output_zsize == 1) {
    // If we have only one channel and we're using alpha, then the
    // gray channel becomes the alpha channel.
    _fg[0] = _fg[3];
    _bg[0] = _bg[3];
  }

  _output_image.fill(_bg[0], _bg[1], _bg[2]);
  if (_output_image.has_alpha()) {
    _output_image.alpha_fill(_bg[3]);
  }

  _group = new EggGroup();
  _data.add_child(_group);
  _tref = new EggTexture("chars", _output_image_filename);
  _group->add_child(_tref);
  _vpool = new EggVertexPool("vpool");
  _group->add_child(_vpool);

  // Set the appropriate flags on the texture.
  EggTexture::Format format = EggTexture::F_unspecified;
  if (_use_alpha) {
    switch (_output_zsize) {
    case 1:
      format = EggTexture::F_alpha;
      break;
    case 2:
      format = EggTexture::F_luminance_alpha;
      break;
    case 4:
      format = EggTexture::F_rgba;
      break;
    }
  } else {
    switch (_output_zsize) {
    case 1:
    case 2:
      format = EggTexture::F_luminance;
      break;
    case 3:
    case 4:
      format = EggTexture::F_rgb;
      break;
    }
  }
  _tref->set_format(format);

  // Make the group a sequence, as a convenience.  If we view the
  // egg file directly we can see all the characters one at a time.
  _group->set_switch_flag(true);
  _group->set_switch_fps(2.0);

  // Compute the font points per polygon unit.
  _ppu = _font->get_ds() / (_ds * _scale);

  // Now we can copy all the characters onto the actual image.
  CharLayout::Placements::const_iterator pi;
  for (pi = _layout._placements.begin();
       pi != _layout._placements.end();
       ++pi) {
    copy_character(*pi);
  }

  // And now put all the Egg structures we created into the egg file,
  // in numeric order.
  EggDefs::const_iterator edi;
  for (edi = _egg_defs.begin(); edi != _egg_defs.end(); ++edi) {
    _group->add_child((*edi).second.p());
  }

  // Also create an egg group indicating the font's design size.
  EggGroup *ds_group = new EggGroup("ds");
  _group->add_child(ds_group);
  EggVertex *vtx = make_vertex(LPoint2d(0.0, _ds));
  EggPoint *point = new EggPoint;
  ds_group->add_child(point);
  point->add_vertex(vtx);

  if (_use_alpha && _output_zsize != 1) {
    if (_bg[3] == 0.0) {
      // If we have a transparent background, then everything in the
      // color channels is pointless--the color information is
      // completely replaced.  Might as well fill it white.
      _output_image.fill(_fg[0], _fg[1], _fg[2]);

    } else if (_bg[3] == 1.0) {
      // Similarly if we have a transparent foreground.
      _output_image.fill(_bg[0], _bg[1], _bg[2]);
    }
  }

  // All done!  Write everything out.
  nout << "Scale factor is " << _scale_factor << "\n";

  if (_no_reduce) {
    // Scaling of the final image forbidden by the user.
    nout << "Image destination size is " << _output_xsize
         << " by " << _output_ysize << " by " << _output_zsize
         << "; not reducing.\n";
    nout << "Generating " << _working_xsize << " by " << _working_ysize
         << " by " << _output_zsize << " image: "
         << _output_image_filename << "\n";

    _output_image.write(_output_image_filename);

  } else if (_output_xsize == _working_xsize &&
             _output_ysize == _working_ysize) {
    // Scaling unnecessary, because the scale factor is 1.0.
    nout << "Generating " << _output_xsize << " by " << _output_ysize
         << " by " << _output_zsize << " image: "
         << _output_image_filename << "\n";
    _output_image.write(_output_image_filename);

  } else {
    // The normal path: reduce the final image by the scale factor to
    // antialias the letters.
    PNMImage small_image(_output_xsize, _output_ysize, _output_zsize);
    small_image.gaussian_filter_from(_gaussian_radius, _output_image);

    // Fix antialiasing, if required.
    if (_use_alpha && _bg[3] != 0.0 && _bg[3] != 1.0) {
      // If we have some non-transparent background, we need to
      // compensate for the antialiasing.
      unsmooth_rgb(small_image);
    }


    nout << "Generating " << _output_xsize << " by " << _output_ysize
         << " by " << _output_zsize << " image: "
         << _output_image_filename << "\n";
    small_image.write(_output_image_filename);
  }

  write_egg_file();
}


int main(int argc, char *argv[]) {
  EggMakeFont prog;
  prog.parse_command_line(argc, argv);
  prog.run();
  return 0;
}
