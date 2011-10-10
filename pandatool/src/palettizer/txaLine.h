// Filename: txaLine.h
// Created by:  drose (30Nov00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef TXALINE_H
#define TXALINE_H

#include "pandatoolbase.h"

#include "paletteGroups.h"

#include "globPattern.h"
#include "eggTexture.h"
#include "eggRenderMode.h"

#include "pvector.h"

class PNMFileType;
class EggFile;
class TextureImage;

////////////////////////////////////////////////////////////////////
//       Class : TxaLine
// Description : This is a single matching line in the .txa file.  It
//               consists of a list of names (texture names or egg
//               file names), followed by a colon and an optional size
//               and a set of keywords.
////////////////////////////////////////////////////////////////////
class TxaLine {
public:
  TxaLine();

  bool parse(const string &line);

  bool match_egg(EggFile *egg_file) const;
  bool match_texture(TextureImage *texture) const;

  void output(ostream &out) const;

private:
  typedef pvector<GlobPattern> Patterns;
  Patterns _texture_patterns;
  Patterns _egg_patterns;

  enum SizeType {
    ST_none,
    ST_scale,
    ST_explicit_2,
    ST_explicit_3
  };

  SizeType _size_type;
  PN_stdfloat _scale;
  int _x_size;
  int _y_size;
  int _num_channels;
  EggTexture::Format _format;
  bool _force_format;
  bool _generic_format;
  bool _keep_format;
  EggRenderMode::AlphaMode _alpha_mode;
  EggTexture::WrapMode _wrap_u, _wrap_v;
  EggTexture::QualityLevel _quality_level;

  int _aniso_degree;
  bool _got_margin;
  int _margin;
  bool _got_coverage_threshold;
  double _coverage_threshold;

  enum Keyword {
    KW_omit,
    KW_nearest,
    KW_linear,
    KW_mipmap,
    KW_cont,
    KW_anisotropic
  };

  typedef pvector<Keyword> Keywords;
  Keywords _keywords;

  PaletteGroups _palette_groups;

  PNMFileType *_color_type;
  PNMFileType *_alpha_type;
};

INLINE ostream &operator << (ostream &out, const TxaLine &line) {
  line.output(out);
  return out;
}

#endif

