/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file textAssembler.h
 * @author drose
 * @date 2004-04-06
 */

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

typedef struct hb_buffer_t hb_buffer_t;

class TextEncoder;
class TextGraphic;
class TextAssembler;

/**
 * This class is not normally used directly by user code, but is used by the
 * TextNode to lay out a block of text and convert it into rows of Geoms
 * according to the TextProperties.  However, user code may take advantage of
 * it, if desired, for very low-level text operations.
 */
class EXPCL_PANDA_TEXT TextAssembler {
PUBLISHED:
  explicit TextAssembler(TextEncoder *encoder);
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

  bool set_wtext(const std::wstring &wtext);
  bool set_wsubstr(const std::wstring &wtext, int start, int count);

  std::wstring get_plain_wtext() const;
  std::wstring get_wordwrapped_plain_wtext() const;
  std::wstring get_wtext() const;
  std::wstring get_wordwrapped_wtext() const;

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

PUBLISHED:
  MAKE_PROPERTY(usage_hint, get_usage_hint, set_usage_hint);
  MAKE_PROPERTY(max_rows, get_max_rows, set_max_rows);
  MAKE_PROPERTY(dynamic_merge, get_dynamic_merge, set_dynamic_merge);
  MAKE_PROPERTY(multiline_mode, get_multiline_mode, set_multiline_mode);
  MAKE_PROPERTY(properties, get_properties, set_properties);

private:
  class ComputedProperties : public ReferenceCount {
  public:
    INLINE ComputedProperties(const TextProperties &orig_properties);
    INLINE ComputedProperties(ComputedProperties *based_on,
                              const std::wstring &wname, TextEncoder *encoder);
    void append_delta(std::wstring &wtext, ComputedProperties *other);

    PT(ComputedProperties) _based_on;
    int _depth;
    std::wstring _wname;
    TextProperties _properties;
  };

  // These structures are built up and operated on by scan_wtext() and
  // wordwrap_text().  It represents the unrolling of the embedded \1 .. \2
  // sequences embedded in the string into a TextProperties pointer associated
  // with each character.
  class TextCharacter {
  public:
    INLINE TextCharacter(wchar_t character, ComputedProperties *cprops);
    INLINE TextCharacter(const TextGraphic *graphic,
                         const std::wstring &graphic_wname,
                         ComputedProperties *cprops);
    INLINE TextCharacter(const TextCharacter &copy);
    INLINE void operator = (const TextCharacter &copy);

    wchar_t _character;
    const TextGraphic *_graphic;
    std::wstring _graphic_wname;
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

  void scan_wtext(TextString &output_string,
                  std::wstring::const_iterator &si,
                  const std::wstring::const_iterator &send,
                  ComputedProperties *current_cprops);

  bool wordwrap_text();

  INLINE static PN_stdfloat calc_width(const TextCharacter &tch);
  static PN_stdfloat calc_hyphen_width(const TextCharacter &tch);

  // These structures are built up by assemble_paragraph() and assemble_row().
  // They represent the actual Geoms as laid out in a paragraph.

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

  struct QuadDef {
    LVecBase4 _dimensions;
    LVecBase4 _uvs;
    PN_stdfloat _slantl, _slanth;
    CPT(TextGlyph) _glyph;
  };
  typedef epvector<QuadDef> QuadDefs;
  typedef pmap<CPT(RenderState), QuadDefs> QuadMap;

  void generate_quads(GeomNode *geom_node, const QuadMap &quad_map);

  class GlyphPlacement {
  public:
    void assign_to(GeomNode *geom_node, const RenderState *state,
                   const LVector2 &offset = LVector2::zero()) const;

    void assign_append_to(GeomCollectorMap &geom_collector_map, const RenderState *state,
                          const LVector2 &offset = LVector2::zero()) const;
    void assign_quad_to(QuadMap &quad_map, const RenderState *state,
                        const LVector2 &offset = LVector2::zero()) const;
    void copy_graphic_to(PandaNode *node, const RenderState *state) const;

    CPT(TextGlyph) _glyph;
    PT(PandaNode) _graphic_model;
    PN_stdfloat _xpos, _ypos;
    PN_stdfloat _scale, _slant;
    const TextProperties *_properties;
  };
  typedef pvector<GlyphPlacement> PlacedGlyphs;

  void assemble_paragraph(PlacedGlyphs &placed_glyphs);
  void assemble_row(TextRow &row,
                    PlacedGlyphs &row_placed_glyphs,
                    PN_stdfloat &row_width, PN_stdfloat &line_height,
                    TextProperties::Alignment &align, PN_stdfloat &wordwrap);

  void shape_buffer(hb_buffer_t *buf, PlacedGlyphs &glyphs, PN_stdfloat &xpos,
                    const TextProperties &properties);

  // These interfaces are for implementing cheesy accent marks and ligatures
  // when the font doesn't support them.
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
                       bool &got_glyph, CPT(TextGlyph) &glyph,
                       CPT(TextGlyph) &second_glyph,
                       UnicodeLatinMap::AccentType &accent_type,
                       int &additional_flags,
                       PN_stdfloat &glyph_scale, PN_stdfloat &advance_scale);

  void
  tack_on_accent(UnicodeLatinMap::AccentType accent_type,
                 const LPoint3 &min_vert, const LPoint3 &max_vert,
                 const LPoint3 &centroid,
                 const TextProperties *properties, GlyphPlacement &placement) const;
  bool
  tack_on_accent(wchar_t accent_mark, CheesyPosition position,
                 CheesyTransform transform,
                 const LPoint3 &min_vert, const LPoint3 &max_vert,
                 const LPoint3 &centroid,
                 const TextProperties *properties, GlyphPlacement &placement) const;

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
