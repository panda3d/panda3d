/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file staticTextFont.h
 * @author drose
 * @date 2001-05-03
 */

#ifndef STATICTEXTFONT_H
#define STATICTEXTFONT_H

#include "pandabase.h"

#include "config_text.h"
#include "coordinateSystem.h"
#include "textFont.h"
#include "textGlyph.h"
#include "pandaNode.h"
#include "geom.h"
#include "geom.h"
#include "pointerTo.h"
#include "pmap.h"

class Node;
class GeomPoint;

/**
 * A StaticTextFont is loaded up from a model that was previously generated
 * via egg-mkfont, and contains all of its glyphs already generated and
 * available for use.  It doesn't require linking with any external libraries
 * like FreeType.
 */
class EXPCL_PANDA_TEXT StaticTextFont : public TextFont {
PUBLISHED:
  StaticTextFont(PandaNode *font_def, CoordinateSystem cs = CS_default);

  virtual PT(TextFont) make_copy() const;

  virtual void write(std::ostream &out, int indent_level) const;

public:
  virtual bool get_glyph(int character, CPT(TextGlyph) &glyph);

private:
  void find_character_gsets(PandaNode *root, CPT(Geom) &ch, CPT(Geom) &dot,
                            const RenderState *&state,
                            const RenderState *net_state);
  void find_characters(PandaNode *root,
                       const RenderState *net_state);

  typedef pmap<int, PT(TextGlyph)> Glyphs;
  Glyphs _glyphs;
  PT(PandaNode) _font;
  CoordinateSystem _cs;

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
