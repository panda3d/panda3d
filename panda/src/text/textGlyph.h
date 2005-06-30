// Filename: textGlyph.h
// Created by:  drose (08Feb02)
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
class EXPCL_PANDA TextGlyph : public ReferenceCount {
public:
  INLINE TextGlyph();
  INLINE TextGlyph(const Geom *geom, const RenderState *state, float advance);
  INLINE TextGlyph(const TextGlyph &copy);
  INLINE void operator = (const TextGlyph &copy);
  virtual ~TextGlyph();

  INLINE PT(Geom) get_geom(Geom::UsageHint usage_hint) const;
  INLINE const RenderState *get_state() const;
  INLINE float get_advance() const;

protected:
  CPT(Geom) _geom;
  CPT(RenderState) _state;
  float _advance;
};

#include "textGlyph.I"

#endif
