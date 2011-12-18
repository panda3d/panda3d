// Filename: textAssembler.h
// Created by:  drose (06Apr04)
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

#ifndef TEXTASSEMBLER_H
#define TEXTASSEMBLER_H

#include "pandabase.h"

#include "textProperties.h"
#include "textFont.h"
#include "unicodeLatinMap.h"
#include "geomNode.h"
#include "pointerTo.h"
#include "geomTextGlyph.h"
#include "textPropertiesManager.h"
#include "textEncoder.h"
#include "geomVertexRewriter.h"

#include "pmap.h"


class TextEncoder;
class TextGraphic;
class TextAssembler;

////////////////////////////////////////////////////////////////////
//       Class : TextAssembler
// Description : This class is not normally used directly by user
//               code, but is used by the TextNode to lay out a block
//               of text and convert it into rows of Geoms according
//               to the TextProperties.  However, user code may take
//               advantage of it, if desired, for very low-level text
//               operations.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_TEXT TextAssembler {
PUBLISHED:
  TextAssembler(TextEncoder *encoder);
  TextAssembler(const TextAssembler &copy);
  void operator = (const TextAssembler &copy);
  ~TextAssembler();

  void clear();

  INLINE void set_usage_hint(Geom::UsageHint usage_hint);
  INLINE Geom::UsageHint get_usage_hint() const;

  INLINE void set_max_rows(int max_rows);
  INLINE int get_max_rows() const;

  INLINE void set_dynamic_merge(bool dynamic_merge);
  INLINE bool get_dynamic_merge() const;

  INLINE void set_multiline_mode(bool flag);
  INLINE bool get_multiline_mode() const;

  INLINE void set_properties(const TextProperties &properties);
  INLINE const TextProperties &get_properties() const;

  bool set_wtext(const wstring &wtext);
  bool set_wsubstr(const wstring &wtext, int start, int count);

  wstring get_plain_wtext() const;
  wstring get_wordwrapped_plain_wtext() const;
  wstring get_wtext() const;
  wstring get_wordwrapped_wtext() const;

  bool calc_r_c(int &r, int &c, int n) const;
  INLINE int calc_r(int n) const;
  INLINE int calc_c(int n) const;
  int calc_index(int r, int c) const;

  INLINE int get_num_characters() const;
  INLINE wchar_t get_character(int n) const;
  INLINE const TextGraphic *get_graphic(int n) const;
  INLINE const TextProperties &get_properties(int n) const;
  INLINE PN_stdfloat get_width(int n) const;

  INLINE int get_num_rows() const;
  INLINE int get_num_cols(int r) const;
  INLINE wchar_t get_character(int r, int c) const;
  INLINE const TextGraphic *get_graphic(int r, int c) const;
  INLINE const TextProperties &get_properties(int r, int c) const;
  INLINE PN_stdfloat get_width(int r, int c) const;
  PN_stdfloat get_xpos(int r, int c) const;
  INLINE PN_stdfloat get_ypos(int r, int c) const;

  PT(PandaNode) assemble_text();

  INLINE const LVector2 &get_ul() const;
  INLINE const LVector2 &get_lr() const;

  static PN_stdfloat calc_width(wchar_t character, const TextProperties &properties);
  static PN_stdfloat calc_width(const TextGraphic *graphic, const TextProperties &properties);

  static bool has_exact_character(wchar_t character, const TextProperties &properties);
  static bool has_character(wchar_t character, const TextProperties &properties);
  static bool is_whitespace(wchar_t character, const TextProperties &properties);

private:
  class ComputedProperties : public ReferenceCount {
  public:
    INLINE ComputedProperties(const TextProperties &orig_properties);
    INLINE ComputedProperties(ComputedProperties *based_on, 
                              const wstring &wname, TextEncoder *encoder);
    void append_delta(wstring &wtext, ComputedProperties *other);

    PT(ComputedProperties) _based_on;
    int _depth;
    wstring _wname;
    TextProperties _properties;
  };

  // These structures are built up and operated on by scan_wtext() and
  // wordwrap_text().  It represents the unrolling of the embedded \1
  // .. \2 sequences embedded in the string into a TextProperties
  // pointer associated with each character.
  class TextCharacter {
  public:
    INLINE TextCharacter(wchar_t character, ComputedProperties *cprops);
    INLINE TextCharacter(const TextGraphic *graphic, 
                         const wstring &graphic_wname,
                         ComputedProperties *cprops);
    INLINE TextCharacter(const TextCharacter &copy);
    INLINE void operator = (const TextCharacter &copy);

    wchar_t _character;
    const TextGraphic *_graphic;
    wstring _graphic_wname;
    PT(ComputedProperties) _cprops;
  };
  typedef pvector<TextCharacter> TextString;

  class TextRow {
  public:
    INLINE TextRow(int row_start);
    INLINE TextRow(const TextRow &copy);
    INLINE void operator = (const TextRow &copy);

    TextString _string;
    int _row_start;
    bool _got_soft_hyphens;
    PN_stdfloat _xpos;
    PN_stdfloat _ypos;
    PT(ComputedProperties) _eol_cprops;
  };
  typedef pvector<TextRow> TextBlock;

  PT(ComputedProperties) _initial_cprops;

  // This is the string, unwordwrapped.
  TextString _text_string;

  // And here it is, wordwrapped.
  TextBlock _text_block;

