// Filename: textAssembler.h
// Created by:  drose (06Apr04)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef TEXTASSEMBLER_H
#define TEXTASSEMBLER_H

#ifndef CPPPARSER  // interrogate has a bit of trouble with wstring.

#include "pandabase.h"

#include "textProperties.h"
#include "textFont.h"
#include "unicodeLatinMap.h"
#include "geomNode.h"
#include "pointerTo.h"

class TextEncoder;

////////////////////////////////////////////////////////////////////
//       Class : TextAssembler
// Description : This class is not intended to be used directly by
//               user code, but is used by the TextNode to lay out a
//               block of text and convert it into rows of Geoms
//               according to the TextProperties.
//
//               It is not exported from the DLL since it is not
//               intended to be used outside of this module.
////////////////////////////////////////////////////////////////////
class TextAssembler {
public:
  TextAssembler(TextEncoder *encoder);
  ~TextAssembler();

  void clear();

  bool set_wtext(const wstring &wtext, const TextProperties &properties,
                 int max_rows = 0);
  INLINE int get_num_rows() const;
  wstring get_wordwrapped_wtext() const;

  PT(PandaNode) assemble_text();

  INLINE const LVector2f &get_ul() const;
  INLINE const LVector2f &get_lr() const;

  static float calc_width(wchar_t character, const TextProperties &properties);

private:
  // These structures are built up and operated on by scan_wtext() and
  // wordwrap_text().  It represents the unrolling of the embedded \1
  // .. \2 sequences embedded in the string into a TextProperties
  // pointer associated with each character.
  typedef pvector<TextProperties *> PropertiesList;

  class TextCharacter {
  public:
    INLINE TextCharacter(wchar_t character, const TextProperties *properties);
    wchar_t _character;
    const TextProperties *_properties;
  };
  typedef pvector<TextCharacter> TextString;

  PropertiesList _properties_list;
  TextString _wordwrapped_string;
  int _num_rows;

  void scan_wtext(wstring::const_iterator &si, 
                  const wstring::const_iterator &send,
                  const TextProperties *current_properties,
                  TextString &text_string);
  bool wordwrap_text(const TextAssembler::TextString &text,
                     TextAssembler::TextString &output_text,
                     int max_rows);

  INLINE static float calc_width(const TextCharacter &tch);
  static float calc_hyphen_width(const TextCharacter &tch);

  // These structures are built up by assemble_paragraph() and
  // assemble_row().  They represent the actual Geoms as laid out in a
  // paragraph.
  
  class Piece {
  public:
    PT(Geom) _geom;
    CPT(RenderState) _state;
  };
  typedef pvector<Piece> Pieces;

  class GlyphPlacement {
  public:
    INLINE void add_piece(Geom *geom, const RenderState *state);
    void calc_tight_bounds(LPoint3f &min_point, LPoint3f &max_point,
                           bool &found_any) const;
    void assign_to(GeomNode *geom_node, const RenderState *state) const;
    void assign_copy_to(GeomNode *geom_node, const RenderState *state, 
                        const LMatrix4f &xform) const;

    Pieces _pieces;
    LMatrix4f _xform;
    const TextProperties *_properties;
  };
  typedef pvector<GlyphPlacement *> PlacedGlyphs;

  void assemble_paragraph(TextString::const_iterator si, 
                          const TextString::const_iterator &send,
                          PlacedGlyphs &placed_glyphs);
  void assemble_row(TextString::const_iterator &si, 
                    const TextString::const_iterator &send,
                    PlacedGlyphs &row_placed_glyphs,
                    float &row_width, float &line_height, 
                    TextProperties::Alignment &align);

  // These interfaces are for implementing cheesy accent marks and
  // ligatures when the font doesn't support them.
  enum CheesyPosition {
    CP_above,
    CP_below,
    CP_top,
    CP_bottom,
    CP_within,
  };
  enum CheesyTransform {
    CT_none,
    CT_mirror_x,
    CT_mirror_y,
    CT_rotate_90,
    CT_rotate_180,
    CT_rotate_270,
    CT_squash,
    CT_squash_mirror_y,
    CT_squash_mirror_diag,
    CT_small_squash,
    CT_small_squash_mirror_y,
    CT_small_squash_mirror_diag,
    CT_small,
    CT_small_rotate_270,
    CT_tiny,
    CT_tiny_mirror_x,
    CT_tiny_rotate_270,
  };

  static void
  get_character_glyphs(int character, const TextProperties *properties,
                       bool &got_glyph, const TextGlyph *&glyph,
                       const TextGlyph *&second_glyph,
                       UnicodeLatinMap::AccentType &accent_type,
                       int &additional_flags,
                       float &glyph_scale, float &advance_scale);

  static void
  tack_on_accent(UnicodeLatinMap::AccentType accent_type,
                 const LPoint3f &min_vert, const LPoint3f &max_vert,
                 const LPoint3f &centroid,
                 const TextProperties *properties, GlyphPlacement *placement);
  static bool 
  tack_on_accent(char accent_mark, CheesyPosition position,
                 CheesyTransform transform,
                 const LPoint3f &min_vert, const LPoint3f &max_vert,
                 const LPoint3f &centroid,
                 const TextProperties *properties, GlyphPlacement *placement);

  // These are filled in by assemble_paragraph().
  LVector2f _ul;
  LVector2f _lr;

  TextEncoder *_encoder;
};

#include "textAssembler.I"

#endif  // CPPPARSER

#endif

