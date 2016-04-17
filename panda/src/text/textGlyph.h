/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file textGlyph.h
 * @author drose
 * @date 2002-02-08
 */

#ifndef TEXTGLYPH_H
#define TEXTGLYPH_H

#include "pandabase.h"
#include "renderState.h"
#include "referenceCount.h"
#include "geom.h"
#include "pointerTo.h"
#include "dcast.h"

/**
 * A representation of a single glyph (character) from a font.  This is a
 * piece of renderable geometry of some kind.
 */
class EXPCL_PANDA_TEXT TextGlyph : public TypedReferenceCount {
public:
  INLINE TextGlyph(int character, PN_stdfloat advance=0);
  INLINE TextGlyph(int character, const Geom *geom,
                   const RenderState *state, PN_stdfloat advance);
  INLINE TextGlyph(const TextGlyph &copy);
  INLINE void operator = (const TextGlyph &copy);
  virtual ~TextGlyph();

PUBLISHED:
  INLINE int get_character() const;
  INLINE bool has_quad() const;
  INLINE bool get_quad(LVecBase4 &dimensions, LVecBase4 &texcoords) const;
  INLINE const RenderState *get_state() const;
  INLINE PN_stdfloat get_advance() const;

  MAKE_PROPERTY(character, get_character);
  MAKE_PROPERTY(state, get_state);
  MAKE_PROPERTY(advance, get_advance);

  virtual bool is_whitespace() const;

  PT(Geom) get_geom(Geom::UsageHint usage_hint) const;

public:
  void calc_tight_bounds(LPoint3 &min_point, LPoint3 &max_point,
                         bool &found_any, Thread *current_thread) const;

  void set_quad(const LVecBase4 &dimensions, const LVecBase4 &texcoords,
                const RenderState *state);

  void set_geom(GeomVertexData *vdata, GeomPrimitive *prim,
                const RenderState *state);

private:
  void check_quad_geom();
  void make_quad_geom();

protected:
  int _character;
  CPT(Geom) _geom;
  CPT(RenderState) _state;
  PN_stdfloat _advance;

  bool _has_quad;
  // top, left, bottom, right
  LVecBase4 _quad_dimensions;
  LVecBase4 _quad_texcoords;

  friend class GeomTextGlyph;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "TextGlyph",
                  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "textGlyph.I"

#endif
