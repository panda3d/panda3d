// Filename: textGlyph.h
// Created by:  drose (08Feb02)
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

#ifndef TEXTGLYPH_H
#define TEXTGLYPH_H

#include "pandabase.h"
#include "allTransitionsWrapper.h"

class Geom;
class TextGlyph;

////////////////////////////////////////////////////////////////////
//       Class : TextGlyph
// Description : A representation of a single glyph (character) from a
//               font.  This is a piece of renderable geometry of some
//               kind.
////////////////////////////////////////////////////////////////////
class TextGlyph {
public:
  INLINE TextGlyph();
  INLINE TextGlyph(Geom *geom, const AllTransitionsWrapper &trans, float advance);
  INLINE TextGlyph(const TextGlyph &copy);
  INLINE void operator = (const TextGlyph &copy);

  INLINE Geom *get_geom() const;
  INLINE const AllTransitionsWrapper &get_trans() const;
  INLINE float get_advance() const;

private:
  Geom *_geom;
  AllTransitionsWrapper _trans;
  float _advance;
};

#include "textGlyph.I"

#endif
