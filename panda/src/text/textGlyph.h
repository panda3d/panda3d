// Filename: textGlyph.h
// Created by:  drose (08Feb02)
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

#ifndef TEXTGLYPH_H
#define TEXTGLYPH_H

#include "pandabase.h"
#include "renderState.h"
#include "referenceCount.h"
#include "geom.h"
#include "pointerTo.h"
#include "dcast.h"

////////////////////////////////////////////////////////////////////
//       Class : TextGlyph
// Description : A representation of a single glyph (character) from a
//               font.  This is a piece of renderable geometry of some
//               kind.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_TEXT TextGlyph : public TypedReferenceCount {
public:
  INLINE TextGlyph(int character);
  INLINE TextGlyph(int character, const Geom *geom, 
                   const RenderState *state, PN_stdfloat advance);
  INLINE TextGlyph(const TextGlyph &copy);
  INLINE void operator = (const TextGlyph &copy);
  virtual ~TextGlyph();

PUBLISHED:
  INLINE int get_character() const;
  INLINE PT(Geom) get_geom(Geom::UsageHint usage_hint) const;
  INLINE const RenderState *get_state() const;
  INLINE PN_stdfloat get_advance() const;

  virtual bool is_whitespace() const;

protected:
  int _character;
  CPT(Geom) _geom;
  CPT(RenderState) _state;
  PN_stdfloat _advance;

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
