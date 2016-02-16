/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file textFont.cxx
 * @author drose
 * @date 2002-02-08
 */

#include "textFont.h"
#include "config_text.h"
#include "string_utils.h"
#include "geomVertexData.h"
#include "geomVertexFormat.h"
#include "geomVertexWriter.h"
#include "geomLinestrips.h"
#include "geom.h"
#include <ctype.h>

TypeHandle TextFont::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: TextFont::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
TextFont::
TextFont() {
  _is_valid = false;
  _line_height = 1.0f;
  _space_advance = 0.25f;
  _total_poly_margin = 0.0f;
}

////////////////////////////////////////////////////////////////////
//     Function: TextFont::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
TextFont::
TextFont(const TextFont &copy) :
  Namable(copy),
  _is_valid(copy._is_valid),
  _line_height(copy._line_height),
  _space_advance(copy._space_advance),
  _total_poly_margin(copy._total_poly_margin)
{
}

////////////////////////////////////////////////////////////////////
//     Function: TextFont::Destructor
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
TextFont::
~TextFont() {
}

////////////////////////////////////////////////////////////////////
//     Function: TextFont::write
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void TextFont::
write(ostream &out, int indent_level) const {
  indent(out, indent_level)
    << "TextFont " << get_name() << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: TextFont::get_invalid_glyph
//       Access: Public
//  Description: Returns a special glyph that can be used as a
//               placeholder for any character not in the font.  Note
//               that it is not guaranteed that a font will return
//               this particular glyph for a missing character (it may
//               return a glyph of its own devising instead).  
//
//               Also note that even if a particular accented letter
//               is missing from the font, Panda may still be able to
//               render a suitable replacement by composing different
//               glyphs together to simulate accent marks; this
//               happens automatically behind the scenes.
////////////////////////////////////////////////////////////////////
TextGlyph *TextFont::
get_invalid_glyph() {
  if (_invalid_glyph == (TextGlyph *)NULL) {
    make_invalid_glyph();
  }
  return _invalid_glyph;
}

////////////////////////////////////////////////////////////////////
//     Function: TextFont::string_render_mode
//       Access: Public
//  Description: Returns the RenderMode value associated with the given
//               string representation, or RM_invalid if the string
//               does not match any known RenderMode value.
////////////////////////////////////////////////////////////////////
TextFont::RenderMode TextFont::
string_render_mode(const string &string) {
  if (cmp_nocase_uh(string, "texture") == 0) {
    return RM_texture;
  } else if (cmp_nocase_uh(string, "wireframe") == 0) {
    return RM_wireframe;
  } else if (cmp_nocase_uh(string, "polygon") == 0) {
    return RM_polygon;
  } else if (cmp_nocase_uh(string, "extruded") == 0) {
    return RM_extruded;
  } else if (cmp_nocase_uh(string, "solid") == 0) {
    return RM_solid;
  } else if (cmp_nocase_uh(string, "distance_field") == 0) {
    return RM_distance_field;
  } else {
    return RM_invalid;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TextFont::make_invalid_glyph
//       Access: Private
//  Description: Constructs the special glyph used to represent a
//               character not in the font.
////////////////////////////////////////////////////////////////////
void TextFont::
make_invalid_glyph() {
  CPT(GeomVertexFormat) vformat = GeomVertexFormat::get_v3();
  PT(GeomVertexData) vdata = 
    new GeomVertexData("invalid_glyph", vformat, GeomEnums::UH_static);

  GeomVertexWriter vertex(vdata, InternalName::get_vertex());
  vertex.add_data3(_line_height * 0.2, 0.0f, _line_height * 0.1f);
  vertex.add_data3(_line_height * 0.5f, 0.0f, _line_height * 0.1f);
  vertex.add_data3(_line_height * 0.5f, 0.0f, _line_height * 0.7f);
  vertex.add_data3(_line_height * 0.2, 0.0f, _line_height * 0.7f);

  PT(GeomPrimitive) prim = new GeomLinestrips(GeomEnums::UH_static);
  prim->add_consecutive_vertices(0, 4);
  prim->add_vertex(0);
  prim->close_primitive();

  PT(Geom) geom = new Geom(vdata);
  geom->add_primitive(prim);

  _invalid_glyph = new TextGlyph(0, geom, RenderState::make_empty(),
                                 _line_height * 0.7f);
}

////////////////////////////////////////////////////////////////////
//     Function: TextFont::RenderMode output operator
//  Description:
////////////////////////////////////////////////////////////////////
ostream &
operator << (ostream &out, TextFont::RenderMode rm) {
  switch (rm) {
  case TextFont::RM_texture:
    return out << "texture";
  case TextFont::RM_wireframe:
    return out << "wireframe";
  case TextFont::RM_polygon:
    return out << "polygon";
  case TextFont::RM_extruded:
    return out << "extruded";
  case TextFont::RM_solid:
    return out << "solid";
  case TextFont::RM_distance_field:
    return out << "distance-field";

  case TextFont::RM_invalid:
    return out << "invalid";
  }

  return out << "(**invalid TextFont::RenderMode(" << (int)rm << ")**)";
}

////////////////////////////////////////////////////////////////////
//     Function: TextFont::RenderMode input operator
//  Description:
////////////////////////////////////////////////////////////////////
istream &
operator >> (istream &in, TextFont::RenderMode &rm) {
  string word;
  in >> word;

  rm = TextFont::string_render_mode(word);
  return in;
}