#ifndef CPPPARSER  // interrogate has a bit of trouble with wstring iterators.
  void scan_wtext(TextString &output_string,
                  wstring::const_iterator &si, 
                  const wstring::const_iterator &send,
                  ComputedProperties *current_cprops);
#endif  // CPPPARSER

  bool wordwrap_text();

  INLINE static PN_stdfloat calc_width(const TextCharacter &tch);
  static PN_stdfloat calc_hyphen_width(const TextCharacter &tch);

  // These structures are built up by assemble_paragraph() and
  // assemble_row().  They represent the actual Geoms as laid out in a
  // paragraph.
  
  class Piece {
  public:
    PT(Geom) _geom;
    CPT(RenderState) _state;
  };
  typedef pvector<Piece> Pieces;

  class GeomCollectorKey {
  public:
    INLINE GeomCollectorKey(const RenderState *state, const GeomVertexFormat *format);
    INLINE bool operator < (const GeomCollectorKey &other) const;

    CPT(RenderState) _state;
    CPT(GeomVertexFormat) _format;
  };

  typedef pmap<int, int> VertexIndexMap;

  class GeomCollector {
  public:
    GeomCollector(const GeomVertexFormat *format);
    GeomCollector(const GeomCollector &copy);

    INLINE void count_geom(const Geom *geom);
    GeomPrimitive *get_primitive(TypeHandle prim_type);
    int append_vertex(const GeomVertexData *orig_vdata, int orig_row,
                      const LMatrix4 &xform);
    void append_geom(GeomNode *geom_node, const RenderState *state);

  private:
    PT(GeomVertexData) _vdata;
    PT(GeomTextGlyph) _geom;
    PT(GeomTriangles) _triangles;
    PT(GeomLines) _lines;
    PT(GeomPoints) _points;
  };
  typedef pmap<GeomCollectorKey, GeomCollector> GeomCollectorMap;

  class GlyphPlacement : public MemoryBase {
  public:
    INLINE void add_piece(Geom *geom, const RenderState *state);
    void calc_tight_bounds(LPoint3 &min_point, LPoint3 &max_point,
                           bool &found_any, Thread *current_thread) const;
    void assign_to(GeomNode *geom_node, const RenderState *state) const;
    void assign_copy_to(GeomNode *geom_node, const RenderState *state, 
                        const LMatrix4 &extra_xform) const;

    void assign_append_to(GeomCollectorMap &geom_collector_map, const RenderState *state,
                          const LMatrix4 &extra_xform) const;
    void copy_graphic_to(PandaNode *node, const RenderState *state,
                         const LMatrix4 &extra_xform) const;

    Pieces _pieces;
    PT(PandaNode) _graphic_model;
    LMatrix4 _xform;
    const TextProperties *_properties;
  };
  typedef pvector<GlyphPlacement *> PlacedGlyphs;

  void assemble_paragraph(PlacedGlyphs &placed_glyphs);
  void assemble_row(TextRow &row,
                    PlacedGlyphs &row_placed_glyphs,
                    PN_stdfloat &row_width, PN_stdfloat &line_height, 
                    TextProperties::Alignment &align, PN_stdfloat &wordwrap);

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
  draw_underscore(TextAssembler::PlacedGlyphs &row_placed_glyphs,
                  PN_stdfloat underscore_start, PN_stdfloat underscore_end, 
                  const TextProperties *underscore_properties);

  static void
  get_character_glyphs(int character, const TextProperties *properties,
                       bool &got_glyph, const TextGlyph *&glyph,
                       const TextGlyph *&second_glyph,
                       UnicodeLatinMap::AccentType &accent_type,
                       int &additional_flags,
                       PN_stdfloat &glyph_scale, PN_stdfloat &advance_scale);

  void
  tack_on_accent(UnicodeLatinMap::AccentType accent_type,
                 const LPoint3 &min_vert, const LPoint3 &max_vert,
                 const LPoint3 &centroid,
                 const TextProperties *properties, GlyphPlacement *placement) const;
  bool 
  tack_on_accent(char accent_mark, CheesyPosition position,
                 CheesyTransform transform,
                 const LPoint3 &min_vert, const LPoint3 &max_vert,
                 const LPoint3 &centroid,
                 const TextProperties *properties, GlyphPlacement *placement) const;

  // These are filled in by assemble_paragraph().
  LVector2 _ul;
  LVector2 _lr;
  PN_stdfloat _next_row_ypos;

  TextEncoder *_encoder;
  Geom::UsageHint _usage_hint;
  int _max_rows;
  bool _dynamic_merge;
  bool _multiline_mode;

};

#include "textAssembler.I"

#endif

