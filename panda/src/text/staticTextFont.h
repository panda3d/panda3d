// Filename: staticTextFont.h
// Created by:  drose (03May01)
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

#ifndef STATICTEXTFONT_H
#define STATICTEXTFONT_H

#include "pandabase.h"

#include "config_text.h"
#include "textFont.h"
#include "textGlyph.h"
#include "pandaNode.h"
#include "geom.h"
#include "qpgeom.h"
#include "pointerTo.h"
#include "pmap.h"

class Node;
class GeomPoint;

////////////////////////////////////////////////////////////////////
//       Class : StaticTextFont
// Description : A StaticTextFont is loaded up from a model that was
//               previously generated via egg-mkfont, and contains all
//               of its glyphs already generated and available for
//               use.  It doesn't require linking with any external
//               libraries like FreeType.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA StaticTextFont : public TextFont {
PUBLISHED:
  StaticTextFont(PandaNode *font_def);

  virtual void write(ostream &out, int indent_level) const;

public:
  virtual bool get_glyph(int character, const TextGlyph *&glyph);

private:
  void find_character_gsets(PandaNode *root, CPT(Geom) &ch, 
                            const GeomPoint *&dot1,
                            CPT(qpGeom) &dot2,
                            const RenderState *&state, 
                            const RenderState *net_state);
  void find_characters(PandaNode *root,
                       const RenderState *net_state);

  typedef pmap<int, PT(TextGlyph)> Glyphs;
  Glyphs _glyphs;
  float _font_height;
  PT(PandaNode) _font;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TextFont::init_type();
    register_type(_type_handle, "StaticTextFont",
                  TextFont::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "staticTextFont.I"

#endif
