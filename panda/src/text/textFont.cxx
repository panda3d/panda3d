// Filename: textFont.cxx
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
  _space_advance(copy._space_advance)
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
  } else {
    return RM_invalid;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TextFont::string_winding_order
//       Access: Public
//  Description: Returns the WindingOrder value associated with the given
//               string representation, or WO_invalid if the string
//               does not match any known WindingOrder value.
////////////////////////////////////////////////////////////////////
TextFont::WindingOrder TextFont::
string_winding_order(const string &string) {
  if (cmp_nocase_uh(string, "default") == 0) {
    return WO_default;
  } else if (cmp_nocase_uh(string, "left") == 0) {
    return WO_left;
  } else if (cmp_nocase_uh(string, "right") == 0) {
    return WO_right;
  } else {
    return WO_invalid;
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

////////////////////////////////////////////////////////////////////
//     Function: TextFont::WindingOrder output operator
//  Description:
////////////////////////////////////////////////////////////////////
ostream &
operator << (ostream &out, TextFont::WindingOrder wo) {
  switch (wo) {
  case TextFont::WO_default:
    return out << "default";
  case TextFont::WO_left:
    return out << "left";
  case TextFont::WO_right:
    return out << "right";

  case TextFont::WO_invalid:
    return out << "invalid";
  }

  return out << "(**invalid TextFont::WindingOrder(" << (int)wo << ")**)";
}

////////////////////////////////////////////////////////////////////
//     Function: TextFont::WindingOrder input operator
//  Description:
////////////////////////////////////////////////////////////////////
istream &
operator >> (istream &in, TextFont::WindingOrder &wo) {
  string word;
  in >> word;

  wo = TextFont::string_winding_order(word);
  return in;
}
