/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pnmTextMaker.h
 * @author drose
 * @date 2002-04-03
 */

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

/**
 * This object uses the Freetype library to generate text directly into an
 * image.  It is different from the TextNode/DynamicTextFont interface, which
 * use the Freetype library to generate text in the scene graph, to be
 * rendered onscreen via the Panda render traversal.
 */
class EXPCL_PANDA_PNMTEXT PNMTextMaker : public FreetypeFont {
PUBLISHED:
  explicit PNMTextMaker(const Filename &font_filename, int face_index);
  explicit PNMTextMaker(const char *font_data, int data_length, int face_index);
  explicit PNMTextMaker(const FreetypeFont &copy);
  PNMTextMaker(const PNMTextMaker &copy);
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

  INLINE void set_fg(const LColor &fg);
  INLINE const LColor &get_fg() const;

  INLINE void set_interior(const LColor &interior);
  INLINE const LColor &get_interior() const;

  INLINE void set_distance_field_radius(int radius);
  INLINE int get_distance_field_radius() const;

  INLINE int generate_into(const std::string &text,
                           PNMImage &dest_image, int x, int y);
  int generate_into(const std::wstring &text,
                    PNMImage &dest_image, int x, int y);
  INLINE int calc_width(const std::string &text);
  int calc_width(const std::wstring &text);

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
  LColor _fg;
  LColor _interior;
  int _distance_field_radius;
};

#include "pnmTextMaker.I"

#endif
