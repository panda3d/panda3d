/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggMakeFont.h
 * @author drose
 * @date 2001-02-16
 */

#ifndef EGGMAKEFONT_H
#define EGGMAKEFONT_H

#include "pandatoolbase.h"
#include "rangeDescription.h"

#include "eggWriter.h"
#include "eggTexture.h"
#include "pmap.h"
#include "pvector.h"
#include "vector_string.h"

class PNMTextMaker;
class PNMTextGlyph;
class EggVertexPool;
class EggGroup;
class TextureImage;

/**
 * This program uses FreeType to generate an egg file and a series of texture
 * images from a font file input, such as a TTF file.  The resulting egg file
 * can be loaded in Panda as a StaticTextFont object for rendering text, even
 * if FreeType is not compiled into the executing Panda.
 */
class EggMakeFont : public EggWriter {
public:
  EggMakeFont();

protected:
  virtual bool handle_args(Args &args);

public:
  void run();

private:
  static bool dispatch_range(const std::string &, const std::string &arg, void *var);
  EggVertex *make_vertex(const LPoint2d &xy);

  void add_character(int code);
  void make_geom(PNMTextGlyph *glyph, int character);
  EggTexture *get_tref(PNMTextGlyph *glyph, int character);
  EggTexture *make_tref(PNMTextGlyph *glyph, int character);
  void add_extra_glyphs(const Filename &extra_filename);
  void r_add_extra_glyphs(EggGroupNode *egg_group);
  static bool is_numeric(const std::string &str);


private:
  LColor _fg, _bg, _interior;
  bool _got_interior;
  RangeDescription _range;
  vector_string _extra_filenames;
  double _pixels_per_unit;
  double _point_size;
  double _poly_margin;
  int _tex_margin;
  double _render_margin;
  bool _got_scale_factor;
  double _scale_factor;
  bool _no_reduce;
  bool _no_native_aa;
  bool _no_palettize;
  int _palette_size[2];
  bool _generate_distance_field;

  double _palettize_scale_factor;
  Filename _input_font_filename;
  int _face_index;
  std::string _output_glyph_pattern;
  std::string _output_palette_pattern;

  PNMTextMaker *_text_maker;

  EggTexture::Format _format;
  int _num_channels;
  EggVertexPool *_vpool;
  EggGroup *_group;

  typedef pmap<PNMTextGlyph *, EggTexture *> TRefs;
  TRefs _trefs;

  typedef pvector<TextureImage *> Textures;
  Textures _textures;
};


#endif
