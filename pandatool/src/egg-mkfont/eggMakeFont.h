// Filename: eggMakeFont.h
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

#ifndef EGGMAKEFONT_H
#define EGGMAKEFONT_H

#include "pandatoolbase.h"
#include "rangeDescription.h"

#include "eggWriter.h"
#include "eggTexture.h"
#include "pmap.h"

class PNMTextMaker;
class PNMTextGlyph;
class EggVertexPool;
class EggGroup;

////////////////////////////////////////////////////////////////////
//       Class : EggMakeFont
// Description : This program uses FreeType to generate an egg file
//               and a series of texture images from a font file
//               input, such as a TTF file.  The resulting egg file
//               can be loaded in Panda as a StaticTextFont object for
//               rendering text, even if FreeType is not compiled into
//               the executing Panda.
////////////////////////////////////////////////////////////////////
class EggMakeFont : public EggWriter {
public:
  EggMakeFont();

protected:
  virtual bool handle_args(Args &args);

public:
  void run();

private:
  static bool dispatch_range(const string &, const string &arg, void *var);
  EggVertex *make_vertex(const LPoint2d &xy);

  void add_character(int code);
  void make_geom(PNMTextGlyph *glyph, int character);
  EggTexture *get_tref(PNMTextGlyph *glyph, int character);
  EggTexture *make_tref(PNMTextGlyph *glyph, int character);

private:
  Colorf _fg, _bg, _interior;
  bool _got_interior;
  RangeDescription _range;
  double _pixels_per_unit;
  double _point_size;
  double _poly_margin;
  int _tex_margin;
  double _scale_factor;
  bool _no_native_aa;

  Filename _input_font_filename;
  int _face_index;
  string _output_image_pattern;

  PNMTextMaker *_text_maker;
  
  EggTexture::Format _format;
  int _num_channels;
  EggVertexPool *_vpool;
  EggGroup *_group;

  typedef pmap<PNMTextGlyph *, EggTexture *> TRefs;
  TRefs _trefs;
};


#endif

