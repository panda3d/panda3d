// Filename: pnmTextMaker.h
// Created by:  drose (03Apr02)
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

#ifndef PNMTEXTMAKER_H
#define PNMTEXTMAKER_H

#include "pandabase.h"
#include "pmap.h"
#include "freetypeFont.h"
#include "textEncoder.h"

#include <ft2build.h>
#include FT_FREETYPE_H

class Filename;
class PNMImage;
class PNMTextGlyph;

////////////////////////////////////////////////////////////////////
//       Class : PNMTextMaker
// Description : This object uses the Freetype library to generate
//               text directly into an image.  It is different from
//               the TextNode/DynamicTextFont interface, which use the
//               Freetype library to generate text in the scene graph,
//               to be rendered onscreen via the Panda render
//               traversal.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA PNMTextMaker : public FreetypeFont {
public:
  PNMTextMaker(const Filename &font_filename, int face_index);
  PNMTextMaker(const char *font_data, int data_length, int face_index);
  ~PNMTextMaker();

  enum Alignment {
    A_left,
    A_right,
    A_center,
  };

  INLINE bool is_valid() const;

  INLINE void set_align(Alignment align_type);
  INLINE Alignment get_align() const;

  INLINE void set_interior_flag(bool interior_flag);
  INLINE bool get_interior_flag() const;

  INLINE void set_fg(const Colorf &fg);
  INLINE const Colorf &get_fg() const;

  INLINE void set_interior(const Colorf &interior);
  INLINE const Colorf &get_interior() const;

  INLINE void generate_into(const string &text,
                            PNMImage &dest_image, int x, int y);
  void generate_into(const wstring &text,
                     PNMImage &dest_image, int x, int y);

  PNMTextGlyph *get_glyph(int character);

private:
  void initialize();
  PNMTextGlyph *make_glyph(int glyph_index);
  void empty_cache();

  bool _is_valid;

  typedef pmap<int, PNMTextGlyph *> Glyphs;
  Glyphs _glyphs;

  Alignment _align;
  bool _interior_flag;
  Colorf _fg;
  Colorf _interior;
};

#include "pnmTextMaker.I"

#endif
